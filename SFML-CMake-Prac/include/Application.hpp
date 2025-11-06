#include "WindowManager.hpp"
#include "InputSystem.hpp"
#include "Player.hpp"

#include <memory>

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    ~Application();

    void run();

private:
    void processEvents();
    void update(sf::Time deltaTime);
    void render();

    // Resources
    WindowManager m_WindowManager;
    InputSystem m_InputSystem;
    sf::Clock m_MainClock;

    // Game Objects
    std::unique_ptr<Player> m_Player{ nullptr };
};