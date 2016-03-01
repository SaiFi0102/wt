#ifndef GS_ADMINPAGES_WIDGET_H
#define GS_ADMINPAGES_WIDGET_H

#include "Widgets/EntityView.h"

#include <Wt/WTemplate>
#include <Wt/WTabWidget>

namespace GS
{

	class AdminPageWidget : public Wt::WTemplate
	{
	public:
		AdminPageWidget(const std::string basePathComponent, Wt::WContainerWidget *parent = nullptr);

		Wt::WNavigationBar *sideBar() const { return _sideBar; }
		Wt::WStackedWidget *stackWidget() const { return _stackWidget; }
		Wt::WMenu *menu() const { return _menu; }
		Wt::WMenuItem *createMenuItem(const Wt::WString &label, const std::string &pathComponent, Wt::WWidget *contents);
		Wt::WMenuItem *createMenuItem(int index, const Wt::WString &label, const std::string &pathComponent, Wt::WWidget *contents);
		bool checkPathComponentExist(const std::string &pathComponent) const;
		std::string basePathComponent() const { return _basePathComponent; }

	protected:
		Wt::WNavigationBar *_sideBar = nullptr;
		Wt::WStackedWidget *_stackWidget = nullptr;
		Wt::WMenu *_menu = nullptr;
		std::string _basePathComponent;
	};

	class EntitiesAdminPage : public AdminPageWidget
	{
	public:
		EntitiesAdminPage(Wt::WContainerWidget *parent = nullptr);

	protected:
		void newEntityViewSubmitted();

		EntityView *_newEntityView = nullptr;
		Wt::WMenuItem *_newEntityMenuItem = nullptr;
		Wt::Signals::connection _submittedConnection;
	};

	class AccountsAdminPage : public AdminPageWidget
	{
	public:
		AccountsAdminPage(Wt::WContainerWidget *parent = nullptr);
	};

}

#endif