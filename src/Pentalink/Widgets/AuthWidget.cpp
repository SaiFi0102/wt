#include "Widgets/AuthWidget.h"
#include "Application/WApplication.h"
#include "Application/WServer.h"

namespace PL
{

	AuthWidget::AuthWidget(Wt::WContainerWidget *parent)
		: Wt::Auth::AuthWidget(SERVER->getAuthService(), APP->userDatabase(), APP->authLogin(), parent)
	{
		WServer *server = SERVER;

		model()->addPasswordAuth(&server->getPasswordService());
		model()->addOAuth(server->getOAuthServices());
		setRegistrationEnabled(false);
		processEnvironment();
	}

}
