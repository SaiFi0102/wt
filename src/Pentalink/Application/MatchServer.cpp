#include "Application/MatchServer.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Widgets/MatchFinder.h"

#include <Wt/WLogger>
#include <boost/thread/lock_guard.hpp>

//boost::lock_guard<boost::mutex> lock(_mutex); //lock

namespace PL
{

	MatchMakingServer::MatchMakingServer(Wt::Dbo::Session &session)
		: dboSession(session)
	{
		
	}

	MatchMakingServer::~MatchMakingServer()
	{
		for(const auto &it : _sessionClientMap)
			delete it.second;
	}

	void MatchMakingServer::connect()
	{
		WApplication *app = APP;
		_connect(app->sessionId(), app->authLogin().authInfoPtr().id());
	}

	void MatchMakingServer::disconnect()
	{
		WApplication *app = APP;
		_disconnect(app->sessionId());
	}

	void MatchMakingServer::createMatch()
	{
		WApplication *app = APP;
		TRANSACTION(app);
		_createMatch(app->sessionId(), app->authLogin().authInfoPtr()->identity(Wt::Auth::Identity::LoginName));
	}

	void MatchMakingServer::joinMatch(long long matchId)
	{
		WApplication *app = APP;
		_joinMatch(app->sessionId(), matchId);
	}

	void MatchMakingServer::leaveMatch()
{
		WApplication *app = APP;
		_leaveMatch(app->sessionId());
	}

	void MatchMakingServer::_connect(const std::string &sessionId, long long authInfoId)
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		Client *newClient = new Client(sessionId, authInfoId);
		_sessionClientMap.insert(std::make_pair(sessionId, newClient));
		_authInfoClientMap.insert(std::make_pair(authInfoId, newClient));
		
		//Copy authInfoIds only into a vector
		IdVector onlineAuthInfoIds;
		for(const auto &c : _sessionClientMap)
			onlineAuthInfoIds.push_back(c.second->authInfoId);

		//Notify client that is connected
		SERVER->post(sessionId, boost::bind(&MatchFinder::_connected, _matchList, onlineAuthInfoIds));

		//Broadcast to all other clients about new client
		for(const auto &c : _sessionClientMap)
		{
			if(c.second->sessionId != sessionId)
				SERVER->post(c.second->sessionId, boost::bind(&MatchFinder::_playerConnected, authInfoId));
		}
		
	}

	void MatchMakingServer::_disconnect(const std::string &sessionId)
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		Client *client = fItr->second;
		if(client->currentMatch)
			_leaveMatch(sessionId);

		_sessionClientMap.erase(sessionId);
		_authInfoClientMap.erase(client->authInfoId);

		//Notify client that it is disconnected
		SERVER->post(sessionId, boost::bind(&MatchFinder::_disconnected));

		//Broadcast to all other clients that a client has been disconnected	
		for(const auto &it : _sessionClientMap)
			SERVER->post(it.first, boost::bind(&MatchFinder::_playerDisconnected, client->authInfoId));

		delete client;
	}

	void MatchMakingServer::_createMatch(const std::string &sessionId, const Wt::WString &matchName)
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		Client *creatorClient = fItr->second;
		Match *newMatch = new Match(++_nextMatchId);
		newMatch->matchName = APP->authLogin().authInfoPtr()->identity(Wt::Auth::Identity::LoginName);
		newMatch->clientList.push_back(creatorClient);
		_matchList.push_back(newMatch);
		creatorClient->currentMatch = newMatch;

		//Notify client
		SERVER->post(sessionId, boost::bind(&MatchFinder::_matchJoined, newMatch));

		//Broadcast
		for(const auto &c : _sessionClientMap)
			SERVER->post(c.second->sessionId, boost::bind(&MatchFinder::_matchAdded, newMatch));
	}

	void MatchMakingServer::_joinMatch(const std::string &sessionId, long long matchId)
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		Client *client = fItr->second;
		if(client->currentMatch)
			return;

		Match *match = 0;
		for(const auto &c : _matchList)
		{
			if(c->matchId == matchId)
			{
				match = c;
				break;
			}
		}
		if(!match)
			return;

		match->clientList.push_back(client);
		client->currentMatch = match;

		//Notify client
		SERVER->post(sessionId, boost::bind(&MatchFinder::_matchJoined, match));

		//Broadcast to all other joined players
		for(const auto &c : match->clientList)
		{
			if(c != client)
				SERVER->post(c->sessionId, boost::bind(&MatchFinder::_matchPlayerAdded, client->authInfoId));
		}
	}

	void MatchMakingServer::_leaveMatch(const std::string &sessionId)
	{
		boost::lock_guard<boost::mutex> lock(_mutex);

		SessionToClientMap::const_iterator fItr = _sessionClientMap.find(sessionId);
		if(fItr == _sessionClientMap.end())
			return;

		Client *client = fItr->second;
		Match *match = client->currentMatch;
		if(!match)
			return;

		//Find client in match
		ClientVector::const_iterator matchClientItr = std::find(match->clientList.begin(), match->clientList.end(), client);
		if(matchClientItr == match->clientList.end())
		{
			client->currentMatch = 0;
			return;
		}

		//Remove player and broadcast to players in the match
		match->clientList.erase(matchClientItr);
		client->currentMatch = 0;
		for(const auto &c : match->clientList)
			SERVER->post(c->sessionId, boost::bind(&MatchFinder::_matchPlayerRemoved, client->authInfoId));
		
		//If no clients left, remove the match
		if(match->clientList.empty())
		{
			//Broadcast match removed
			for(const auto &c : _sessionClientMap)
				SERVER->post(c.second->sessionId, boost::bind(&MatchFinder::_matchRemoved, match->matchId));

			MatchVector::const_iterator matchListItr = std::find(_matchList.begin(), _matchList.end(), match);
			_matchList.erase(matchListItr);
			delete match;
		}

		//Notify client
		SERVER->post(sessionId, boost::bind(&MatchFinder::_matchLeft));
	}

}

