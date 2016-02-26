#include "Application/WApplication.h"
#include "Application/WServer.h"
#include "Dbo/Configuration.h"

#include "Widgets/AdminPages.h"
#include "Widgets/EntityView.h"
#include "Widgets/EntityList.h"
#include "Widgets/LocationMVC.h"
#include "Widgets/EntryCycleMVC.h"

#include <Wt/WNavigationBar>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WBootstrapTheme>
#include <Wt/WPushButton>

#include <Wt/Dbo/QueryModel>

namespace GS
{

Session::Session()
	: _userDatabase(*this)
{
	setConnectionPool(*WServer::instance()->sqlPool());

	MAP_DBO_TREE((*this))
	MAP_CONFIGURATION_DBO_TREE((*this))
}

Wt::Dbo::ptr<User> Session::user()
{
	if(_login.loggedIn())
		return user(_login.user());
	else
		return Wt::Dbo::ptr<GS::User>();
}

Wt::Dbo::ptr<User> Session::user(const Wt::Auth::User &authUser)
{
	Wt::Dbo::ptr<GS::AuthInfo> authInfo = _userDatabase.find(authUser);
	Wt::Dbo::ptr<GS::User> user = authInfo->user();

	if(!user)
	{
		user = add(new GS::User());
		authInfo.modify()->setUser(user);
	}
	return user;
}

WApplication::WApplication(const Wt::WEnvironment& env)
	: Wt::WApplication(env),
	_startTime(boost::posix_time::microsec_clock::local_time()), _entitiesDatabase(_session), _accountsDatabase(_session)
{
	//Enable server push
	enableUpdates();

	setTitle(Wt::WString::tr("GS.Title"));

	messageResourceBundle().use(appRoot() + "templates", false); //CHECK_BEFORE_RELEASE
	messageResourceBundle().use(appRoot() + "strings", false); //CHECK_BEFORE_RELEASE

	Wt::WLocale newLocale("en");
	newLocale.setDateFormat(Wt::WString::tr("GS.DateFormat"));
	newLocale.setTimeFormat(Wt::WString::tr("GS.TimeFormat"));
	newLocale.setDateTimeFormat(Wt::WString::tr("GS.DateTimeFormat"));
	newLocale.setDecimalPoint(Wt::WString::tr("GS.DecimalPoint").toUTF8());
	newLocale.setGroupSeparator(Wt::WString::tr("GS.GroupSeparator").toUTF8());
	setLocale(newLocale);

	useStyleSheet(Wt::WLink("style.css"));
	useStyleSheet(Wt::WLink(resourcesUrl() + "font-awesome/css/font-awesome.min.css"));

	Wt::WBootstrapTheme *theme = new Wt::WBootstrapTheme();
	theme->setVersion(Wt::WBootstrapTheme::Version3);
	setTheme(theme);

	setInternalPathDefaultValid(true);
	internalPathChanged().connect(this, &WApplication::handleInternalPathChanged);
	session().login().changed().connect(this, &WApplication::handleAuthChanged);

	//Error Dialog
	_errorDialog = new Wt::WDialog(Wt::WString::tr("GS.AnErrorOccurred"), this);
	_errorDialogText = new Wt::WText(_errorDialog->contents());
	Wt::WBreak *hr = new Wt::WBreak(_errorDialog->contents());
	hr->setHtmlTagName("hr");
	auto errorOkBtn = new Wt::WPushButton(Wt::WString::tr("GS.Ok"), _errorDialog->contents());
	errorOkBtn->clicked().connect(_errorDialog, &Wt::WDialog::accept);

	//Main Widgets
	_mainTemplate = new Wt::WTemplate(Wt::WString::tr("GS.Main"), root());
	_mainStack = new Wt::WStackedWidget();
	_navBar = new Wt::WNavigationBar();
	_navBar->setTemplateText(Wt::WString::tr("GS.NavigationBar"));
	_navBar->bindString("container-class", "container");
	_navBar->setResponsive(true);
	_navBar->addMenu(_mainMenu = new Wt::WMenu(_mainStack));
	_mainMenu->setInternalPathEnabled("/");

	//Logo
	Wt::WImage *logoImage = new Wt::WImage(Wt::WLink("images/logo.png"), Wt::WString::tr("GS.Logo.Alt"));
	_navBar->setTitle(logoImage, Wt::WLink(Wt::WLink::InternalPath, "/"));

	_mainTemplate->bindWidget("navigation", _navBar);
	_mainTemplate->bindWidget("content", _mainStack);

	//Page widgets
	auto homeMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Home"), new Wt::WText(Wt::WString::tr("GS.Home")));
	homeMenuItem->setPathComponent("");
	_mainMenu->addItem(homeMenuItem);
	auto aboutMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.About"), new Wt::WText(Wt::WString::tr("GS.About")));
	aboutMenuItem->setPathComponent("about");
	_mainMenu->addItem(aboutMenuItem);
	auto contactMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Contact"), new Wt::WText(Wt::WString::tr("GS.Contact")));
	contactMenuItem->setPathComponent("contact");
	_mainMenu->addItem(contactMenuItem);

	handleInternalPathChanged(internalPath());
	//WServer *server = WServer::instance();
}

WApplication::~WApplication()
{

}

void WApplication::handleAuthChanged()
{

}

void WApplication::handleInternalPathChanged(std::string path)
{
	std::string firstComponent = internalPathNextPart("/");
	if(firstComponent == "admin")
	{
		lazyLoadAdminWidgets();
		_navBar->bindString("container-class", "container-fluid");
		_navBar->addStyleClass("navbar-admin");
		_mainMenu->hide();
		_adminMenu->show();
		_mainStack->setCurrentWidget(_adminStack);

		//entity view
		std::string pathPrefix = "/" + firstComponent + "/entities/entity-";
		if(internalPath().substr(0, pathPrefix.size()) == pathPrefix)
		{
			std::string entityIdStr = internalPath().substr(pathPrefix.size());
			if(!_entitiesAdminPage->checkPathComponentExist("entity-" + entityIdStr))
			{
				try
				{
					long long entityId = boost::lexical_cast<long long>(entityIdStr);

					Wt::Dbo::Transaction t(session());
					Wt::Dbo::ptr<Entity> entityPtr = session().find<Entity>().where("id = ?").bind(entityId);
					if(entityPtr)
					{
						auto menuItem = _entitiesAdminPage->createMenuItem(
							Wt::WString::fromUTF8(entityPtr->name),
							"entity-" + boost::lexical_cast<std::string>(entityId),
							new EntityView(entityPtr));
					}
					t.commit();
				}
				catch(boost::bad_lexical_cast &) {}
				catch(Wt::Dbo::Exception &e)
				{
					Wt::log("error") << "WApplication::handleInternalPathChanged(): Dbo error(" << e.code() << "): " << e.what();
					showDbBackendError(e.code());
				}
			}
		}
	}
	else
	{
		_navBar->bindString("container-class", "container");
		_navBar->removeStyleClass("navbar-admin");
		_mainMenu->show();
		if(_adminMenu) _adminMenu->hide();
	}
}

void WApplication::lazyLoadAdminWidgets()
{
	if(_adminMenu)
		return;

	_adminStack = new Wt::WStackedWidget(_mainStack);
	_navBar->addMenu(_adminMenu = new Wt::WMenu(_adminStack));
	_adminMenu->setInternalPathEnabled("/admin/");

	//Admin page widgets
	//Dashboard
	AdminPageWidget *dashboardTemplate = new AdminPageWidget();
	dashboardTemplate->menu()->setInternalPathEnabled("/admin/");

	auto overviewMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Overview"), new Wt::WText(Wt::WString::tr("GS.Admin.OverviewPage")));
	overviewMenuItem->setPathComponent("");
	dashboardTemplate->menu()->addItem(overviewMenuItem);

	auto dashboardMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Dashboard"), dashboardTemplate);
	dashboardMenuItem->setPathComponent("");
	_adminMenu->addItem(dashboardMenuItem);

	//Entites
	auto entitiesMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Entities"), _entitiesAdminPage = new EntitiesAdminPage());
	entitiesMenuItem->setPathComponent("entities");
	_adminMenu->addItem(entitiesMenuItem);
}

void WApplication::initFindPersonModel()
{
	if(_findPersonModel)
		return;

	_findPersonModel = new FindPersonModel(this);
	_findPersonModel->setQuery(session().find<Entity>().where("type = ?").bind(Entity::PersonType));
	_findPersonModel->addColumn("name");
	_findPersonModel->addColumn("id");
}

void WApplication::initFindBusinessModel()
{
	if(_findBusinessModel)
		return;

	_findBusinessModel = new FindBusinessModel(this);
	_findBusinessModel->setQuery(session().find<Entity>().where("type = ?").bind(Entity::BusinessType));
	_findBusinessModel->addColumn("name");
	_findBusinessModel->addColumn("id");
}

// void WApplication::initLocationQueryModel()
// {
// 	if(_locationQueryModel)
// 		return;
// 
// 	_locationQueryModel = new LocationQueryModel(this);
// 	_locationQueryModel->setQuery(session().find<Location>());
// 	_locationQueryModel->addColumn("address");
// 	_locationQueryModel->addColumn("country_code");
// 	_locationQueryModel->addColumn("city_id");
// 	_locationQueryModel->addColumn("id");
// 
// 	_locationFilterModel = new LocationFilterModel(_locationQueryModel);
// 	_locationFilterModel->setSourceModel(_locationQueryModel);
// }

void WApplication::initCountryQueryModel()
{
	if(_countryQueryModel)
		return;

	_countryQueryModel = new CountryQueryModel(this);
	_countryQueryModel->setQuery(session().find<Country>().orderBy("CASE WHEN code = 'PK' THEN 1 ELSE 2 END ASC"));
	_countryQueryModel->addColumn("name");
	_countryQueryModel->addColumn("code");

	_countryProxyModel = new CountryProxyModel(_countryQueryModel, _countryQueryModel);
}

void WApplication::initCityQueryModel()
{
	if(_cityQueryModel)
		return;

	_cityQueryModel = new CityQueryModel(this);
	_cityQueryModel->setQuery(session().find<City>());
	_cityQueryModel->addColumn("name");
	_cityQueryModel->addColumn("id");
	_cityQueryModel->addColumn("country_code");
}

void WApplication::initPositionQueryModel()
{
	if(_positionQueryModel)
		return;

	_positionQueryModel = new PositionQueryModel(this);
	_positionQueryModel->setQuery(session().find<EmployeePosition>());
	_positionQueryModel->addColumn("title");
	_positionQueryModel->addColumn("id");

	_positionProxyModel = new PositionProxyModel(_positionQueryModel, _positionQueryModel);
}

void WApplication::initServiceQueryModel()
{
	if(_serviceQueryModel)
		return;

	_serviceQueryModel = new ServiceQueryModel(this);
	_serviceQueryModel->setQuery(session().find<ClientService>());
	_serviceQueryModel->addColumn("title");
	_serviceQueryModel->addColumn("id");

	_serviceProxyModel = new ServiceProxyModel(_serviceQueryModel, _serviceQueryModel);
}

void WApplication::showErrorDialog(const Wt::WString &message)
{
	_errorDialogText->setText(message);
	_errorDialog->show();
}

void WApplication::showDbBackendError(const std::string &code)
{
	showErrorDialog(Wt::WString::tr("GS.DbInternalError"));
}

}
