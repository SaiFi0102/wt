#ifndef GS_MYTEMPLATEFORMVIEW_H
#define GS_MYTEMPLATEFORMVIEW_H

#include <Wt/WTemplateFormView>
#include <Wt/WComboBox>

namespace GS
{
	class AbstractProxyModelComboBox : public Wt::WComboBox
	{
	public:
		AbstractProxyModelComboBox(Wt::WContainerWidget *parent = nullptr) : Wt::WComboBox(parent) { }
		virtual void setViewValue(const boost::any &v) = 0;
		virtual boost::any modelValue() = 0;
	};

	template<class ProxyModel>
	class ProxyModelComboBox : public AbstractProxyModelComboBox
	{
	public:
		ProxyModelComboBox(ProxyModel *model, Wt::WContainerWidget *parent = nullptr) : AbstractProxyModelComboBox(parent)
		{
			setModel(model);
			setCurrentIndex(0);
		}
		virtual void setViewValue(const boost::any &v) override
		{
			if(v.empty())
				return;

			const auto &result = boost::any_cast<ProxyModel::Result>(v);
			auto proxyModel = dynamic_cast<ProxyModel*>(model());
			setCurrentIndex(std::max(0, proxyModel->indexOf(result)));
		}
		virtual boost::any modelValue() override
		{
			auto proxyModel = dynamic_cast<ProxyModel*>(model());
			return proxyModel->resultRow(currentIndex());
		}
	};

	class MyTemplateFormView : public Wt::WTemplateFormView
	{
	public:
		MyTemplateFormView(Wt::WContainerWidget *parent = nullptr) : Wt::WTemplateFormView(parent) { }
		MyTemplateFormView(const Wt::WString &text, Wt::WContainerWidget *parent = nullptr) : Wt::WTemplateFormView(text, parent) { }

		virtual bool updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
		virtual bool updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override;
	};
}

#endif