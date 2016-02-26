#include "Dbo/AccountsDatabase.h"
#include <Wt/WLogger>

namespace GS
{

	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateSelfAccount()
	{
		Wt::Dbo::Transaction t(dboSession);
		auto selfEntity = _entitiesDatabase.findOrCreateSelfEntity();
		if(!selfEntity)
			return Wt::Dbo::ptr<Account>();

		Wt::Dbo::ptr<Account> selfAccount = selfEntity->accountWPtr;
		if(!selfAccount)
		{
			Wt::log("warn") << "Self account was not found, creating self account";
			selfAccount = dboSession.add(new Account());
			initEntityAccountValues(selfAccount, selfEntity);
		}

		t.commit();
		return selfAccount;
	}

	Wt::Dbo::ptr<Account> AccountsDatabase::findOrCreateEntityAccount(Wt::Dbo::ptr<Entity> entityPtr)
	{
		if(!entityPtr)
			return Wt::Dbo::ptr<Account>();

		Wt::Dbo::Transaction t(dboSession);
		Wt::Dbo::ptr<Account> accountPtr = entityPtr->accountWPtr.query();
		if(!accountPtr)
		{
			accountPtr = dboSession.add(new Account());
			initEntityAccountValues(accountPtr, entityPtr);
		}
		t.commit();
		return accountPtr;
	}

	void AccountsDatabase::initEntityAccountValues(Wt::Dbo::ptr<Account> accountPtr, Wt::Dbo::ptr<Entity> entityPtr)
	{
		accountPtr.modify()->entityPtr = entityPtr;
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime)
	{
		Wt::Dbo::Transaction t(dboSession);
		Wt::Dbo::ptr<Account> selfAccountPtr = findOrCreateSelfAccount();
		createPendingCycleEntry(cyclePtr, lastEntryPtr, currentPTime, selfAccountPtr);
		t.commit();
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime)
	{
		Wt::Dbo::Transaction t(dboSession);
		Wt::Dbo::ptr<Account> selfAccountPtr = findOrCreateSelfAccount();
		createPendingCycleEntry(cyclePtr, lastEntryPtr, currentPTime, selfAccountPtr);
		t.commit();
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime, Wt::Dbo::ptr<Account> selfAccountPtr, boost::posix_time::time_duration *nextEntryDuration)
	{
		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(currentPTime.is_special())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentPTime was invalid");
		if(!selfAccountPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): selfAccountPtr was null");

		Wt::Dbo::Transaction t(dboSession);

		if(lastEntryPtr && lastEntryPtr->incomeCyclePtr && lastEntryPtr->incomeCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->incomeCyclePtr != cyclePtr");

		while(Wt::Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentPTime, nextEntryDuration))
		{
			newEntry.modify()->incomeCyclePtr = cyclePtr;
			newEntry.modify()->debitAccountPtr = selfAccountPtr;
			newEntry.modify()->creditAccountPtr = cyclePtr->accountPtr;
			newEntry.modify()->type = AccountEntry::UnspecifiedType;

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	void AccountsDatabase::createPendingCycleEntry(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::Dbo::ptr<AccountEntry> lastEntryPtr, boost::posix_time::ptime currentPTime, Wt::Dbo::ptr<Account> selfAccountPtr, boost::posix_time::time_duration *nextEntryDuration)
	{
		if(!cyclePtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): cyclePtr was null");
		if(currentPTime.is_special())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): currentPTime was invalid");
		if(!selfAccountPtr)
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): selfAccountPtr was null");
		
		Wt::Dbo::Transaction t(dboSession);

		if(lastEntryPtr && lastEntryPtr->expenseCyclePtr && lastEntryPtr->expenseCyclePtr.id() != cyclePtr.id())
			throw std::logic_error("AccountsDatabase::createPendingCycleEntry(): lastEntryPtr->expenseCyclePtr != cyclePtr");

		while(Wt::Dbo::ptr<AccountEntry> newEntry = _createPendingCycleEntry(*cyclePtr, lastEntryPtr, currentPTime, nextEntryDuration))
		{
			newEntry.modify()->expenseCyclePtr = cyclePtr;
			newEntry.modify()->debitAccountPtr = cyclePtr->accountPtr;
			newEntry.modify()->creditAccountPtr = selfAccountPtr;
			newEntry.modify()->type = AccountEntry::UnspecifiedType;

			lastEntryPtr = newEntry;
		}

		t.commit();
	}

	Wt::Dbo::ptr<AccountEntry> AccountsDatabase::_createPendingCycleEntry(
		const EntryCycle &cycle,
		Wt::Dbo::ptr<AccountEntry> lastEntryPtr,
		boost::posix_time::ptime currentPTime,
		boost::posix_time::time_duration *nextEntryDuration)
	{
		Wt::WDateTime currentDt(currentPTime);

		//DO NOT ALLOW THESE NULL DATES(in scan query)
		if(cycle.startDate.isNull() || cycle.creationDt.isNull())
			return Wt::Dbo::ptr<AccountEntry>();

		//Do not allow cycles with start dates later than today and creation dt later than now(already filtered, yet asserted)
		if(cycle.startDate > currentDt.date() || cycle.creationDt > currentDt)
			return Wt::Dbo::ptr<AccountEntry>();

		//Do not allow cycles with end dates earlier or equal to start date
		if(cycle.endDate.isValid() && cycle.endDate <= cycle.startDate)
			return Wt::Dbo::ptr<AccountEntry>();

		//Do not allow invalid interval or nIntervals
		if(cycle.interval < DailyInterval || cycle.interval > YearlyInterval)
			return Wt::Dbo::ptr<AccountEntry>();
		if(cycle.nIntervals < 1)
			return Wt::Dbo::ptr<AccountEntry>();

		if(lastEntryPtr)
		{
			//Do not allow cycles which have ended and the final entry had already been made(already filtered, yet asserted)
			if(cycle.endDate.isValid() && cycle.endDate <= lastEntryPtr->timestamp.date())
				return Wt::Dbo::ptr<AccountEntry>();

			//Do not allow entries with invalid date(in scan query)
			if(lastEntryPtr->timestamp.isNull() || lastEntryPtr->timestamp > currentDt)
				return Wt::Dbo::ptr<AccountEntry>();
		}

		//BEGIN
		bool checkElapsedDuration = false;
		if(lastEntryPtr || cycle.firstEntryAfterCycle)
			checkElapsedDuration = true;

		bool createEntry = false;
		bool incompleteDurationEntry = false;

		boost::posix_time::ptime previousCyclePeriodPTime; //either startDate or lastEntry timestamp
		boost::posix_time::ptime nextCyclePeriodPTime;
		boost::posix_time::time_duration cycleDuration;
		boost::posix_time::time_duration elapsedDuration;

		if(checkElapsedDuration)
		{
			if(lastEntryPtr)
			{
				previousCyclePeriodPTime = lastEntryPtr->timestamp.toPosixTime();
				elapsedDuration = currentPTime - previousCyclePeriodPTime;
			}
			else if(cycle.firstEntryAfterCycle)
			{
				previousCyclePeriodPTime = boost::posix_time::ptime(cycle.startDate.toGregorianDate());
				elapsedDuration = currentPTime - previousCyclePeriodPTime;
			}
			else
			{
				Wt::log("error") << "AccountsDatabase::_createPendingCycleEntry() logic error: lastEntryPtr AND cycle.firstEntryAfterCycle were both false!";
				return Wt::Dbo::ptr<AccountEntry>();
			}

			nextCyclePeriodPTime = addInterval(previousCyclePeriodPTime, cycle.interval, cycle.nIntervals);

			cycleDuration = nextCyclePeriodPTime - previousCyclePeriodPTime;
			if(elapsedDuration >= cycleDuration) //complete cycle
			{
				createEntry = true;

				//Next entry duration
				if(nextEntryDuration && (nextEntryDuration->is_special() || *nextEntryDuration > cycleDuration))
					*nextEntryDuration = cycleDuration;
			}
			else
			{ //incomplete duration
				//Create incomplete duration entry ONLY IF the cycle has ended
				//no entry has been made after the cycle.endDt: IS ASSERTED previously
				if(cycle.endDate.isValid() && currentDt.date() >= cycle.endDate)
				{
					createEntry = true;
					incompleteDurationEntry = true;
				}
				else
				{
					createEntry = false;

					//Next entry duration
					auto remainingDuration = cycleDuration - elapsedDuration;
					if(nextEntryDuration && (nextEntryDuration->is_special() || *nextEntryDuration > remainingDuration))
						*nextEntryDuration = remainingDuration;
				}
			}
		}
		else
		{
			nextCyclePeriodPTime = currentPTime;
			createEntry = true;
		}

		if(!createEntry)
			return Wt::Dbo::ptr<AccountEntry>();

		Wt::Dbo::ptr<AccountEntry> newEntry = dboSession.add(new AccountEntry());
		newEntry.modify()->timestamp = Wt::WDateTime(nextCyclePeriodPTime);

		if(incompleteDurationEntry)
		{
			boost::gregorian::days cycleDays = nextCyclePeriodPTime.date() - previousCyclePeriodPTime.date();
			boost::gregorian::days incompleteDays = cycle.endDate.toGregorianDate() - previousCyclePeriodPTime.date();
			double ratio = static_cast<double>(incompleteDays.days()) / cycleDays.days();
			newEntry.modify()->amount = cycle.amount * ratio;
		}
		else
		{
			newEntry.modify()->amount = cycle.amount;
		}

		return newEntry;
	}

	boost::posix_time::ptime AccountsDatabase::addInterval(boost::posix_time::ptime pTime, CycleInterval interval, int nIntervals) const
	{
		if(interval == DailyInterval)
			pTime += boost::gregorian::days(nIntervals);
		else if(interval == WeeklyInterval)
			pTime += boost::gregorian::weeks(nIntervals);
		else if(interval == MonthlyInterval)
			pTime += boost::gregorian::months(nIntervals);
		else if(interval == YearlyInterval)
			pTime += boost::gregorian::years(nIntervals);

		return pTime;
	}

}