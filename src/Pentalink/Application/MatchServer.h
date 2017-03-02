#ifndef TP_MATCHSERVER_H
#define TP_MATCHSERVER_H

#include <boost/thread/mutex.hpp>
#include <Wt/Dbo/Session>
#include <Wt/WString>


namespace PL
{
	struct Match;
	struct Client;
	typedef std::vector<Client*> ClientVector;
	typedef std::vector<Match*> MatchVector;
	
	struct Client
	{
		Client(const std::string &sessionId, long long authInfoId) : sessionId(sessionId), authInfoId(authInfoId) { }
		std::string sessionId;
		long long authInfoId;
		Match *currentMatch = 0;
	};
	struct Match
	{
		Match(long long matchId) : matchId(matchId) { }
		long long matchId;
		Wt::WString matchName;
		ClientVector clientList;
	};

	class MatchMakingServer
	{
	public:
		typedef std::vector<long long> IdVector;

		MatchMakingServer(Wt::Dbo::Session &session);
		~MatchMakingServer();
		void connect();
		void disconnect();
		void createMatch();
		void joinMatch(long long matchId);
		void leaveMatch();

	protected:
		typedef std::map<std::string, Client*> SessionToClientMap;
		typedef std::map<long long, Client*> AuthInfoToClientMap;

		void _connect(const std::string &sessionId, long long authInfoId);
		void _disconnect(const std::string &sessionId);
		void _createMatch(const std::string &sessionId, const Wt::WString &matchName);
		void _joinMatch(const std::string &sessionId, long long matchId);
		void _leaveMatch(const std::string &sessionId);

		SessionToClientMap _sessionClientMap;
		AuthInfoToClientMap _authInfoClientMap;
		MatchVector _matchList;
		long long _nextMatchId = -1;
		Wt::Dbo::Session &dboSession;

	private:
		mutable boost::mutex _mutex;
	};
}

#endif