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
	}
}
