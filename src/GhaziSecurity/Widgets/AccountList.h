#ifndef GS_ACCOUNTLIST_WIDGET_H
#define GS_ACCOUNTLIST_WIDGET_H

#include "Dbo/Dbos.h"
#include "Utilities/FilteredList.h"
#include <Wt/WBatchEditProxyModel>

namespace GS
{

	//PROXY MODEL
	class AccountListProxyModel : public Wt::WBatchEditProxyModel
	{
	public:
		AccountListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent = nullptr);
		virtual boost::any data(const Wt::WModelIndex &index, int role = Wt::DisplayRole) const override;
		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const override;
		virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const override;

	protected:
		void addAdditionalColumns();
		int _linkColumn = -1;
	};

	class AccountList : public QueryModelFilteredList<boost::tuple<long long, std::string, Account::Type, boost::optional<long long>, double, std::string>>
	{
	public:
		enum ResultColumns { Id, Name, Type, EntityId, Balance, EntityName };
		AccountList(Wt::WContainerWidget *parent = nullptr);

	protected:
		virtual void initModel() override;
	};

	//TEMPLATE CLASS DEFINITIONS

}

#endif