/*
 * Copyright (C) 2014 Emweb bvba, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "ProxyReply.h"

#include "Wt/Http/Request"
#include "Connection.h"
#include "Server.h"
#include "StockReply.h"
#include "Wt/Json/Serializer"
#include "Wt/Json/Value"
#include "Wt/Json/Array"
#include "Wt/Json/Object"
#include "Wt/WSslCertificate"
#include "Wt/WSslInfo"
#include "SslUtils.h"
#include "Wt/WString"
#include "Wt/Utils"

namespace Wt {
  LOGGER("wthttp/proxy");
}

namespace http {
namespace server {

ProxyReply::ProxyReply(Request& request,
		       const Configuration& config,
		       SessionProcessManager& sessionManager)
  : Reply(request, config),
    sessionManager_(sessionManager),
    out_(&out_buf_),
    sending_(0),
    more_(true),
    receiving_(false),
    fwCertificates_(false)
{
  reset(0);
}

ProxyReply::~ProxyReply()
{
  if (sessionProcess_ && sessionProcess_->sessionId().empty())
    sessionProcess_->stop();

  closeClientSocket();
}

void ProxyReply::closeClientSocket()
{
  if (socket_) {
    boost::system::error_code ignored_ec;
    socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    socket_->close();
    socket_.reset();
  }
}

void ProxyReply::reset(const Wt::EntryPoint *ep)
{
  if (sessionProcess_ && sessionProcess_->sessionId().empty())
    sessionProcess_->stop();
  sessionProcess_.reset();

  closeClientSocket();
  contentType_.clear();
  requestBuf_.consume(requestBuf_.size());
  responseBuf_.consume(responseBuf_.size());
  out_buf_.consume(out_buf_.size());
  sending_ = 0;
  more_ = true;
  receiving_ = false;
  contentLength_ = -1;
  queryParams_.clear();

  Reply::reset(ep);
}

void ProxyReply::writeDone(bool success)
{
  if (!success) {
    closeClientSocket();
    return;
  }

  out_buf_.consume(sending_);

  if (request_.type == Request::TCP && !receiving_) {
    // Sent 101 response, start receiving more data from the client
    receiving_ = true;
    LOG_DEBUG(this << ": receive() upstream");
    receive();
  }

  if (more_ && socket_) {
    LOG_DEBUG(this << ": async_read downstream");
    asio::async_read
      (*socket_, responseBuf_,
       asio::transfer_at_least(1),
       connection()->strand().wrap
       (boost::bind(&ProxyReply::handleResponseRead,
		    boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
		    asio::placeholders::error)));
  }
}

bool ProxyReply::consumeData(Buffer::const_iterator begin,
			     Buffer::const_iterator end,
			     Request::State state)
{
  LOG_DEBUG(this << ": consumeData()");

  if (state == Request::Error) {
    return false;
  }

  beginRequestBuf_ = begin;
  endRequestBuf_ = end;
  state_ = state;

  if (sessionProcess_) {
   if (socket_) {
      LOG_DEBUG(this << ": sending to child");
      // Connection with child already established, send request data
      asio::async_write
	(*socket_,
	 asio::buffer(beginRequestBuf_, endRequestBuf_ - beginRequestBuf_),
	 connection()->strand().wrap
	 (boost::bind
	  (&ProxyReply::handleDataWritten,
	   boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	   asio::placeholders::error,
	   asio::placeholders::bytes_transferred)));
    } else {
      /* Connection with child was closed */
      error(service_unavailable);
    }
  } else {
    queryParams_.clear();
    Wt::Http::Request::parseFormUrlEncoded(request_.request_query,
					   queryParams_);

    // First connect to child process
    std::string sessionId = getSessionId();

    sessionProcess_ = sessionManager_.sessionProcess(sessionId);

    if (sessionId.empty() || !sessionProcess_) {
      if (!sessionId.empty()) {
	/*
	 * Do not spawn a new process only to indicate that the request
	 * is illegal. We also handle obvious 'reload' indications here.
	 */
	Wt::Http::ParameterMap::const_iterator i = queryParams_.find("request");
	if (i != queryParams_.end()) {
	  if (i->second[0] == "resource" || i->second[0] == "style") {
	    LOG_INFO("resource request from dead session, not responding.");
	    error(not_found);
	    return true;
	  } else if (i->second[0] == "ws") {
	    LOG_INFO("websocket request from dead session, not responding.");
	    error(service_unavailable);
	    return true;
	  }
	} else if (request_.method.icontains("POST") &&
		   queryParams_.size() == 1) {
	  sendReload();
	  return true;
	}
      }

      if (sessionManager_.tryToIncrementSessionCount()) {
	fwCertificates_ = true;
	// Launch new child process
	sessionProcess_.reset(
	    new SessionProcess(connection()->server()->service()));

	sessionProcess_->asyncExec(
	    configuration(),
	    connection()->strand().wrap
	    (boost::bind
	     (&ProxyReply::connectToChild,
	      boost::dynamic_pointer_cast<ProxyReply>(shared_from_this())
	      , _1)));
	sessionManager_.addPendingSessionProcess(sessionProcess_);
      } else {
	LOG_ERROR("maximum amount of sessions reached!");
	error(service_unavailable);
      }
    } else {
      connectToChild(true);
    }
  }

  // Don't immediately consume more data, but do this when it has
  // been sent to child
  return false;
}

void ProxyReply::connectToChild(bool success)
{
  if (success) {
    socket_.reset(new asio::ip::tcp::socket(connection()->server()->service()));
    socket_->async_connect
      (sessionProcess_->endpoint(),
       connection()->strand().wrap
       (boost::bind(&ProxyReply::handleChildConnected,
		    boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
		    asio::placeholders::error)));
  } else {
    error(service_unavailable);
  }
}


void ProxyReply::handleChildConnected(const boost::system::error_code& ec)
{
  if (ec) {
    LOG_ERROR("error connecting to child: " << ec.message());
    error(service_unavailable);
    return;
  }

  assembleRequestHeaders();

  // Send any request data we already have
  std::ostream os(&requestBuf_);
  os.write(beginRequestBuf_, static_cast<std::streamsize>(endRequestBuf_ - beginRequestBuf_));

  asio::async_write
    (*socket_, requestBuf_,
     connection()->strand().wrap
     (boost::bind
      (&ProxyReply::handleDataWritten,
       boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
       asio::placeholders::error,
       asio::placeholders::bytes_transferred)));
}

void ProxyReply::assembleRequestHeaders()
{
  std::ostream os(&requestBuf_);
  os << request_.method << " " << request_.uri << " HTTP/1.1\r\n";
  bool establishWebSockets = false;
  std::string forwardedFor;
  std::string forwardedProto = request_.urlScheme;
  std::string forwardedPort;
  for (Request::HeaderList::const_iterator it = request_.headers.begin();
       it != request_.headers.end(); ++it) {
    if (it->name.iequals("Connection") || it->name.iequals("Keep-Alive") ||
	it->name.iequals("TE") || it->name.iequals("Transfer-Encoding")) {
      // Remove hop-by-hop header
    } else if (it->name.iequals("X-Forwarded-For") ||
	       it->name.iequals("Client-IP")) {
      const Wt::Configuration& wtConfiguration
	= connection()->server()->controller()->configuration();
      if (wtConfiguration.behindReverseProxy()) {
	forwardedFor = it->value.str() + ", ";
      }
    } else if (it->name.iequals("Upgrade")) {
      if (it->value.iequals("websocket")) {
	establishWebSockets = true;
      }
    } else if (it->name.iequals("X-Forwarded-Proto")) { 
      forwardedProto = it->value.str();
    } else if(it->name.iequals("X-Forwarded-Port")) {
      forwardedPort = it->value.str();
    } else if (it->name.length() > 0) {
      os << it->name << ": " << it->value << "\r\n";
  }
  }
  if (establishWebSockets) {
    os << "Connection: Upgrade\r\n";
    os << "Upgrade: websocket\r\n";
  } else {
    os << "Connection: close\r\n";
  }
  os << "X-Forwarded-For: " << forwardedFor << request_.remoteIP << "\r\n";
  os << "X-Forwarded-Proto: " <<  forwardedProto  << "\r\n";
  if(forwardedPort.size() > 0)
	os << "X-Forwarded-Port: " <<  forwardedPort << "\r\n";
  else
	os << "X-Forwarded-Port: " <<  request_.port << "\r\n";
  // Forward SSL Certificate to session only for first request
  if (request_.sslInfo() && fwCertificates_) {
    appendSSLInfo(request_.sslInfo(), os);
  }

  // Append redirect secret
  os << "Redirect-Secret: "
     <<  Wt::WServer::instance()->controller()->redirectSecret_ << "\r\n";
  os << "\r\n";

  fwCertificates_ = false;
}

void ProxyReply::appendSSLInfo(const Wt::WSslInfo* sslInfo, std::ostream& os) {
#ifdef WT_WITH_SSL
  os << "SSL-Client-Certificates: ";

  Wt::Json::Value val(Wt::Json::ObjectType);
  Wt::Json::Object &obj = val;

  Wt::WSslCertificate clientCert = sslInfo->clientCertificate();
  std::string pem = clientCert.toPem();

  obj["client-certificate"] = Wt::WString(pem); 

  Wt::Json::Value arrVal(Wt::Json::ArrayType);
  Wt::Json::Array &sslCertsArr = arrVal;
  for(unsigned int i = 0; i< sslInfo->clientPemCertificateChain().size(); ++i) {
	sslCertsArr.push_back(Wt::WString(sslInfo->clientPemCertificateChain()[i].toPem()));
  }

  obj["client-pem-certification-chain"] = arrVal;
  obj["client-verification-result-state"] = (int)sslInfo->clientVerificationResult().state();
  obj["client-verification-result-message"] = sslInfo->clientVerificationResult().message();

  os <<Wt::Utils::base64Encode(Wt::Json::serialize(obj), false);
  os << "\r\n";
#endif
}

void ProxyReply::handleDataWritten(const boost::system::error_code &ec,
				   std::size_t transferred)
{
  if (!ec) {
    if (state_ == Request::Partial) {
      requestBuf_.consume(transferred);
      LOG_DEBUG(this << ": receive() upstream");
      receive();
    } else {
      asio::async_read_until
	(*socket_, responseBuf_, "\r\n",
	 connection()->strand().wrap
	 (boost::bind
	  (&ProxyReply::handleStatusRead,
	   boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
	   asio::placeholders::error)));
    }
  } else {
    LOG_ERROR("error sending data to child: " << ec.message());
    if (!sendReload())
      error(service_unavailable);
  }
}

void ProxyReply::handleStatusRead(const boost::system::error_code &ec)
{
  if (!ec) {
    std::istream response_stream(&responseBuf_);
    std::string http_version;
    response_stream >> http_version;
    int status_code;
    response_stream >> status_code;
    setStatus((Reply::status_type) status_code);
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
      LOG_ERROR("got malformed response!");
      if (!sendReload())
	error(internal_server_error);
      return;
    }

    asio::async_read_until
      (*socket_, responseBuf_, "\r\n\r\n",
       connection()->strand().wrap
       (boost::bind(&ProxyReply::handleHeadersRead,
		    boost::dynamic_pointer_cast<ProxyReply>(shared_from_this()),
		    asio::placeholders::error)));
  } else {
    LOG_ERROR("error reading status line: " << ec.message());
    if (!sendReload())
      error(service_unavailable);
  }
}

void ProxyReply::handleHeadersRead(const boost::system::error_code &ec)
{
  if (ec) {
    LOG_ERROR("error reading headers: " << ec.message());
    if (!sendReload())
      error(service_unavailable);
    return;
  }

  bool webSocketStatus = status() == switching_protocols;
  bool webSocketConnection = false;
  bool webSocketUpgrade = false;

  std::istream response_stream(&responseBuf_);
  std::string header;

  while (std::getline(response_stream, header) && header != "\r") {
    std::size_t i = header.find(':');
    if (i != std::string::npos) {
      std::string name = boost::trim_copy(header.substr(0, i));
      std::string value = boost::trim_copy(header.substr(i+1));
      if (boost::iequals(name, "Content-Type")) {
	contentType_ = value;
      } else if (boost::iequals(name, "Content-Length")) { 
		contentLength_ = boost::lexical_cast<int64_t>(value);
	  } else if (boost::iequals(name, "Date")) {
	// Ignore, we're overriding it
      } else if (boost::iequals(name, "Transfer-Encoding") || boost::iequals(name, "Keep-Alive") ||
	  boost::iequals(name, "TE")) {
	// Remove hop-by-hop header
      } else if (boost::iequals(name, "Connection")) {
	// Remove hop-by-hop header
	if (boost::icontains(value, "Upgrade")) {
	  webSocketConnection = true;
	}
      } else if (boost::iequals(name, "X-Wt-Session")) {
	sessionManager_.addSessionProcess(value, sessionProcess_);
      } else if (boost::iequals(name, "Upgrade")) {
	// Remove hop-by-hop header
	if (boost::icontains(value, "websocket")) {
	  webSocketUpgrade = true;
	}
      } else {
	addHeader(name, value);
      }

      if (boost::iequals(name, "Transfer-Encoding") &&
	  boost::iequals(value, "chunked")) {
	// NOTE: Wt shouldn't return chunked encoding, because
	//	 we sent Connection: close.
	// If decoding is needed, look in Wt::Http::Client
	LOG_ERROR("unexpected chunked encoding!");
	if (!sendReload())
	  error(internal_server_error);
	return;
      }
    }
  }

  if (webSocketStatus && webSocketConnection && webSocketUpgrade) {
    addHeader("Connection", "Upgrade");
    addHeader("Upgrade", "websocket");
    setCloseConnection();
    request_.type = Request::TCP;

  }

  if (responseBuf_.size() > 0) {
    out_ << &responseBuf_;
  }

  send();
}

void ProxyReply::handleResponseRead(const boost::system::error_code &ec)
{
  LOG_DEBUG(this << ": async_read done.");

  if (!ec) {
    if (responseBuf_.size() > 0) {
      out_ << &responseBuf_;
    }

    send();
  } else if (ec == boost::asio::error::eof
	     || ec == boost::asio::error::shut_down
	     || ec == boost::asio::error::operation_aborted
	     || ec == boost::asio::error::connection_reset) {
    closeClientSocket();

    more_ = false;

    if (request_.type != Request::TCP) {
      send();
    }
  } else {
    LOG_ERROR("error reading response: " << ec.message());
    if (!sendReload()) 
      error(service_unavailable);
  }
}

std::string ProxyReply::getSessionId() const
{
  std::string sessionId;

  std::string wtd;
  Wt::Http::ParameterMap::const_iterator i = queryParams_.find("wtd");
  if (i != queryParams_.end())
    wtd = i->second[0];

  const Wt::Configuration& wtConfiguration
    = connection()->server()->controller()->configuration();

  if (wtConfiguration.sessionTracking() == Wt::Configuration::CookiesURL &&
      !wtConfiguration.reloadIsNewSession()) {
    const Request::Header *cookieHeader = request_.getHeader("Cookie");
    if (cookieHeader) {
      std::string cookie = cookieHeader->value.str();
      sessionId = Wt::WebController::sessionFromCookie
	(cookie.c_str(), request_.request_path,
	 wtConfiguration.sessionIdLength());
    }
  }

  if (sessionId.empty())
    sessionId = wtd;

  return sessionId;
}

std::string ProxyReply::contentType()
{
  return contentType_;
}

::int64_t ProxyReply::contentLength()
{
  return contentLength_;
}

bool ProxyReply::nextContentBuffers(std::vector<asio::const_buffer>& result)
{
  sending_ = out_buf_.size();
  if (sending_ > 0) {
    result.push_back(out_buf_.data());
  }
  return !more_;
}

void ProxyReply::error(status_type status)
{
  closeClientSocket();

  if (request_.type == Request::HTTP) {
    setStatus(status);
    setCloseConnection();
    more_ = false;
    setRelay(ReplyPtr(new StockReply(request_, status, configuration())));
    send();
  } else {
    connection()->close();
  }
}

bool ProxyReply::sendReload()
{
  bool jsRequest
    = request_.method.icontains("POST") && queryParams_.size() == 1;

  if (!jsRequest) {
    Wt::Http::ParameterMap::const_iterator i = queryParams_.find("request");
    if (i != queryParams_.end())
      jsRequest = i->second[0] == "script";
  }

  if (jsRequest) {
    LOG_INFO("signal from dead session, sending reload.");
    setStatus(ok);
    contentType_ = "text/javascript; charset=UTF-8";
    out_ <<
      "if (window.Wt) window.Wt._p_.quit(null); window.location.reload(true);";
    more_ = false;
    send();
    closeClientSocket();

    return true;
  }

  return false;
}

  } // namespace server
} // namespace http
