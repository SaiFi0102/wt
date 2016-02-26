#ifndef GS_ENTITIESDATABASE_H
#define GS_ENTITIESDATABASE_H

#include "Dbo/Dbos.h"

namespace GS
{
	class EntitiesDatabase
	{
	public:
		EntitiesDatabase(Wt::Dbo::Session &session) : dboSession(session) { }

		Wt::Dbo::ptr<Entity> findOrCreateSelfEntity(); //MAY FLUSH

	protected:
		Wt::Dbo::Session &dboSession;
	};
}

#endif