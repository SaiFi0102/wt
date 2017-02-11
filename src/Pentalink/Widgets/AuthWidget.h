#ifndef TP_AUTH_WIDGET_H
#define TP_AUTH_WIDGET_H

#include <Wt/Auth/AuthWidget>

namespace PL
{
	class AuthWidget : public Wt::Auth::AuthWidget
	{
	public:
		AuthWidget(Wt::WContainerWidget *parent = nullptr);
	};
}

#endif