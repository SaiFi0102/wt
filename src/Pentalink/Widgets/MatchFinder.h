#ifndef TP_MATCHFINDER_WIDGET_H
#define TP_MATCHFINDER_WIDGET_H

#include "Dbo/Dbos.h"
#include "Application/MatchServer.h"

#include <Wt/WTemplateFormView>
#include <Wt/WFormModel>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>

namespace PL
{
	struct ClientMatch
	{
		ClientMatch() = default;
		ClientMatch(Match *serverMatch) : id(serverMatch->matchId), name(serverMatch->matchName) { }
		long long id = -1;
		Wt::WString name;
	};
	typedef std::vector<ClientMatch> ClientMatchVector;

	class MatchFinder : public Wt::WTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field modeField;
		static const Wt::WFormModel::Field nPlayersField;

		MatchFinder(Wt::WContainerWidget *parent = nullptr);
		virtual ~MatchFinder() override;

	protected:
		typedef std::vector<Wt::Dbo::ptr<AuthInfo>> AuthInfoVector;

		//UI stuff
		Wt::WPushButton *createMatchButton(const Wt::WString &matchName);
		Wt::WPushButton *createPlayerButton(Wt::Dbo::ptr<AuthInfo> authInfoPtr);
		Wt::WTemplate *createMatchPlayerTemplate(Wt::Dbo::ptr<AuthInfo> authInfoPtr);
		void setMatchView();
		void setFormView();

		//Client event Handlers
		void createClicked();
		void leaveClicked();
		void matchClicked(Wt::WPushButton *btn);
		void playerClicked(Wt::WPushButton *btn);

		//Server event handlers
		//Basic stuff
		static void _connected(const MatchVector &matchVector, const MatchMakingServer::IdVector &onlinePlayerAutoInfoIds);
		static void _disconnected();

		//Player list
		static void _playerConnected(long long authInfoId);
		static void _playerDisconnected(long long authInfoId);

		//Match list
		static void _matchAdded(Match *match);
		static void _matchRemoved(long long matchId);

		//Current match
		static void _matchJoined(Match *match);
		static void _matchLeft();
		static void _matchPlayerAdded(long long authInfoId);
		static void _matchPlayerRemoved(long long authInfoId);

		bool _isConnected = false;
		ClientMatch _currentMatch;
		ClientMatchVector _matchList;
		AuthInfoVector _matchPlayerList;
		AuthInfoVector _onlinePlayerList;

		Wt::WContainerWidget *_matchesContainer = nullptr;
		Wt::WContainerWidget *_onlinePlayersContainer = nullptr;
		Wt::WContainerWidget *_matchPlayersContainer = nullptr;
		Wt::WFormModel *_formModel = nullptr;

	private:
		friend class MatchMakingServer;
	};
}

#endif