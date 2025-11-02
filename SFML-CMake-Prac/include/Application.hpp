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
    WindowManager windowManager;
    InputSystem inputSystem;
    sf::Clock mainClock;

    // Game Objects
    std::unique_ptr<Player> mPlayer{ nullptr };
};