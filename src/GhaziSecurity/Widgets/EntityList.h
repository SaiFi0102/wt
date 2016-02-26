#ifndef GS_ENTITYLIST_WIDGET_H
#define GS_ENTITYLIST_WIDGET_H

#include "Dbo/Dbos.h"
#include "Application/WApplication.h"

#include <Wt/WTemplate>
#include <Wt/WLink>
#include <Wt/WItemDelegate>

#include <Wt/Dbo/QueryModel>
#include <Wt/WBatchEditProxyModel>

namespace GS
{

	//PROXY MODEL
	template<int IdColumn>
	class EntityListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		EntityListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &index, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	//FILTER MODELS
	class AbstractFilterWidgetModel : public Wt::WObject
	{
	public:
		AbstractFilterWidgetModel(const Wt::WString &filterTitle, const std::string &columnName, Wt::WObject *parent = nullptr)
			: Wt::WObject(parent), _columnName(columnName), _filterTitle(filterTitle)
		{ }
		//virtual void updateView() = 0;
		virtual void updateModel();
		virtual std::string value() = 0;
		virtual std::string sqlCondition() = 0;
		virtual Wt::WWidget *createWidget();
		virtual std::vector<std::string> boundValues() { return std::vector<std::string>(1, value()); }
		Wt::WString filterTitle() const { return _filterTitle; };
		Wt::WCheckBox *checkbox() const { return _cb; }
		bool enabled() const { return _enabled; }

	protected:
		Wt::WString _filterTitle;
		std::string _columnName;
		Wt::WCheckBox *_cb = nullptr;
		bool _enabled = false;
	};

	class WLineEditFilterModel : public AbstractFilterWidgetModel
	{
	public:
		typedef std::function<void(Wt::WLineEdit*)> FuncType;
		WLineEditFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: AbstractFilterWidgetModel(filterTitle, columnName, parent), _function(f)
		{ }
		//virtual void updateView() override { _edit->setValueText(_value); }
		virtual void updateModel() override;
		virtual std::string value() override { return _value.toUTF8(); }
		virtual std::string sqlCondition() override { return _columnName + " = ?"; }
		Wt::WLineEdit *edit() const { return _edit; }
		virtual Wt::WWidget *createWidget() override;

	protected:
		Wt::WLineEdit *_edit = nullptr;
		Wt::WString _value;
		FuncType _function;
	};

	class WComboBoxFilterModel : public AbstractFilterWidgetModel
	{
	public:
		typedef std::function<void(Wt::WComboBox*)> FuncType;
		WComboBoxFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: AbstractFilterWidgetModel(filterTitle, columnName, parent), _function(f)
		{ }
		//virtual void updateView() override { _edit->setCurrentIndex(_value); }
		virtual void updateModel() override;
		virtual std::string value() override { return boost::lexical_cast<std::string>(_value); }
		virtual std::string sqlCondition() override { return _columnName + " = ?"; }
		Wt::WComboBox *edit() const { return _edit; }
		virtual Wt::WWidget *createWidget() override;

	protected:
		Wt::WComboBox *_edit = nullptr;
		int _value;
		FuncType _function;
	};

	class BitmaskFilterModel : public WComboBoxFilterModel
	{
	public:
		BitmaskFilterModel(const Wt::WString &filterTitle, const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: WComboBoxFilterModel(filterTitle, columnName, f, parent)
		{ }
		virtual std::string sqlCondition() override { return _columnName + " & ?"; }
		virtual void updateModel() override;
	};

	class NameFilterModel : public WLineEditFilterModel
	{
	public:
		NameFilterModel(const std::string &columnName, const FuncType &f = FuncType(), Wt::WObject *parent = nullptr)
			: WLineEditFilterModel(Wt::WString::tr("GS.Name"), columnName, f, parent)
		{ }
		virtual std::string sqlCondition() override;
		virtual std::vector<std::string> boundValues() override;
		virtual Wt::WWidget *createWidget() override;
	};

	//ITEM DELEGATES
	class EntityTypeDelegate : public Wt::WItemDelegate
	{
	public:
		EntityTypeDelegate(Wt::WObject *parent = nullptr) : Wt::WItemDelegate(parent) { }
		virtual Wt::WWidget *update(Wt::WWidget *widget, const Wt::WModelIndex &index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
		{

		}
	};

	//ENTITY LISTS
	class AbstractEntityList : public Wt::WTemplate
	{
	public:
		typedef std::vector<AbstractFilterWidgetModel*> FilterModelVector;

		AbstractEntityList(Wt::WContainerWidget *parent = nullptr);
		virtual void reload() = 0;

		static Wt::WString representRole(int typeMask);
		static void initIdEdit(Wt::WLineEdit *edit);
		static void initTypeEdit(Wt::WComboBox *edit);
		static void initRoleEdit(Wt::WComboBox *edit);

	protected:
		void init();
		virtual void initModel() = 0;
		void addFilterModel(AbstractFilterWidgetModel *model);
		void handleAddFilter();
		void handleApplyFilters();
		virtual void applyFilter(const std::string sqlCondition) = 0;
		void addFilter(int filtersComboIndex);

		Wt::WContainerWidget *_filterWidgetsContainer = nullptr;
		Wt::WTableView *_tableView = nullptr;
		Wt::WAbstractItemModel *_model = nullptr;
		Wt::WAbstractItemModel *_proxyModel = nullptr;
		Wt::WComboBox *_filtersComboBox = nullptr;
		FilterModelVector _modelVector;
	};

	template<typename T>
	class QueryModelEntityList : public AbstractEntityList
	{
	public:
		typedef typename T ResultType;
		typedef Wt::Dbo::QueryModel<ResultType> QueryModelType;

		QueryModelEntityList(Wt::WContainerWidget *parent = nullptr) : AbstractEntityList(parent) { }
		QueryModelType *queryModel() const { return dynamic_cast<QueryModelType*>(_model); }
		virtual void reload() override;

	protected:
		virtual void applyFilter(const std::string sqlCondition) override;

		Wt::Dbo::Query<ResultType> _baseQuery;
	};

	class AllEntityList : public QueryModelEntityList<boost::tuple<long long, std::string, Entity::Type, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, EntityType, SpecificTypeMask };
		AllEntityList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class PersonList : public QueryModelEntityList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class EmployeeList : public QueryModelEntityList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		EmployeeList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class PersonnelList : public QueryModelEntityList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonnelList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class BusinessList : public QueryModelEntityList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		BusinessList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class ClientList : public QueryModelEntityList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		ClientList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	//TEMPLATE CLASS DEFINITIONS
	template<typename T>
	void QueryModelEntityList<T>::reload()
	{
		try
		{
			queryModel()->reload();
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "QueryModelEntityList::reload(): Dbo error(" << e.code() << "): " << e.what();
			APP->showDbBackendError(e.code());
		}
	}

	template<typename T>
	void QueryModelEntityList<T>::applyFilter(const std::string sqlCondition)
	{
		WApplication *app = APP;
		auto model = queryModel();
		Wt::Dbo::Query<ResultType> query(_baseQuery);

		if(!sqlCondition.empty())
		{
			query.where(sqlCondition);
			for(auto model : _modelVector)
			{
				if(!model->enabled())
					continue;

				auto values = model->boundValues();
				for(const std::string &v : values)
					query.bind(v);
			}
		}

		try
		{
			model->setQuery(query, true);
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "QueryModelEntityList::applyFilter(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	//TEMPLATE CLASS DEFINITIONS
	template<int IdColumn>
	EntityListProxyModel<IdColumn>::EntityListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
		//layoutChanged().connect(this, &EntityListProxyModel::addAdditionalColumns);
	}

	template<int IdColumn>
	boost::any EntityListProxyModel<IdColumn>::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");

			if(role == Wt::LinkRole)
			{
				boost::any idData = data(index(idx.row(), IdColumn));
				return Wt::WLink(Wt::WLink::InternalPath, "/admin/entities/entity-" + Wt::asString(idData).toUTF8());
			}
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

	template<int IdColumn>
	Wt::WFlags<Wt::ItemFlag> GS::EntityListProxyModel<IdColumn>::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	template<int IdColumn>
	boost::any EntityListProxyModel<IdColumn>::headerData(int section, Wt::Orientation orientation /* = Wt::Horizontal */, int role /* = Wt::DisplayRole */) const
	{
		if(section == _linkColumn)
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	template<int IdColumn>
	void EntityListProxyModel<IdColumn>::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

}

#endif