#include "Dbos.h"
#include <Wt/Dbo/Session>

namespace PL
{
	void mapDboTree(Wt::Dbo::Session &dboSession)
	{
		dboSession.mapClass<User>(User::tableName());
		dboSession.mapClass<AuthInfo>("auth_info");
		dboSession.mapClass<AuthIdentity>("auth_identity");
		dboSession.mapClass<AuthToken>("auth_token");
		dboSession.mapClass<Game>(Game::tableName());
		dboSession.mapClass<Player>(Player::tableName());
		dboSession.mapClass<GameMove>(GameMove::tableName());
	}

	std::ostream &operator<<(std::ostream &o, const PlayerPK &k)
	{
		return o << "(" << k.gamePtr << ", " << k.userPtr << ")";
	}
}

Wt::Dbo::dbo_traits<PL::Player>::IdType Wt::Dbo::dbo_traits<PL::Player>::invalidId()
{
	return IdType();
}