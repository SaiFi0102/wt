#include "Widgets/AdminPages.h"
#include "Widgets/EntityList.h"

#include <Wt/WNavigationBar>
#include <Wt/WMenu>
#include <Wt/WStackedWidget>

namespace GS
{

	AdminPageWidget::AdminPageWidget(Wt::WContainerWidget *parent)
		: Wt::WTemplate(tr("GS.Admin.Main"), parent)
	{
		_sideBar = new Wt::WNavigationBar();
		_stackWidget = new Wt::WStackedWidget();
		_menu = new Wt::WMenu(_stackWidget);
		_sideBar->setResponsive(true);
		_menu->addStyleClass("nav-admin-side");

		_sideBar->addMenu(_menu, Wt::AlignLeft, true);
		bindWidget("sidebar", _sideBar);
		bindWidget("content", _stackWidget);
	}

	Wt::WMenuItem * AdminPageWidget::createMenuItem(int index, const Wt::WString &label, const std::string &pathComponent, Wt::WWidget *contents)
	{
		auto menuItem = new Wt::WMenuItem(label, contents);
		menuItem->setPathComponent(pathComponent);
		menu()->insertItem(index, menuItem);
		return menuItem;
	}

	Wt::WMenuItem *AdminPageWidget::createMenuItem(const Wt::WString &label, const std::string &pathComponent, Wt::WWidget *contents)
	{
		return createMenuItem(menu()->count(), label, pathComponent, contents);
	}

	bool AdminPageWidget::checkPathComponentExist(const std::string &pathComponent) const
	{
		for(int i = 0; i < menu()->count(); ++i)
		{
			if(menu()->itemAt(i)->pathComponent() == pathComponent)
				return true;
		}
		return false;
	}

	EntitiesAdminPage::EntitiesAdminPage(Wt::WContainerWidget *parent /*= nullptr*/)
	{
		menu()->setInternalPathEnabled("/admin/entities/");

		auto allEntitiesMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.AllEntities"));
		allEntitiesMenuItem->setPathComponent("");
		AllEntityList *allEntityList = new AllEntityList();
		allEntitiesMenuItem->setContents(allEntityList);
		allEntitiesMenuItem->triggered().connect(allEntityList, &AllEntityList::reload);
		menu()->addItem(allEntitiesMenuItem);

		auto personsMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Persons"));
		personsMenuItem->setPathComponent("persons");
		PersonList *personList = new PersonList();
		personsMenuItem->setContents(personList);
		personsMenuItem->triggered().connect(personList, &PersonList::reload);
		menu()->addItem(personsMenuItem);

		auto employeesMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Employees"));
		employeesMenuItem->setPathComponent("employees");
		EmployeeList *employeeList = new EmployeeList();
		employeesMenuItem->setContents(employeeList);
		employeesMenuItem->triggered().connect(employeeList, &EmployeeList::reload);
		menu()->addItem(employeesMenuItem);

		auto personnelMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Personnel"));
		personnelMenuItem->setPathComponent("personnel");
		PersonnelList *personnelList = new PersonnelList();
		personnelMenuItem->setContents(personnelList);
		personnelMenuItem->triggered().connect(personnelList, &PersonnelList::reload);
		menu()->addItem(personnelMenuItem);

		auto businessesMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Businesses"));
		businessesMenuItem->setPathComponent("businesses");
		BusinessList *businessList = new BusinessList();
		businessesMenuItem->setContents(businessList);
		businessesMenuItem->triggered().connect(businessList, &BusinessList::reload);
		menu()->addItem(businessesMenuItem);

		auto clientsMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.Clients"));
		clientsMenuItem->setPathComponent("clients");
		ClientList *clientList = new ClientList();
		clientsMenuItem->setContents(clientList);
		clientsMenuItem->triggered().connect(clientList, &ClientList::reload);
		menu()->addItem(clientsMenuItem);

		menu()->addSeparator();

		_newEntityMenuItem = new Wt::WMenuItem(Wt::WString::tr("GS.AddNewEntity"));
		_newEntityMenuItem->setPathComponent("new");
		_newEntityMenuItem->setContents(_newEntityView = new EntityView(Entity::InvalidType));
		menu()->addItem(_newEntityMenuItem);
		_submittedConnection = _newEntityView->submitted().connect(this, &EntitiesAdminPage::newEntityViewSubmitted);

		menu()->addSeparator();
	}

	void EntitiesAdminPage::newEntityViewSubmitted()
	{
		if(!_newEntityView || _newEntityView->entityPtr().id() == -1)
			return;

		auto submittedEntityView = _newEntityView;
		auto submittedEntityItem = _newEntityMenuItem;
		int newEntityMenuItemIndex = menu()->indexOf(submittedEntityItem);
		std::string newPathComponent = "entity-" + boost::lexical_cast<std::string>(submittedEntityView->entityPtr().id());

		menu()->removeItem(submittedEntityItem);
		submittedEntityItem->setText(tr("GS.AddNewEntity"));
		submittedEntityItem->setPathComponent(newPathComponent);

		_newEntityView = new EntityView(Entity::InvalidType);
		_submittedConnection.disconnect();
		_submittedConnection = _newEntityView->submitted().connect(this, &EntitiesAdminPage::newEntityViewSubmitted);
		auto newMenuItem = createMenuItem(newEntityMenuItemIndex, _newEntityView->name(), "new", _newEntityView);

		menu()->addItem(submittedEntityItem);
		submittedEntityItem->select();
	}

}