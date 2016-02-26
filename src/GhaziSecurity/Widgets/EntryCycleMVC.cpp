#include "Widgets/EntryCycleMVC.h"
#include "Widgets/EntityView.h"
#include "Application/WApplication.h"

#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WComboBox>
#include <Wt/WDateEdit>
#include <Wt/WIntValidator>
#include <Wt/WDoubleValidator>

namespace GS
{
	//POSITION VIEW
	const Wt::WFormModel::Field PositionView::titleField = "title";

	PositionView::PositionView(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplateFormView(tr("GS.Admin.PositionView"), parent)
	{
		_model = new Wt::WFormModel(this);
		model()->addField(titleField);

		Wt::WLineEdit *title = new Wt::WLineEdit();
		title->setMaxLength(70);
		setFormWidget(titleField, title);
		model()->setValidator(titleField, new Wt::WValidator(true));

		Wt::WPushButton *submit = new Wt::WPushButton(tr("GS.Submit"));
		submit->clicked().connect(this, &PositionView::submit);
		bindWidget("submit", submit);

		updateView(model());
	}

	void PositionView::submit()
	{
		WApplication *app = WApplication::instance();
		updateModel(model());

		try
		{
			if(model()->validate())
			{
				Wt::Dbo::Transaction t(app->session());
				if(!_positionPtr)
					_positionPtr = app->session().add(new EmployeePosition());

				_positionPtr.modify()->title = model()->valueText(titleField).toUTF8();
				if(app->positionQueryModel())
					app->positionQueryModel()->reload();
				t.commit();
				updateView(model());
				submitted().emit();
			}
			else
				updateView(model());
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "PositionView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}
	
	//POSITION PROXY MODEL
	PositionProxyModel::PositionProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<EmployeePosition>> *sourceModel, Wt::WObject *parent /*= nullptr*/)
		: QueryProxyModel<Wt::Dbo::ptr<EmployeePosition>>(parent)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &PositionProxyModel::addAdditionalRows);
	}

	void PositionProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), Wt::WString::tr("GS.SelectPosition"));
			setData(index(0, 0), false, Wt::UserRole + 1);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), Wt::WString::tr("GS.AddNewPosition"));
			setData(index(lastRow, 0), true, Wt::UserRole + 1);
		}
	}

	//SERVICE VIEW
	const Wt::WFormModel::Field ServiceView::titleField = "title";

	ServiceView::ServiceView(Wt::WContainerWidget *parent /*= nullptr*/)
		: Wt::WTemplateFormView(tr("GS.Admin.ServiceView"), parent)
	{
		_model = new Wt::WFormModel(this);
		model()->addField(titleField);

		Wt::WLineEdit *title = new Wt::WLineEdit();
		title->setMaxLength(70);
		setFormWidget(titleField, title);
		model()->setValidator(titleField, new Wt::WValidator(true));

		Wt::WPushButton *submit = new Wt::WPushButton(tr("GS.Submit"));
		submit->clicked().connect(this, &ServiceView::submit);
		bindWidget("submit", submit);

		updateView(model());
	}

	void ServiceView::submit()
	{
		WApplication *app = WApplication::instance();
		updateModel(model());

		try
		{
			if(model()->validate())
			{
				Wt::Dbo::Transaction t(app->session());
				if(!_servicePtr)
					_servicePtr = app->session().add(new ClientService());

				_servicePtr.modify()->title = model()->valueText(titleField).toUTF8();
				if(app->serviceQueryModel())
				app->serviceQueryModel()->reload();
				t.commit();
				updateView(model());
				submitted().emit();
			}
			else
				updateView(model());
		}
		catch(Wt::Dbo::Exception &e)
		{
			Wt::log("error") << "ServiceView::submit(): Dbo error(" << e.code() << "): " << e.what();
			app->showDbBackendError(e.code());
		}
	}

	//SERVICE PROXY MODEL
	ServiceProxyModel::ServiceProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<ClientService>> *sourceModel, Wt::WObject *parent /*= nullptr*/)
		: QueryProxyModel<Wt::Dbo::ptr<ClientService>>(parent)
	{
		setSourceModel(sourceModel);
		addAdditionalRows();
		layoutChanged().connect(this, &ServiceProxyModel::addAdditionalRows);
	}

	void ServiceProxyModel::addAdditionalRows()
	{
		if(insertRow(0))
		{
			setData(index(0, 0), Wt::WString::tr("GS.SelectService"));
			setData(index(0, 0), false, Wt::UserRole + 1);
		}

		int lastRow = rowCount();
		if(insertRow(lastRow))
		{
			setData(index(lastRow, 0), Wt::WString::tr("GS.AddNewService"));
			setData(index(lastRow, 0), true, Wt::UserRole + 1);
		}
	}

	//PAYMENT CYCLE MODEL
	const Wt::WFormModel::Field EntryCycleFormModel::startDateField = "startDate";
	const Wt::WFormModel::Field EntryCycleFormModel::endDateField = "endDate";
	const Wt::WFormModel::Field EntryCycleFormModel::intervalField = "interval";
	const Wt::WFormModel::Field EntryCycleFormModel::nIntervalsField = "nIntervals";
	const Wt::WFormModel::Field EntryCycleFormModel::amountField = "amount";
	const Wt::WFormModel::Field EntryCycleFormModel::firstEntryAfterCycleField = "firstEntryAfterCycle";

	EntryCycleFormModel::EntryCycleFormModel(Wt::WObject *parent)
		: Wt::WFormModel(parent)
	{
		addField(startDateField);
		addField(endDateField);
		addField(intervalField);
		addField(nIntervalsField);
		addField(amountField);
		addField(firstEntryAfterCycleField);
	}

	//ENTRY CYCLE VIEW
	void EntryCycleView::initEntryCycleView(Wt::WFormModel *model)
	{
		auto startDateEdit = new Wt::WDateEdit();
		startDateEdit->setPlaceholderText(APP->locale().dateFormat());
		auto startDateValidator = new Wt::WDateValidator();
		startDateValidator->setMandatory(true);
		startDateValidator->setBottom(Wt::WDate(boost::gregorian::day_clock::local_day()));
		model->setValidator(EntryCycleFormModel::startDateField, startDateValidator);
		setFormWidget(EntryCycleFormModel::startDateField, startDateEdit);

		auto endDateEdit = new Wt::WDateEdit();
		endDateEdit->setPlaceholderText(tr("GS.EmptyEndDate"));
		auto endDateValidator = new Wt::WDateValidator();
		model->setValidator(EntryCycleFormModel::endDateField, endDateValidator);

		startDateEdit->changed().connect(std::bind([startDateEdit, endDateValidator]() {
			endDateValidator->setBottom(startDateEdit->date().addDays(1));
		}));
		setFormWidget(EntryCycleFormModel::endDateField, endDateEdit);

		auto intervalCombo = new Wt::WComboBox();
		intervalCombo->insertItem(DailyInterval, tr("GS.Days"));
		intervalCombo->insertItem(WeeklyInterval, tr("GS.Weeks"));
		intervalCombo->insertItem(MonthlyInterval, tr("GS.Months"));
		intervalCombo->insertItem(YearlyInterval, tr("GS.Years"));
		intervalCombo->setCurrentIndex(MonthlyInterval);
		setFormWidget(EntryCycleFormModel::intervalField, intervalCombo);

		auto amountEdit = new Wt::WLineEdit();
		auto amountValidator = new Wt::WDoubleValidator();
		amountValidator->setBottom(0);
		amountValidator->setMandatory(true);
		model->setValidator(EntryCycleFormModel::amountField, amountValidator);
		setFormWidget(EntryCycleFormModel::amountField, amountEdit);

		auto nIntervalsEdit = new Wt::WLineEdit();
		auto nIntervalsValidator = new Wt::WIntValidator();
		nIntervalsValidator->setBottom(1);
		nIntervalsValidator->setMandatory(true);
		model->setValidator(EntryCycleFormModel::nIntervalsField, nIntervalsValidator);
		setFormWidget(EntryCycleFormModel::nIntervalsField, nIntervalsEdit);

		auto firstEntryOnEdit = new Wt::WComboBox();
		firstEntryOnEdit->insertItem(0, tr("GS.OnStartingDate"));
		firstEntryOnEdit->insertItem(1, Wt::WString());
		setFormWidget(EntryCycleFormModel::firstEntryAfterCycleField, firstEntryOnEdit);

		nIntervalsEdit->changed().connect(this, &EntryCycleView::handleIntervalChanged);
		intervalCombo->changed().connect(this, &EntryCycleView::handleIntervalChanged);
		handleIntervalChanged();
	}

	void EntryCycleView::handleIntervalChanged()
	{
		auto nIntervalsEdit = resolve<Wt::WLineEdit*>(EntryCycleFormModel::nIntervalsField);
		auto intervalCombo = resolve<Wt::WComboBox*>(EntryCycleFormModel::intervalField);
		auto firstEntryOnEdit = resolve<Wt::WComboBox*>(EntryCycleFormModel::firstEntryAfterCycleField);

		firstEntryOnEdit->setItemText(1, tr("GS.AfterTheInterval"));

		if(!nIntervalsEdit->valueText().empty() && nIntervalsEdit->validate() == Wt::WValidator::Valid)
		{
			int nIntervals = boost::lexical_cast<int>(nIntervalsEdit->valueText());
			switch(intervalCombo->currentIndex())
			{
			case DailyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("GS.AfterNDays", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case WeeklyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("GS.AfterNWeeks", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case MonthlyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("GS.AfterNMonths", nIntervals).arg(nIntervalsEdit->valueText())); break;
			case YearlyInterval: firstEntryOnEdit->setItemText(1, Wt::WString::trn("GS.AfterNYears", nIntervals).arg(nIntervalsEdit->valueText())); break;
			}
		}
	}

	//EXPENSE CYCLE MODEL
	const Wt::WFormModel::Field ExpenseCycleFormModel::purposeField = "purpose";
	const Wt::WFormModel::Field ExpenseCycleFormModel::positionField = "position";

	ExpenseCycleFormModel::ExpenseCycleFormModel(ExpenseCycleView *view, Wt::Dbo::ptr<ExpenseCycle> cyclePtr /*= Wt::Dbo::ptr<ExpenseCycle>()*/)
		: EntryCycleFormModel(view), _view(view), _cyclePtr(cyclePtr)
	{
		addField(purposeField);
		addField(positionField);

		if(cyclePtr)
		{
			setValue(startDateField, cyclePtr->startDate);
			setValue(endDateField, cyclePtr->endDate);
			setValue(purposeField, (int)cyclePtr->purpose);
			setValue(positionField, cyclePtr->positionPtr);
			setValue(intervalField, (int)cyclePtr->interval);
			setValue(nIntervalsField, cyclePtr->nIntervals);
			setValue(amountField, cyclePtr->amount);
			setValue(firstEntryAfterCycleField, cyclePtr->firstEntryAfterCycle ? 1 : 0);
		}
	}

	void ExpenseCycleFormModel::saveChanges(Wt::Dbo::ptr<ExpenseCycle> &inputCyclePtr, Wt::Dbo::ptr<Entity> entityPtr)
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		Wt::Dbo::ptr<Account> accountPtr = app->accountsDatabase().findOrCreateEntityAccount(entityPtr);
		bool newCycle = false;

		if(inputCyclePtr)
			_cyclePtr = inputCyclePtr;
		else
		{
			if(!_cyclePtr)
			{
				_cyclePtr = app->session().add(new ExpenseCycle());
				newCycle = true;
			}
		}

		_cyclePtr.modify()->accountPtr = accountPtr;
		_cyclePtr.modify()->startDate = boost::any_cast<Wt::WDate>(value(startDateField));
		_cyclePtr.modify()->endDate = boost::any_cast<Wt::WDate>(value(endDateField));
		_cyclePtr.modify()->purpose = ExpenseCycle::Purpose(boost::any_cast<int>(value(purposeField)));
		_cyclePtr.modify()->interval = CycleInterval(boost::any_cast<int>(value(intervalField)));
		_cyclePtr.modify()->nIntervals = boost::lexical_cast<int>(valueText(nIntervalsField).toUTF8());
		_cyclePtr.modify()->amount = boost::lexical_cast<double>(valueText(amountField).toUTF8());
		_cyclePtr.modify()->firstEntryAfterCycle = boost::any_cast<int>(value(firstEntryAfterCycleField)) == 1;

		boost::any positionVal = value(positionField);
		if(positionVal.empty())
			_cyclePtr.modify()->positionPtr = Wt::Dbo::ptr<EmployeePosition>();
		else
			_cyclePtr.modify()->positionPtr = boost::any_cast<Wt::Dbo::ptr<EmployeePosition>>(positionVal);

		app->accountsDatabase().createPendingCycleEntry(_cyclePtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		t.commit();

		inputCyclePtr = _cyclePtr;
	}

	//EXPENSE CYCLE VIEW
	ExpenseCycleView::ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: EntryCycleView(tr("GS.Admin.ExpenseCycleView"), parent)
	{
		_model = new ExpenseCycleFormModel(this, cyclePtr);
		initEntryCycleView(_model);

		WApplication *app = WApplication::instance();
		app->initPositionQueryModel();

		_purposeCombo = new Wt::WComboBox();
		_purposeCombo->insertItem(ExpenseCycle::UnspecifiedPurpose, tr("GS.UnspecificPurpose"));
		_purposeCombo->insertItem(ExpenseCycle::Salary, tr("GS.Salary"));
		_purposeCombo->changed().connect(this, &ExpenseCycleView::handlePurposeChanged);
		setFormWidget(ExpenseCycleFormModel::purposeField, _purposeCombo);

		model()->setVisible(ExpenseCycleFormModel::positionField, false);
		updateView(model());
	}

	Wt::WWidget *ExpenseCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == ExpenseCycleFormModel::positionField)
		{
			delete _positionCombo;
			_positionCombo = new ProxyModelComboBox<PositionProxyModel>(APP->positionProxyModel());
			_positionCombo->changed().connect(this, &ExpenseCycleView::handlePositionChanged);
			return _positionCombo;
		}
		return nullptr;
	}

	void ExpenseCycleView::handlePurposeChanged()
	{
		updateModel(model());
		model()->setVisible(ExpenseCycleFormModel::positionField, _purposeCombo->currentIndex() == ExpenseCycle::Salary);
		updateView(model());
	}

	void ExpenseCycleView::handlePositionChanged()
	{
		boost::any v = _positionCombo->model()->index(_positionCombo->currentIndex(), 0).data(Wt::UserRole + 1);
		if(v.empty())
			return;
		
		if(boost::any_cast<bool>(v) == true)
			createAddPositionDialog();
	}

	Wt::WDialog *ExpenseCycleView::createAddPositionDialog()
	{
		updateModel(model());
		Wt::WDialog *dialog = new Wt::WDialog(tr("GS.AddNewPosition"), this);
		dialog->setClosable(true);
		dialog->setWidth(Wt::WLength(500));
		PositionView *positionView = new PositionView(dialog->contents());

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			delete dialog;
			if(code == Wt::WDialog::Rejected)
			{
				updateModel(model());
				model()->setValue(ExpenseCycleFormModel::positionField, Wt::Dbo::ptr<EmployeePosition>());
				updateView(model());
			}
		}, std::placeholders::_1));

		positionView->submitted().connect(std::bind([=]() {
			updateModel(model());
			model()->setValue(ExpenseCycleFormModel::positionField, positionView->positionPtr());
			updateView(model());
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

	//EXPENSE CYCLES CONTAINER
	ExpenseCyclesContainer::ExpenseCyclesContainer(ExpenseCyclesManagerModel *model, Wt::WContainerWidget *parent /*= nullptr*/)
		: TemplateViewsContainer<ExpenseCycleView, ExpenseCycleFormModel>(parent), _model(model)
	{
		for(int i = 0; i < _model->ptrVector().size(); ++i)
			addFieldWidget(_model->ptrVector()[i]);
	}

	void ExpenseCyclesContainer::addFieldWidget(Wt::Dbo::ptr<ExpenseCycle> cyclePtr /*= Wt::Dbo::ptr<ExpenseCycle>()*/)
	{
		auto w = new ExpenseCycleView(cyclePtr);
		addWidget(w);
		w->bindInt("index", count());
	}

	//INCOME CYCLE MODEL
	const Wt::WFormModel::Field IncomeCycleFormModel::purposeField = "purpose";
	const Wt::WFormModel::Field IncomeCycleFormModel::serviceField = "service";

	IncomeCycleFormModel::IncomeCycleFormModel(IncomeCycleView *view, Wt::Dbo::ptr<IncomeCycle> cyclePtr /*= Wt::Dbo::ptr<IncomeCycle>()*/)
		: EntryCycleFormModel(view), _view(view), _cyclePtr(cyclePtr)
	{
		addField(purposeField);
		addField(serviceField);

		if(cyclePtr)
		{
			setValue(startDateField, cyclePtr->startDate);
			setValue(endDateField, cyclePtr->endDate);
			setValue(purposeField, (int)cyclePtr->purpose);
			setValue(serviceField, cyclePtr->servicePtr);
			setValue(intervalField, (int)cyclePtr->interval);
			setValue(nIntervalsField, cyclePtr->nIntervals);
			setValue(amountField, cyclePtr->amount);
			setValue(firstEntryAfterCycleField, cyclePtr->firstEntryAfterCycle ? 1 : 0);
		}
	}

	void IncomeCycleFormModel::saveChanges(Wt::Dbo::ptr<IncomeCycle> &inputCyclePtr, Wt::Dbo::ptr<Entity> entityPtr)
	{
		if(!valid())
			return;

		WApplication *app = WApplication::instance();
		Wt::Dbo::Transaction t(app->session());

		Wt::Dbo::ptr<Account> accountPtr = app->accountsDatabase().findOrCreateEntityAccount(entityPtr);
		bool newCycle = false;

		if(inputCyclePtr)
			_cyclePtr = inputCyclePtr;
		else
		{
			if(!_cyclePtr)
			{
				_cyclePtr = app->session().add(new IncomeCycle());
				newCycle = true;
			}
		}

		_cyclePtr.modify()->accountPtr = accountPtr;
		_cyclePtr.modify()->startDate = boost::any_cast<Wt::WDate>(value(startDateField));
		_cyclePtr.modify()->endDate = boost::any_cast<Wt::WDate>(value(endDateField));
		_cyclePtr.modify()->purpose = IncomeCycle::Purpose(boost::any_cast<int>(value(purposeField)));
		_cyclePtr.modify()->interval = CycleInterval(boost::any_cast<int>(value(intervalField)));
		_cyclePtr.modify()->nIntervals = boost::lexical_cast<int>(valueText(nIntervalsField).toUTF8());
		_cyclePtr.modify()->amount = boost::lexical_cast<double>(valueText(amountField).toUTF8());
		_cyclePtr.modify()->firstEntryAfterCycle = boost::any_cast<int>(value(firstEntryAfterCycleField)) == 1;

		boost::any serviceVal = value(serviceField);
		if(serviceVal.empty())
			_cyclePtr.modify()->servicePtr = Wt::Dbo::ptr<ClientService>();
		else
			_cyclePtr.modify()->servicePtr = boost::any_cast<Wt::Dbo::ptr<ClientService>>(serviceVal);

		app->accountsDatabase().createPendingCycleEntry(_cyclePtr, Wt::Dbo::ptr<AccountEntry>(), boost::posix_time::microsec_clock::local_time());

		t.commit();

		inputCyclePtr = _cyclePtr;
	}

	//INCOME CYCLE VIEW
	IncomeCycleView::IncomeCycleView(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::WContainerWidget *parent /*= nullptr*/)
		: EntryCycleView(tr("GS.Admin.IncomeCycleView"), parent)
	{
		_model = new IncomeCycleFormModel(this, cyclePtr);
		initEntryCycleView(_model);

		WApplication *app = WApplication::instance();
		app->initServiceQueryModel();

		_purposeCombo = new Wt::WComboBox();
		_purposeCombo->insertItem(IncomeCycle::UnspecifiedPurpose, tr("GS.UnspecificPurpose"));
		_purposeCombo->insertItem(IncomeCycle::Services, tr("GS.Services"));
		_purposeCombo->changed().connect(this, &IncomeCycleView::handlePurposeChanged);
		setFormWidget(IncomeCycleFormModel::purposeField, _purposeCombo);

		model()->setVisible(IncomeCycleFormModel::serviceField, false);
		updateView(model());
	}

	Wt::WWidget *IncomeCycleView::createFormWidget(Wt::WFormModel::Field field)
	{
		if(field == IncomeCycleFormModel::serviceField)
		{
			delete _serviceCombo;
			_serviceCombo = new ProxyModelComboBox<ServiceProxyModel>(APP->serviceProxyModel());
			_serviceCombo->changed().connect(this, &IncomeCycleView::handleServiceChanged);
			return _serviceCombo;
		}
		return nullptr;
	}

	void IncomeCycleView::handlePurposeChanged()
	{
		updateModel(model());
		model()->setVisible(IncomeCycleFormModel::serviceField, _purposeCombo->currentIndex() == IncomeCycle::Services);
		updateView(model());
	}

	void IncomeCycleView::handleServiceChanged()
	{
		boost::any v = _serviceCombo->model()->index(_serviceCombo->currentIndex(), 0).data(Wt::UserRole + 1);
		if(v.empty())
			return;

		if(boost::any_cast<bool>(v) == true)
			createAddServiceDialog();
	}

	Wt::WDialog *IncomeCycleView::createAddServiceDialog()
	{
		updateModel(model());
		Wt::WDialog *dialog = new Wt::WDialog(tr("GS.AddNewService"), this);
		dialog->setClosable(true);
		dialog->setWidth(Wt::WLength(500));
		ServiceView *serviceView = new ServiceView(dialog->contents());

		dialog->finished().connect(std::bind([=](Wt::WDialog::DialogCode code) {
			delete dialog;
			if(code == Wt::WDialog::Rejected)
			{
				updateModel(model());
				model()->setValue(IncomeCycleFormModel::serviceField, Wt::Dbo::ptr<ClientService>());
				updateView(model());
			}
		}, std::placeholders::_1));

		serviceView->submitted().connect(std::bind([=]() {
			updateModel(model());
			model()->setValue(IncomeCycleFormModel::serviceField, serviceView->servicePtr());
			updateView(model());
			dialog->accept();
		}));

		dialog->show();
		return dialog;
	}

	//INCOME CYCLES CONTAINER
	IncomeCyclesContainer::IncomeCyclesContainer(IncomeCyclesManagerModel *model, Wt::WContainerWidget *parent /*= nullptr*/)
		: TemplateViewsContainer<IncomeCycleView, IncomeCycleFormModel>(parent), _model(model)
	{
		for(int i = 0; i < _model->ptrVector().size(); ++i)
			addFieldWidget(_model->ptrVector()[i]);
	}

	void IncomeCyclesContainer::addFieldWidget(Wt::Dbo::ptr<IncomeCycle> cyclePtr /*= Wt::Dbo::ptr<IncomeCycle>()*/)
	{
		auto w = new IncomeCycleView(cyclePtr);
		addWidget(w);
		w->bindInt("index", count());
	}

}
