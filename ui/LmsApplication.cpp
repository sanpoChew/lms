#include <Wt/WBootstrapTheme>
#include "auth/LmsAuth.hpp"
#include "LmsHome.hpp"

#include "LmsApplication.hpp"

namespace UserInterface {

Wt::WApplication*
LmsApplication::create(const Wt::WEnvironment& env, boost::filesystem::path dbPath)
{
	/*
	 * You could read information from the environment to decide whether
	 * the user has permission to start a new application
	 */
	std::cout << "Creating new Application" << std::endl;
	return new LmsApplication(env, dbPath);
}

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the Wt::WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
*/
LmsApplication::LmsApplication(const Wt::WEnvironment& env, boost::filesystem::path dbPath)
: Wt::WApplication(env),
 _sessionData(dbPath),
 _home(nullptr)
{

	Wt::WBootstrapTheme *bootstrapTheme = new Wt::WBootstrapTheme(this);
	bootstrapTheme->setVersion(Wt::WBootstrapTheme::Version3);
	bootstrapTheme->setResponsive(true);
	setTheme(bootstrapTheme);

	// Add a resource bundle
	messageResourceBundle().use(appRoot() + "templates");

	setTitle("LMS");                               // application title

	_sessionData.getDatabaseHandler().getLogin().changed().connect(this, &LmsApplication::handleAuthEvent);

	LmsAuth *authWidget = new LmsAuth(_sessionData.getDatabaseHandler());

	authWidget->model()->addPasswordAuth(&Database::Handler::getPasswordService());
	authWidget->setRegistrationEnabled(false);

	authWidget->processEnvironment();

	root()->addWidget(authWidget);

}



void
LmsApplication::handleAuthEvent(void)
{
	if (_sessionData.getDatabaseHandler().getLogin().loggedIn())
	{
		if (_home == nullptr) {
			_home = new LmsHome(_sessionData, root() );
		}
		else
			std::cerr << "Already logged in??" << std::endl;
	}
	else
	{
		std::cerr << "user log out" << std::endl;
		if (_home != nullptr) {
			delete _home;
			_home = nullptr;

			// Hack: quit/redirect in order to avoid 'signal not exposed' problems
			// TODO, investigate/remove?
			quit();
			redirect("/");

		}
		else
			std::cerr << "Already logged out??" << std::endl;
	}
}

} // namespace UserInterface


