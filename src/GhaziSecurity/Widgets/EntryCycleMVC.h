#ifndef GS_POSITION_MVC_H
#define GS_POSITION_MVC_H

#include "Dbo/Dbos.h"
#include "Utilities/QueryProxyModel.h"
#include "Utilities/MyTemplateFormView.h"
#include "Utilities/TemplateViewsContainer.h"

#include <Wt/WTemplateFormView>


namespace GS
{
	class ExpenseCycleView;
	class IncomeCycleView;
	class ExpenseCyclesManagerModel;
	class IncomeCyclesManagerModel;

	//PositionProxyModel
	class PositionProxyModel : public QueryProxyModel<Wt::Dbo::ptr<EmployeePosition>>
	{
	public:
		PositionProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<EmployeePosition>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	//PositionView
	class PositionView : public Wt::WTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field titleField;

		PositionView(Wt::WContainerWidget *parent = nullptr);
		Wt::Signal<void> &submitted() { return _submitted; }
		Wt::Dbo::ptr<EmployeePosition> positionPtr() const { return _positionPtr; }
		Wt::WFormModel *model() const { return _model; }

	protected:
		void submit();

		Wt::WFormModel *_model = nullptr;
		Wt::Signal<void> _submitted;
		Wt::Dbo::ptr<EmployeePosition> _positionPtr;
	};

	//ServiceProxyModel
	class ServiceProxyModel : public QueryProxyModel<Wt::Dbo::ptr<ClientService>>
	{
	public:
		ServiceProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<ClientService>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	//ServiceView
	class ServiceView : public Wt::WTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field titleField;

		ServiceView(Wt::WContainerWidget *parent = nullptr);
		Wt::Signal<void> &submitted() { return _submitted; }
		Wt::Dbo::ptr<ClientService> servicePtr() const { return _servicePtr; }
		Wt::WFormModel *model() const { return _model; }

	protected:
		void submit();

		Wt::WFormModel *_model = nullptr;
		Wt::Signal<void> _submitted;
		Wt::Dbo::ptr<ClientService> _servicePtr;
	};

	//EntryCycleFormModel
	class EntryCycleFormModel : public Wt::WFormModel
	{
	public:
		static const Field startDateField;
		static const Field endDateField;
		static const Field intervalField;
		static const Field nIntervalsField;
		static const Field amountField;
		static const Field firstEntryAfterCycleField;

		EntryCycleFormModel(Wt::WObject *parent);
	};

	//EntryCycleView
	class EntryCycleView : public MyTemplateFormView
	{
	public:
		EntryCycleView(const Wt::WString &text, Wt::WContainerWidget *parent = nullptr) : MyTemplateFormView(text, parent) { }
		void initEntryCycleView(Wt::WFormModel *model);

	protected:
		void handleIntervalChanged();
	};

	//ExpenseCycleFormModel
	class ExpenseCycleFormModel : public EntryCycleFormModel
	{
	public:
		static const Field purposeField;
		static const Field positionField;

		ExpenseCycleFormModel(ExpenseCycleView *view, Wt::Dbo::ptr<ExpenseCycle> cyclePtr = Wt::Dbo::ptr<ExpenseCycle>());
		Wt::Dbo::ptr<ExpenseCycle> cyclePtr() const { return _cyclePtr; }
		void saveChanges(Wt::Dbo::ptr<ExpenseCycle> &cyclePtr, Wt::Dbo::ptr<Entity> entityPtr);

	protected:
		ExpenseCycleView *_view = nullptr;
		Wt::Dbo::ptr<ExpenseCycle> _cyclePtr;
	};

	//ExpenseCycleView
	class ExpenseCycleView : public EntryCycleView
	{
	public:
		ExpenseCycleView(Wt::Dbo::ptr<ExpenseCycle> cyclePtr, Wt::WContainerWidget *parent = nullptr);

		void handlePurposeChanged();
		void handlePositionChanged();
		Wt::WComboBox *purposeCombo() const { return _purposeCombo; }
		ProxyModelComboBox<PositionProxyModel> *positionCombo() const { return _positionCombo; }
		ExpenseCycleFormModel *model() const { return _model; }
		Wt::WDialog *createAddPositionDialog();

	protected:
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;

		Wt::WComboBox *_purposeCombo = nullptr;
		ProxyModelComboBox<PositionProxyModel> *_positionCombo = nullptr;
		ExpenseCycleFormModel *_model = nullptr;
	};

	//ExpenseCyclesContainer
	class ExpenseCyclesContainer : public TemplateViewsContainer<ExpenseCycleView, ExpenseCycleFormModel>
	{
	public:
		ExpenseCyclesContainer(ExpenseCyclesManagerModel *model, Wt::WContainerWidget *parent = nullptr);
		void addFieldWidget(Wt::Dbo::ptr<ExpenseCycle> cyclePtr = Wt::Dbo::ptr<ExpenseCycle>());

	protected:
		ExpenseCyclesManagerModel *_model = nullptr;
	};

	//IncomeCycleFormModel
	class IncomeCycleFormModel : public EntryCycleFormModel
	{
	public:
		static const Field purposeField;
		static const Field serviceField;

		IncomeCycleFormModel(IncomeCycleView *view, Wt::Dbo::ptr<IncomeCycle> cyclePtr = Wt::Dbo::ptr<IncomeCycle>());
		Wt::Dbo::ptr<IncomeCycle> cyclePtr() const { return _cyclePtr; }
		void saveChanges(Wt::Dbo::ptr<IncomeCycle> &cyclePtr, Wt::Dbo::ptr<Entity> entityPtr);

	protected:
		IncomeCycleView *_view = nullptr;
		Wt::Dbo::ptr<IncomeCycle> _cyclePtr;
	};

	//IncomeCycleView
	class IncomeCycleView : public EntryCycleView
	{
	public:
		IncomeCycleView(Wt::Dbo::ptr<IncomeCycle> cyclePtr, Wt::WContainerWidget *parent = nullptr);

		void handlePurposeChanged();
		void handleServiceChanged();
		Wt::WComboBox *purposeCombo() const { return _purposeCombo; }
		ProxyModelComboBox<ServiceProxyModel> *serviceCombo() const { return _serviceCombo; }
		IncomeCycleFormModel *model() const { return _model; }
		Wt::WDialog *createAddServiceDialog();

	protected:
		virtual Wt::WWidget *createFormWidget(Wt::WFormModel::Field field) override;

		Wt::WComboBox *_purposeCombo = nullptr;
		ProxyModelComboBox<ServiceProxyModel> *_serviceCombo = nullptr;
		IncomeCycleFormModel *_model = nullptr;
	};

	//IncomeCyclesContainer
	class IncomeCyclesContainer : public TemplateViewsContainer<IncomeCycleView, IncomeCycleFormModel>
	{
	public:
		IncomeCyclesContainer(IncomeCyclesManagerModel *model, Wt::WContainerWidget *parent = nullptr);
		void addFieldWidget(Wt::Dbo::ptr<IncomeCycle> cyclePtr = Wt::Dbo::ptr<IncomeCycle>());

	protected:
		IncomeCyclesManagerModel *_model = nullptr;
	};

}

#endif