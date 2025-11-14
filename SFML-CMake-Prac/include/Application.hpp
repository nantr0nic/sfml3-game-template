#include "AppContext.hpp"
#include "StateManager.hpp"

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    ~Application();

    void initMainWindow();
    void initResources();

    void run();

private:
    void processEvents();
    void update(sf::Time deltaTime);
    void render();

    // Resources
    AppContext m_AppContext;
    StateManager m_StateManager;
};