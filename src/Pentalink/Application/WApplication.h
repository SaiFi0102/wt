#ifndef TP_WAPPLICATION_H
#define TP_WAPPLICATION_H

#include <Wt/WApplication>
#include <Wt/Auth/Login>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/WDialog>

#include "Dbo/Dbos.h"

namespace PL
{
	class AuthWidget;

	class AuthLogin : public Wt::Auth::Login
	{
	public:
		AuthLogin();
		Wt::Dbo::ptr<AuthInfo> authInfoPtr() const { return _authInfoPtr; };
		Wt::Dbo::ptr<User> userPtr() const { return _authInfoPtr ? _authInfoPtr->user() : Wt::Dbo::ptr<User>(); }

	protected:
		void handleBeforeLoginChanged();
		Wt::Dbo::ptr<AuthInfo> _authInfoPtr;
	};

	class WApplication : public Wt::WApplication
	{
	public:
		WApplication(const Wt::WEnvironment& env);
		//virtual ~WApplication() override;

		static WApplication *instance() { return dynamic_cast<WApplication*>(Wt::WApplication::instance()); }
		static WApplication *createApplication(const Wt::WEnvironment &env) { return new WApplication(env); }

		Wt::Dbo::Session &dboSession() { return _dboSession; }
		AuthLogin &authLogin() { return _login; }
		UserDatabase &userDatabase() { return _userDatabase; }

		//Widgets
		Wt::WTemplate *mainTemplate() const { return _mainTemplate; }
		Wt::WNavigationBar *mainNavBar() const { return _navBar; }
		Wt::WMenu *mainMenu() const { return _mainMenu; }
		Wt::WStackedWidget *mainStack() const { return _mainStack; }

		//Error handling
		Wt::WDialog *errorDialog() const { return _errorDialog; }
		void showErrorDialog(const Wt::WString &message);
		void showStaleObjectError() { showErrorDialog(Wt::WString::tr("StaleObjectError")); }
		void showDbBackendError(const std::string &code);

	protected:
		void handleAuthChanged();
		void handleInternalPathChanged(std::string path);

		void lazyLoadWidgets();
		void lazyLoadLoginWidget();

		//Widgets
		Wt::WTemplate *_mainTemplate = nullptr;
		Wt::WStackedWidget *_mainStack = nullptr;
		Wt::WNavigationBar *_navBar = nullptr;
		Wt::WMenu *_mainMenu = nullptr;

		AuthWidget *_authWidget = nullptr;

		Wt::WDialog *_errorDialog = nullptr;
		Wt::WText *_errorDialogText = nullptr;

		Wt::Dbo::Session _dboSession;
		AuthLogin _login;
		UserDatabase _userDatabase;

		boost::posix_time::ptime _startTime;
	};
}

#define APP WApplication::instance()

#endif