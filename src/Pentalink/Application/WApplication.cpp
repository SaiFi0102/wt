#include "Application/WApplication.h"
#include "Application/WServer.h"

#include "Widgets/AuthWidget.h"
#include "Widgets/MatchFinder.h"

#include <Wt/WNavigationBar>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WBootstrapTheme>
#include <Wt/WPushButton>
#include <Wt/WLoadingIndicator>
#include <Wt/WSuggestionPopup>
#include <web/WebUtils.h>


namespace PL
{

AuthLogin::AuthLogin()
{
	handleBeforeLoginChanged();
	beforeChanged_.connect(this, &AuthLogin::handleBeforeLoginChanged);
}

void AuthLogin::handleBeforeLoginChanged()
{
	WApplication *app = APP;

	if(loggedIn())
		_authInfoPtr = app->userDatabase().find(user());
	else
		_authInfoPtr = Wt::Dbo::ptr<AuthInfo>();
		
	//Make sure user entry for authinfo exists
	if(_authInfoPtr && !_authInfoPtr->user())
	{
		TRANSACTION(app);
		_authInfoPtr.modify()->setUser(app->dboSession().add(new User()));
	}
}

WApplication::WApplication(const Wt::WEnvironment& env)
	: Wt::WApplication(env),
	_startTime(boost::posix_time::microsec_clock::local_time()), _userDatabase(_dboSession), _login()
{
	messageResourceBundle().use(appRoot() + "templates", false); //CHECK_BEFORE_RELEASE
	messageResourceBundle().use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE

	WServer *server = SERVER;
	_dboSession.setConnectionPool(*server->sqlPool());
	mapDboTree(_dboSession);

	setTitle(Wt::WString::tr("PL.Title"));
	styleSheet().addRule("div.Wt-loading",
		"position:absolute;top:0px;right:0;z-index:9999999;background:#457044;padding:10px 15px;color:#fff;border-radius:0 0 0 3px;"
		"-webkit-box-shadow: -1px 1px 2px 0px #000;"
		"-moz-box-shadow: -1px 1px 2px 0px #000;"
		"box-shadow: -1px 1px 2px 0px #000;");

	Wt::WLocale newLocale("en");
	newLocale.setDateFormat(Wt::WString::tr("DateFormat"));
	newLocale.setTimeFormat(Wt::WString::tr("TimeFormat"));
	newLocale.setDateTimeFormat(Wt::WString::tr("DateTimeFormat"));
	newLocale.setDecimalPoint(Wt::WString::tr("DecimalPoint").toUTF8());
	newLocale.setGroupSeparator(Wt::WString::tr("GroupSeparator").toUTF8());
	setLocale(newLocale);

	useStyleSheet(Wt::WLink("style.css"));
	useStyleSheet(Wt::WLink(resourcesUrl() + "font-awesome/css/font-awesome.min.css"));

	Wt::WBootstrapTheme *theme = new Wt::WBootstrapTheme();
	theme->setVersion(Wt::WBootstrapTheme::Version3);
	setTheme(theme);

	//Error Dialog
	_errorDialog = new Wt::WDialog(Wt::WString::tr("AnErrorOccurred"), this);
	_errorDialog->setTransient(true);
	_errorDialog->rejectWhenEscapePressed(true);
	_errorDialog->setClosable(true);
	_errorDialogText = new Wt::WText(_errorDialog->contents());
	Wt::WBreak *hr = new Wt::WBreak(_errorDialog->contents());
	hr->setHtmlTagName("hr");
	auto errorOkBtn = new Wt::WPushButton(Wt::WString::tr("Ok"), _errorDialog->contents());
	errorOkBtn->clicked().connect(_errorDialog, &Wt::WDialog::accept);

	//Main Widgets
	_mainStack = new Wt::WStackedWidget(root());
	lazyLoadLoginWidget();

	setInternalPathDefaultValid(true);
	handleInternalPathChanged(internalPath());
	internalPathChanged().connect(this, &WApplication::handleInternalPathChanged);
	authLogin().changed().connect(this, &WApplication::handleAuthChanged);

	//Enable server push
	enableUpdates();
}

WApplication::~WApplication()
{
	delete _matchFinder; //Must delete it from derived class so that the widget can access the application's session id
}

void WApplication::handleAuthChanged()
{
	handleInternalPathChanged(internalPath());
}

void WApplication::handleInternalPathChanged(std::string path)
{
	Wt::Utils::append(path, '/');
	
	if(!authLogin().loggedIn())
	{
		_mainStack->setCurrentWidget(_authWidget);
	}
	else
	{
		lazyLoadMatchFinder();
		_mainStack->setCurrentWidget(_matchFinder);
	}
}

void WApplication::lazyLoadLoginWidget()
{
	if(!_authWidget)
		_authWidget = new AuthWidget(_mainStack);
}

void WApplication::lazyLoadMatchFinder()
{
	if(!_matchFinder)
		_matchFinder = new MatchFinder(_mainStack);
}

void WApplication::showErrorDialog(const Wt::WString &message)
{
	_errorDialogText->setText(message);
	_errorDialog->show();
}

void WApplication::showDbBackendError(const std::string &)
{
	showErrorDialog(Wt::WString::tr("DbInternalError"));
}

}
