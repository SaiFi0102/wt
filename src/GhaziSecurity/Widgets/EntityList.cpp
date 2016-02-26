#include "Widgets/EntityList.h"

#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>
#include <Wt/WTableView>
#include <Wt/WPushButton>
#include <Wt/WIntValidator>

namespace GS
{

	void AbstractFilterWidgetModel::updateModel()
	{
		if(_cb)
			_enabled = _cb->isChecked();
		else
			_enabled = false;
	}

	Wt::WWidget *AbstractFilterWidgetModel::createWidget()
	{
		return _cb = new Wt::WCheckBox();
	}

	void WLineEditFilterModel::updateModel()
	{
		AbstractFilterWidgetModel::updateModel();

		if(_edit)
			_value = _edit->valueText();
		else
			_value = "";

		if(_value.empty())
			_enabled = false;
	}

	void WComboBoxFilterModel::updateModel()
	{
		AbstractFilterWidgetModel::updateModel();

		if(_edit)
			_value = _edit->currentIndex();
		else
			_value = -1;

		if(_value == -1)
			_enabled = false;
	}

	void BitmaskFilterModel::updateModel()
	{
		AbstractFilterWidgetModel::updateModel();

		if(_edit)
			_value = boost::any_cast<int>(_edit->model()->index(_edit->currentIndex(), 0).data(Wt::UserRole));
		else
			_value = 0;

		if(_value == 0)
			_enabled = false;
	}

	Wt::WWidget *WLineEditFilterModel::createWidget()
	{
		if(_edit)
			return nullptr;

		_edit = new Wt::WLineEdit();
		if(_function) _function(_edit);

		AbstractFilterWidgetModel::createWidget();
		return _edit;
	}

	Wt::WWidget *WComboBoxFilterModel::createWidget()
	{
		if(_edit)
			return nullptr;

		_edit = new Wt::WComboBox();
		if(_function) _function(_edit);

		AbstractFilterWidgetModel::createWidget();
		return _edit;
	}

	Wt::WWidget *NameFilterModel::createWidget()
	{
		if(!WLineEditFilterModel::createWidget())
			return nullptr;

		_edit->setMaxLength(70);
		return _edit;
	}

	std::string NameFilterModel::sqlCondition()
	{
		std::string result;
		std::string v = value();
		boost::tokenizer<> tokens(v);
		for(const auto &namePart : tokens)
			result += _columnName + " LIKE ? OR ";

		result = result.substr(0, result.size() - 4);

		return result;
	}

	std::vector<std::string> NameFilterModel::boundValues()
	{
		std::string v = value();
		boost::tokenizer<> tokens(v);
		auto vec = std::vector<std::string>(tokens.begin(), tokens.end());
		for(auto &s : vec)
			s = "%" + s + "%";
		return vec;
	}

	AbstractEntityList::AbstractEntityList(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplate(tr("GS.Admin.Entities.List"), parent)
	{
		_tableView = new Wt::WTableView();
		_tableView->setSelectable(true);
		_tableView->setHeaderHeight(40);
		_tableView->setRowHeight(30);
		_tableView->setMaximumSize(Wt::WLength::Auto, 600);

		_filtersComboBox = new Wt::WComboBox();
		_filtersComboBox->addItem(tr("GS.SelectFilter"));

		auto addFilter = new Wt::WPushButton(tr("GS.Add"));
		addFilter->clicked().connect(this, &AbstractEntityList::handleAddFilter);

		auto applyFilters = new Wt::WPushButton(tr("GS.ApplyFilters"));
		applyFilters->clicked().connect(this, &AbstractEntityList::handleApplyFilters);

		_filterWidgetsContainer = new Wt::WContainerWidget();

		bindWidget("table-view", _tableView);
		bindWidget("filters-combo", _filtersComboBox);
		bindWidget("add-filter", addFilter);
		bindWidget("apply-filter", applyFilters);
		bindWidget("filters-container", _filterWidgetsContainer);
	}

	void AbstractEntityList::init()
	{
		if(_model)
			return;

		initModel();
		_tableView->setModel(_proxyModel);

		int diff = _proxyModel->columnCount() - _model->columnCount();
		if(diff > 0)
		{
			_tableView->setColumnWidth(_proxyModel->columnCount() - 1, 40);
			_tableView->setColumnAlignment(_proxyModel->columnCount() - 1, Wt::AlignCenter);
		}

		while(diff > 0)
		{
			_tableView->setSortingEnabled(_model->columnCount() + diff - 1, false);
			--diff;
		}
	}

	void AbstractEntityList::addFilterModel(AbstractFilterWidgetModel *model)
	{
		if(!model)
			return;

		Wt::WObject::addChild(model);
		_filtersComboBox->addItem(model->filterTitle());
		_modelVector.push_back(model);
	}

	void AbstractEntityList::handleAddFilter()
	{
		addFilter(_filtersComboBox->currentIndex());
	}

	void AbstractEntityList::addFilter(int filtersComboIndex)
	{
		if(filtersComboIndex < 1 || filtersComboIndex > _modelVector.size())
			return;

		auto model = _modelVector[filtersComboIndex - 1];
		auto newWidget = model->createWidget();
		if(!newWidget)
			return;

		Wt::WTemplate *filterTemplate = new Wt::WTemplate(tr("GS.Admin.Entities.List.Filter"));
		model->checkbox()->setChecked(true);
		filterTemplate->bindWidget("checkbox", model->checkbox());
		filterTemplate->bindString("title", model->filterTitle());
		filterTemplate->bindWidget("widget", newWidget);
		_filterWidgetsContainer->addWidget(filterTemplate);
	}

	void AbstractEntityList::handleApplyFilters()
	{
		std::string sqlCondition;
		for(auto model : _modelVector)
		{
			model->updateModel();
			if(!model->enabled())
				continue;

			std::string thisCondition = model->sqlCondition();
			if(thisCondition.empty())
				continue;

			thisCondition = "(" + thisCondition + ")";
			sqlCondition += thisCondition + " AND ";
		}
		sqlCondition = sqlCondition.substr(0, sqlCondition.size() - 5);
		applyFilter(sqlCondition);
	}

#define IdColumnWidth 80
#define NameColumnWidth 250
#define TypeColumnWidth 80
#define RolesColumnWidth 250

	AllEntityList::AllEntityList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelEntityList<ResultType>(parent)
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

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractEntityList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new WComboBoxFilterModel(tr("GS.Type"), "type", std::bind(&AbstractEntityList::initTypeEdit, std::placeholders::_1)));
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractEntityList::initRoleEdit, std::placeholders::_1)));
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

// 	boost::any AllEntityList::Model::data(const Wt::WModelIndex& index, int role /*= Wt::DisplayRole*/) const
// 	{
// 		if(index.column() == EntityType && role == Wt::DisplayRole)
// 			return AbstractEntityList::representEntityType(resultRow(index.row()).get<EntityType>());
// 		if(index.column() == SpecificTypeMask && role == Wt::DisplayRole)
// 			return AbstractEntityList::representRole(resultRow(index.row()).get<SpecificTypeMask>());
// 		if(index.column() == Id && role == Wt::LinkRole)
// 			return AbstractEntityList::representLink(resultRow(index.row()).get<Id>());
// 
// 		return QueryModelType::data(index, role);
// 	}

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

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractEntityList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractEntityList::initRoleEdit, std::placeholders::_1)));
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

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractEntityList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractEntityList::initRoleEdit, std::placeholders::_1)));
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

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractEntityList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractEntityList::initRoleEdit, std::placeholders::_1)));
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

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractEntityList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractEntityList::initRoleEdit, std::placeholders::_1)));
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

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractEntityList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new BitmaskFilterModel(tr("GS.Role"), "specificTypeMask", std::bind(&AbstractEntityList::initRoleEdit, std::placeholders::_1)));
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
	void AbstractEntityList::initRoleEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(0, tr("GS.Employee"));
		edit->model()->setData(edit->model()->index(0, 0), Entity::EmployeeType, Wt::UserRole);

		edit->insertItem(1, tr("GS.Personnel"));
		edit->model()->setData(edit->model()->index(1, 0), Entity::PersonnelType, Wt::UserRole);

		edit->insertItem(2, tr("GS.Client"));
		edit->model()->setData(edit->model()->index(2, 0), Entity::ClientType, Wt::UserRole);
	}

	void AbstractEntityList::initTypeEdit(Wt::WComboBox *edit)
	{
		edit->insertItem(Entity::PersonType, tr("GS.Person"));
		edit->insertItem(Entity::BusinessType, tr("GS.Business"));
	}

	void AbstractEntityList::initIdEdit(Wt::WLineEdit *edit)
	{
		edit->setValidator(new Wt::WIntValidator());
		edit->setMaxLength(20);
	}

	Wt::WString AbstractEntityList::representRole(int typeMask)
	{
		std::string result;
		if(typeMask & Entity::EmployeeType)
			result += tr("GS.Employee").toUTF8() + ", ";
		if(typeMask & Entity::PersonnelType)
			result += tr("GS.Personnel").toUTF8() + ", ";
		if(typeMask & Entity::ClientType)
			result += tr("GS.Client").toUTF8() + ", ";

		result = result.substr(0, result.size() - 2);
		return Wt::WString::fromUTF8(result);
	}

}
