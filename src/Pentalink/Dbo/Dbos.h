#ifndef TP_DBOS_H
#define TP_DBOS_H

#include "Utilities/Common.h"
#include <Wt/Dbo/Dbo>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WDate>

#define TRANSACTION(app) Wt::Dbo::Transaction t(app->dboSession())

namespace PL
{
	void mapDboTree(Wt::Dbo::Session &dboSession);

	//User related
	class User;
	typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
	typedef Wt::Auth::Dbo::AuthIdentity<AuthInfo> AuthIdentity;
	typedef Wt::Auth::Dbo::AuthToken<AuthInfo> AuthToken;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AuthInfo>> AuthInfoCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<User>> UserCollection;

	typedef Wt::Auth::Dbo::UserDatabase<PL::AuthInfo> UserDatabase;
}

namespace Wt
{
	namespace Dbo
	{
		
	}
}

namespace PL
{
	
	class User
	{
	public:
		Wt::Dbo::weak_ptr<AuthInfo> authInfoWPtr;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::hasOne(a, authInfoWPtr, "user");
		}
		static const char *tableName()
		{
			return "user";
		}
	};

}

#endif
