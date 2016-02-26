#include <iostream>
#include "Application/WServer.h"
#include "Application/WApplication.h"

int main(int argc, char **argv)
{
	/* *************************************************************************
	* *************************  Start Web Server *****************************
	* *************************************************************************/
	// 	try
	// 	{
	//Initialize Server
	GS::WServer Server(argv[0], "D:/Programming/GhaziSecurity/src/GhaziSecurity/resources/approot/wt_config.xml");

	//Configuration
	Server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
	Server.addEntryPoint(Wt::Application, GS::WApplication::createApplication);

	//Start Server
	if(Server.start())
	{
		//And wait till a shutdown signal is given
		int sig = GS::WServer::waitForShutdown(argv[0]);
		Server.stop();
		Wt::log("info") << "Shutdown (Signal = " << sig << ")";
	}
	// 	}
	// 	catch(Wt::WServer::Exception &e)
	// 	{
	// 		Wt::log("fatal") << "Error starting the server: " << e.what();
	// 		exit(EXIT_FAILURE);
	// 	}
	// 	catch(std::exception &e)
	// 	{
	// 		Wt::log("fatal") << "Server exception error: " << e.what();
	// 		exit(EXIT_FAILURE);
	// 	}
}