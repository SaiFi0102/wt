#include "Widgets/EntityList.h"
#include "Utilities/FilteredList.h"

#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>
#include <Wt/WTableView>
#include <Wt/WPushButton>
#include <Wt/WIntValidator>

#define IdColumnWidth 80
#define NameColumnWidth 250
#define TypeColumnWidth 80
#define RolesColumnWidth 250

namespace GS
{

	AllEntityList::AllEntityList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("GS.Id"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("GS.Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(EntityType, Wt::Horizontal, tr("GS.Type"));
		_tableView->setColumnWidth(EntityType, TypeColumnWidth);
		model->setHeaderData(SpecificTypeMask, Wt::Horizontal, tr("GS.Roles"));
		_tableView->setColumnWidth(SpecificTypeMask, RolesColumnWidth);

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractFilteredList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new WComboBoxFilterModel(tr("GS.Type"), "type", std::bind(&AbstractFilteredList::initEntityTypeEdit, std::placeholders::_1)));
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractFilteredList::initRoleEdit, std::placeholders::_1)));
	}

	void AllEntityList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, type, specificTypeMask FROM " + std::string(Entity::tableName()));
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);
		model->addAllFieldsAsColumns();

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	PersonList::PersonList(Wt::WContainerWidget *parent /*= nullptr*/)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("GS.Id"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("GS.Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(SpecificTypeMask, Wt::Horizontal, tr("GS.Roles"));
		_tableView->setColumnWidth(SpecificTypeMask, RolesColumnWidth);

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractFilteredList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractFilteredList::initRoleEdit, std::placeholders::_1)));
	}

	void PersonList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::PersonType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);
		model->addAllFieldsAsColumns();

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	BusinessList::BusinessList(Wt::WContainerWidget *parent /*= nullptr*/)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("GS.Id"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("GS.Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(SpecificTypeMask, Wt::Horizontal, tr("GS.Roles"));
		_tableView->setColumnWidth(SpecificTypeMask, RolesColumnWidth);

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractFilteredList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractFilteredList::initRoleEdit, std::placeholders::_1)));
	}

	void BusinessList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::BusinessType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);
		model->addAllFieldsAsColumns();

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	EmployeeList::EmployeeList(Wt::WContainerWidget *parent /*= nullptr*/)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("GS.Id"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("GS.Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(SpecificTypeMask, Wt::Horizontal, tr("GS.Roles"));
		_tableView->setColumnWidth(SpecificTypeMask, RolesColumnWidth);

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractFilteredList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractFilteredList::initRoleEdit, std::placeholders::_1)));
	}

	void EmployeeList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName()))
			.where("type = ? AND specificTypeMask & ?").bind(Entity::PersonType).bind(Entity::EmployeeType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);
		model->addAllFieldsAsColumns();

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	PersonnelList::PersonnelList(Wt::WContainerWidget *parent /*= nullptr*/)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("GS.Id"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("GS.Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(SpecificTypeMask, Wt::Horizontal, tr("GS.Roles"));
		_tableView->setColumnWidth(SpecificTypeMask, RolesColumnWidth);

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractFilteredList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractFilteredList::initRoleEdit, std::placeholders::_1)));
	}

	void PersonnelList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName()))
			.where("type = ? AND (specificTypeMask & ?) = ?")
			.bind(Entity::PersonType).bind(Entity::EmployeeType | Entity::PersonnelType).bind(Entity::EmployeeType | Entity::PersonnelType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);
		model->addAllFieldsAsColumns();

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	ClientList::ClientList(Wt::WContainerWidget *parent /*= nullptr*/)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("GS.Id"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("GS.Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(SpecificTypeMask, Wt::Horizontal, tr("GS.Roles"));
		_tableView->setColumnWidth(SpecificTypeMask, RolesColumnWidth);

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractFilteredList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractFilteredList::initRoleEdit, std::placeholders::_1)));
	}

	void ClientList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>("SELECT id, name, specificTypeMask FROM " + std::string(Entity::tableName())).where("type = ?").bind(Entity::ClientType);
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);
		model->addAllFieldsAsColumns();

		_proxyModel = new EntityListProxyModel<Id>(_model, _model);
	}

	//CRAAAAP
	void AbstractFilteredList::initRoleEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(0, tr("GS.Employee"));
		edit->model()->setData(edit->model()->index(0, 0), Entity::EmployeeType, Wt::UserRole);

		edit->insertItem(1, tr("GS.Personnel"));
		edit->model()->setData(edit->model()->index(1, 0), Entity::PersonnelType, Wt::UserRole);

		edit->insertItem(2, tr("GS.Client"));
		edit->model()->setData(edit->model()->index(2, 0), Entity::ClientType, Wt::UserRole);
	}

	void AbstractFilteredList::initEntityTypeEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(Entity::PersonType, tr("GS.Person"));
		edit->insertItem(Entity::BusinessType, tr("GS.Business"));
	}

}
