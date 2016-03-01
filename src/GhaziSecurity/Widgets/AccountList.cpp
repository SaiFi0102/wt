#include "Widgets/AccountList.h"
#include "Application/WApplication.h"

#include <Wt/WTableView>
#include <Wt/Dbo/QueryModel>

#define IdColumnWidth 80
#define NameColumnWidth 150
#define BalanceColumnWidth 150
#define EntityColumnWidth 200
#define TypeColumnWidth 250

namespace GS
{

	AccountList::AccountList(Wt::WContainerWidget *parent /*= nullptr*/)
		: QueryModelFilteredList<ResultType>(parent)
	{
		init();
		auto model = queryModel();
		model->setHeaderData(Id, Wt::Horizontal, tr("GS.Id"));
		_tableView->setColumnWidth(Id, IdColumnWidth);
		model->setHeaderData(Name, Wt::Horizontal, tr("GS.Name"));
		_tableView->setColumnWidth(Name, NameColumnWidth);
		model->setHeaderData(Type, Wt::Horizontal, tr("GS.Type"));
		_tableView->setColumnWidth(Type, TypeColumnWidth);
		model->setHeaderData(EntityId, Wt::Horizontal, tr("GS.Entity"));
		_tableView->setColumnWidth(EntityId, EntityColumnWidth);
		model->setHeaderData(Balance, Wt::Horizontal, tr("GS.Balance"));
		_tableView->setColumnWidth(Balance, BalanceColumnWidth);

		addFilterModel(new WLineEditFilterModel(tr("GS.Id"), "id", std::bind(&AbstractFilteredList::initIdEdit, std::placeholders::_1)));
		addFilterModel(new NameFilterModel("name")); addFilter(2);
		addFilterModel(new RangeFilterModel(tr("GS.Balance"), "balance"));
		//addFilterModel(new BitmaskFilterModel(tr("GS.Recurring?"), "entryCycle", std::bind(&AbstractFilteredList::initEntryCycleEdit, std::placeholders::_1)));
	}

	void AccountList::initModel()
	{
		QueryModelType *model;
		_model = model = new QueryModelType(this);

		_baseQuery = APP->session().query<ResultType>(
			"SELECT acc.id, acc.name, acc.type, entity.id entity_id, acc.balance, entity.name entity_name FROM " + std::string(Account::tableName()) + " acc "
			"LEFT JOIN " + Entity::tableName() + " entity ON (entity.bal_account_id = acc.id OR entity.bal_account_id = acc.id)");
		Wt::Dbo::Query<ResultType> query(_baseQuery); //must copy the query first
		model->setQuery(query);
		model->addColumn("acc.id");
		model->addColumn("acc.name");
		model->addColumn("acc.type");
		model->addColumn("entity.id entity_id"); //represented in data()
		model->addColumn("acc.balance");

		_proxyModel = new AccountListProxyModel(_model, _model);
	}

	AccountListProxyModel::AccountListProxyModel(Wt::WAbstractItemModel *model, Wt::WObject *parent /*= nullptr*/)
		: Wt::WBatchEditProxyModel(parent)
	{
		setSourceModel(model);
		addAdditionalColumns();
	}

	void AccountListProxyModel::addAdditionalColumns()
	{
		int lastColumn = columnCount();

		if(insertColumn(lastColumn))
			_linkColumn = lastColumn;
		else
			_linkColumn = -1;
	}

	Wt::WFlags<Wt::ItemFlag> AccountListProxyModel::flags(const Wt::WModelIndex &index) const
	{
		if(index.column() == _linkColumn)
			return Wt::ItemIsXHTMLText;
		return Wt::WBatchEditProxyModel::flags(index);
	}

	boost::any AccountListProxyModel::headerData(int section, Wt::Orientation orientation /*= Wt::Horizontal*/, int role /*= Wt::DisplayRole*/) const
	{
		if(section == _linkColumn)
			return Wt::WAbstractItemModel::headerData(section, orientation, role);
		return Wt::WBatchEditProxyModel::headerData(section, orientation, role);
	}

	boost::any AccountListProxyModel::data(const Wt::WModelIndex &idx, int role /*= Wt::DisplayRole*/) const
	{
		if(_linkColumn != -1 && idx.column() == _linkColumn)
		{
			if(role == Wt::DisplayRole)
				return Wt::WString::tr("GS.LinkIcon");
			else if(role == Wt::LinkRole)
			{
				boost::any idData = data(index(idx.row(), AccountList::Id));
				return Wt::WLink(Wt::WLink::InternalPath, "/admin/accounts/account-" + Wt::asString(idData).toUTF8());
			}
		}

		if(idx.column() == AccountList::Balance && role == Wt::DisplayRole)
		{
			auto queryModel = dynamic_cast<Wt::Dbo::QueryModel<AccountList::ResultType>*>(sourceModel());
			const AccountList::ResultType &res = queryModel->resultRow(idx.row());
			Account::Type type = boost::get<AccountList::Type>(res);
			double balance = boost::get<AccountList::Balance>(res);
			Wt::WString balanceStr = APP->locale().toFixedString(std::abs(balance), 2);

			switch(type)
			{
			default:
			case Account::Unspecified:
				if(balance > 0) return "Rs. " + balanceStr + " " + Wt::WString::tr("GS.Debit");
				else if(balance < 0) return "Rs. " + balanceStr + " " + Wt::WString::tr("GS.Credit");
				else return "Rs. 0";
			case Account::EntityBalanceAccount:
				if(balance > 0) return "Rs. " + balanceStr + " " + Wt::WString::tr("GS.Receivable");
				else if(balance < 0) return "Rs. " + balanceStr + " " + Wt::WString::tr("GS.Payable");
				else return "Rs. 0";
			case Account::EntityPnlAccount:
				if(balance > 0) return "Rs. " + balanceStr + " " + Wt::WString::tr("GS.Loss");
				else if(balance < 0) return "Rs. " + balanceStr + " " + Wt::WString::tr("GS.Profit");
				else return "Rs. 0";
			}
		}

		if(idx.column() == AccountList::EntityId)
		{
			auto queryModel = dynamic_cast<Wt::Dbo::QueryModel<AccountList::ResultType>*>(sourceModel());
			if(role == Wt::DisplayRole)
			{
				const AccountList::ResultType &res = queryModel->resultRow(idx.row());
				return boost::get<AccountList::EntityName>(res);
			}
			else if(role == Wt::LinkRole)
			{
				const AccountList::ResultType &res = queryModel->resultRow(idx.row());
				const auto &entityId = boost::get<AccountList::EntityId>(res);
				if(entityId.is_initialized())
					return Wt::WLink(Wt::WLink::InternalPath, "/admin/entities/entity-" + boost::lexical_cast<std::string>(*entityId));
			}
		}

		return Wt::WBatchEditProxyModel::data(idx, role);
	}

}

