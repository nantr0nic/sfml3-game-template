#include "AppContext.hpp"
#include "Managers/StateManager.hpp"

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    ~Application();

    void run();

private:
    void initMainWindow();
    void initResources();

    void processEvents();
    void update(sf::Time deltaTime);
    void render();

    // Resources
    AppContext m_AppContext;
    StateManager m_StateManager;
};