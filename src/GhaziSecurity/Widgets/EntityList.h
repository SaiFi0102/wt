#ifndef GS_ENTITYLIST_WIDGET_H
#define GS_ENTITYLIST_WIDGET_H

#include "Dbo/Dbos.h"
#include "Application/WApplication.h"
#include "Utilities/FilteredList.h"

#include <Wt/WTemplate>
#include <Wt/WLink>

#include <Wt/Dbo/QueryModel>
#include <Wt/WBatchEditProxyModel>

namespace GS
{
	class AbstractFilterWidgetModel;

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

	//ENTITY LISTS
	class AllEntityList : public QueryModelFilteredList<boost::tuple<long long, std::string, Entity::Type, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, EntityType, SpecificTypeMask };
		AllEntityList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class PersonList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class EmployeeList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		EmployeeList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class PersonnelList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		PersonnelList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class BusinessList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		BusinessList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	class ClientList : public QueryModelFilteredList<boost::tuple<long long, std::string, Wt::WFlags<Entity::SpecificType>>>
	{
	public:
		enum ResultColumns { Id, Name, SpecificTypeMask };
		ClientList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	//TEMPLATE CLASS DEFINITIONS
	template<int IdColumn>
	EntityListProxyModel<IdColumn>::EntityListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
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