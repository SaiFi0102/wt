#ifndef GS_COUNTRYCITYVIEWS_WIDGET_H
#define GS_COUNTRYCITYVIEWS_WIDGET_H

#include "Dbo/Dbos.h"
#include "Utilities/QueryProxyModel.h"
#include "Utilities/MyTemplateFormView.h"
#include "Utilities/TemplateViewsContainer.h"

#include <Wt/Dbo/QueryModel>
#include <Wt/WSortFilterProxyModel>
#include <Wt/WTemplateFormView>
#include <Wt/WValidator>

namespace GS
{
	class LocationView;
	class LocationsManagerModel;

	class CountryProxyModel : public QueryProxyModel<Wt::Dbo::ptr<Country>>
	{
	public:
		CountryProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<Country>> *sourceModel, Wt::WObject *parent = nullptr);

	protected:
		void addAdditionalRows();
	};

	//LocationFilterModel
	class CityFilterModel : public Wt::WSortFilterProxyModel
	{
	public:
		CityFilterModel(Wt::WObject *parent = nullptr);
		virtual bool filterAcceptRow(int sourceRow, const Wt::WModelIndex &sourceParent) const override;

		void setCountryCode(const std::string code);
		std::string countryCode() const { return _countryCode; }

	protected:
		std::string _countryCode;
	};

	class CityProxyModel : public QueryProxyModel<Wt::Dbo::ptr<City>>
	{
	public:
		CityProxyModel(Wt::Dbo::QueryModel<Wt::Dbo::ptr<City>> *sourceModel, Wt::WObject *parent = nullptr);
		CityFilterModel *filterModel() const { return _filterModel; }

	protected:
		void addAdditionalRows();
		CityFilterModel *_filterModel = nullptr;
	};

	class CountryView : public Wt::WTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field codeField;
		static const Wt::WFormModel::Field nameField;

		CountryView(Wt::WContainerWidget *parent = nullptr);
		Wt::Signal<void> &submitted() { return _submitted; }
		Wt::Dbo::ptr<Country> countryPtr() const { return _countryPtr; }
		Wt::WFormModel *model() const { return _model; }

	protected:
		void submit();

		Wt::WFormModel *_model = nullptr;
		Wt::Signal<void> _submitted;
		Wt::Dbo::ptr<Country> _countryPtr;
	};

	class CountryCodeValidator : public Wt::WValidator
	{
	public:
		CountryCodeValidator(bool mandatory = false, Wt::WObject *parent = nullptr)
			: Wt::WValidator(mandatory, parent)
		{ }
		virtual Result validate(const Wt::WString &input) const override;
	};

	class CityView : public MyTemplateFormView
	{
	public:
		static const Wt::WFormModel::Field countryField;
		static const Wt::WFormModel::Field nameField;

		CityView(Wt::WContainerWidget *parent = nullptr);
		Wt::Signal<void> &submitted() { return _submitted; }
		Wt::Dbo::ptr<City> cityPtr() const { return _cityPtr; }
		Wt::WFormModel *model() const { return _model; }

	protected:
		void submit();
		Wt::WFormModel *_model = nullptr;
		Wt::Signal<void> _submitted;
		Wt::Dbo::ptr<City> _cityPtr;
	};

	class CityCountryValidator : public Wt::WValidator
	{
	public:
		CityCountryValidator(Wt::WComboBox *countryCB)
			: Wt::WValidator((Wt::WObject*)countryCB), _countryCB(countryCB)
		{ }
		virtual Result validate(const Wt::WString &input) const override;

	protected:
		Wt::WComboBox *_countryCB;
	};

	class LocationFormModel : public Wt::WFormModel
	{
	public:
		static const Field countryField;
		static const Field cityField;
		static const Field addressField;

		LocationFormModel(LocationView *view, Wt::Dbo::ptr<Location> locationPtr = Wt::Dbo::ptr<Location>());
		Wt::Dbo::ptr<Location> locationPtr() const { return _locationPtr; }
		void saveChanges(Wt::Dbo::ptr<Location> &locationPtr, Wt::Dbo::ptr<Entity> entityPtr);

	protected:
		LocationView *_view = nullptr;
		Wt::Dbo::ptr<Location> _locationPtr;
	};

	class LocationView : public MyTemplateFormView
	{
	public:
		LocationView(Wt::Dbo::ptr<Location> locationPtr, Wt::WContainerWidget *parent = nullptr);

		void handleCountryChanged();
		void handleCityChanged();
		Wt::WLineEdit *addressEdit() const { return _addressEdit; }
		ProxyModelComboBox<CountryProxyModel> *countryCombo() const { return _countryCombo; }
		ProxyModelComboBox<CityProxyModel> *cityCombo() const { return _cityCombo; }
		LocationFormModel *model() const { return _model; }
		Wt::WDialog *createAddCountryDialog();
		Wt::WDialog *createAddCityDialog();

	protected:
		Wt::WLineEdit *_addressEdit = nullptr;
		ProxyModelComboBox<CountryProxyModel> *_countryCombo = nullptr;
		ProxyModelComboBox<CityProxyModel> *_cityCombo = nullptr;
		CityProxyModel *_cityProxyModel = nullptr;
		LocationFormModel *_model = nullptr;
	};

	//LocationsContainer
	class LocationsContainer : public TemplateViewsContainer<LocationView, LocationFormModel>
	{
	public:
		LocationsContainer(LocationsManagerModel *model, Wt::WContainerWidget *parent = nullptr);
		void addFieldWidget(Wt::Dbo::ptr<Location> locationPtr = Wt::Dbo::ptr<Location>());

	protected:
		LocationsManagerModel *_model = nullptr;
	};

}

#endif