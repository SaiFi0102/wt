#ifndef GS_ACCOUNTSDATABASE_H
#define GS_ACCOUNTSDATABASE_H

#include "Dbo/Dbos.h"
#include "Dbo/EntitiesDatabase.h"

namespace GS
{
	class AccountsDatabase
	{
	public:
		AccountsDatabase(Wt::Dbo::Session &session) : dboSession(session), _entitiesDatabase(dboSession) { }

		Wt::Dbo::ptr<Account> findOrCreateSelfAccount(); //MAY FLUSH
		Wt::Dbo::ptr<Account> findOrCreateEntityAccount(Wt::Dbo::ptr<Entity> entityPtr);

		void createPendingCycleEntry(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime);
		void createPendingCycleEntry(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime, Wt::Dbo::ptr<Account> selfAccountPtr, boost::posix_time::time_duration *nextEntryDuration = nullptr);
		void createPendingCycleEntry(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime);
		void createPendingCycleEntry(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime, Wt::Dbo::ptr<Account> selfAccountPtr, boost::posix_time::time_duration *nextEntryDuration = nullptr);

	protected:
		void initEntityAccountValues(Wt::Dbo::ptr<Account> accountPtr, Wt::Dbo::ptr<Entity> entityPtr);
		boost::posix_time::ptime addInterval(boost::posix_time::ptime pTime, CycleInterval interval, int nIntervals) const;

		Wt::Dbo::Session &dboSession;
		EntitiesDatabase _entitiesDatabase;

	private:
		Wt::Dbo::ptr<AccountEntry> _createPendingCycleEntry(
			const EntryCycle &cycle,
			Wt::Dbo::ptr<AccountEntry> lastEntryPtr,
			boost::posix_time::ptime currentPTime,
			boost::posix_time::time_duration *nextEntryDuration = nullptr);
	};

}

#endif