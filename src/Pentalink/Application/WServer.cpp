#include "Application/WServer.h"
#include "Application/MatchServer.h"
#include "Dbo/Dbos.h"

#include <Wt/WMessageResourceBundle>
#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/Dbo/backend/MySQL>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/Exception>

#include <Wt/Auth/HashFunction>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/GoogleService>
#include <Wt/Auth/FacebookService>
#include <Wt/Auth/Dbo/UserDatabase>

namespace PL
{

WServer::WServer(int argc, char *argv[], const std::string &wtConfigurationFile)
	: Wt::WServer(argv[0], wtConfigurationFile), _passwordService(_authService)
{
	_ptBeforeLoad = boost::posix_time::microsec_clock::local_time();
	setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
}

void WServer::initialize()
{
	auto resolver = new Wt::WMessageResourceBundle();
	resolver->use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE
	setLocalizedStrings(resolver);

	/* *************************************************************************
	* ***********************  Connect to SQL Server  *************************
	* *************************************************************************/
	try
	{
		log("gs-info") << "Connecting to database backend";

		std::string dbBackend;
		if(!readConfigurationProperty("DbBackend", dbBackend))
			dbBackend = "SQLite";

		Wt::Dbo::SqlConnection *sqlConnection;
		if(dbBackend == "MySQL")
		{
			std::string host, portStr, db, user, password;
			readConfigurationProperty("MySQLHost", host);
			readConfigurationProperty("MySQLPort", portStr);
			readConfigurationProperty("MySQLDb", db);
			readConfigurationProperty("MySQLUser", user);
			readConfigurationProperty("MySQLPassword", password);

			unsigned int port = 0;
			if(!portStr.empty())
				port = boost::lexical_cast<unsigned int>(portStr);

			sqlConnection = new Wt::Dbo::backend::MySQL(db, user, password, host, port);
		}
		else if(dbBackend == "SQLite")
		{
			std::string db = ":memory:";
			readConfigurationProperty("SQLiteDb", db);
			sqlConnection = new Wt::Dbo::backend::Sqlite3(db);
		}
		else
		{
			throw std::runtime_error("Invalid 'DbBackend' configuration: \"" + dbBackend + "\"");
		}

		sqlConnection->setProperty("show-queries", "true");
		_sqlPool = new Wt::Dbo::FixedSqlConnectionPool(sqlConnection, 1);

		log("success") << "Successfully connected to database";
	}
	catch(const Wt::Dbo::Exception &e)
	{
		log("fatal") << "Database error connecting to database: " << e.what();
		throw;
	}
	catch(const std::exception &e)
	{
		log("fatal") << "Error connecting to database: " << e.what();
		throw;
	}

	//Prepare server's Dbo::Session
	_dboSession.setConnectionPool(*_sqlPool);
	mapDboTree(_dboSession);

	//Game Servers
	_matchMakingServer = new MatchMakingServer(_dboSession);

	//Configure authorization module
	configureAuth();

	/* *************************************************************************
	* ***************************  Create Tables  *****************************
	* *************************************************************************/

	std::string reinstallDb;
	readConfigurationProperty("ReinstallDb", reinstallDb);
	if(reinstallDb == "true")
	{
		//Drop
		try
		{
			_dboSession.dropTables();
		}
		catch(const Wt::Dbo::Exception &e)
		{
			log("error") << "Database error dropping tables: " << e.what();
		}
		catch(const std::exception &e)
		{
			log("error") << "Error dropping tables: " << e.what();
		}

		//Create
		try
		{
			Wt::Dbo::Transaction t(_dboSession);
			_dboSession.createTables();
			t.commit();
		}
		catch(const Wt::Dbo::Exception &e)
		{
			log("fatal") << "Database error creating tables: " << e.what();
			throw;
		}
		catch(const std::exception &e)
		{
			log("fatal") << "Error creating tables: " << e.what();
			throw;
		}

		try
		{
			Wt::Dbo::Transaction tr(_dboSession);



			tr.commit();
		}
		catch(const Wt::Dbo::Exception &e)
		{
			log("fatal") << "Database error inserting default values: " << e.what();
			throw;
		}
		catch(const std::exception &e)
		{
			log("fatal") << "Error inserting default values: " << e.what();
			throw;
		}
	}
}

WServer::~WServer()
{
	for(OAuthServiceMap::size_type i = 0; i < _oAuthServices.size(); ++i)
		delete _oAuthServices[i];

	delete _matchMakingServer;
	delete _sqlPool; //Also deletes SQLConnections
	delete localizedStrings();
}

bool WServer::start()
{
	if(Wt::WServer::start())
	{
		//Load Finish Time
		_ptStart = boost::posix_time::microsec_clock::local_time();

		log("success") << "Server successfully started! Time taken to start: "
			<< boost::posix_time::time_duration(_ptStart - _ptBeforeLoad).total_milliseconds()
			<< " ms";
		return true;
	}
	return false;
}

void WServer::configureAuth()
{
	_authService.setAuthTokensEnabled(true, "authtoken");
	_authService.setAuthTokenValidity(7 * 24 * 60);
	_authService.setIdentityPolicy(Wt::Auth::LoginNameIdentity);
	_authService.setEmailVerificationEnabled(false);

	//Hash and throttling
	Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
	verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
	_passwordService.setVerifier(verifier);
	_passwordService.setAttemptThrottlingEnabled(true);

	//Password strength
	Wt::Auth::PasswordStrengthValidator *strengthValidator = new Wt::Auth::PasswordStrengthValidator();
	strengthValidator->setMinimumLength(Wt::Auth::PasswordStrengthValidator::OneCharClass, 5);
	_passwordService.setStrengthValidator(strengthValidator);

// 	if(Wt::Auth::GoogleService::configured() && configurations()->getBool("GoogleOAuth", ModuleDatabase::Authentication, false))
// 		_oAuthServices.push_back(new Wt::Auth::GoogleService(_authService));
// 	if(Wt::Auth::FacebookService::configured() && configurations()->getBool("FacebookOAuth", ModuleDatabase::Authentication, false))
// 		_oAuthServices.push_back(new Wt::Auth::FacebookService(_authService));
}


}
