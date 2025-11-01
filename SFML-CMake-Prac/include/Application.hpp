#include "WindowManager.hpp"
#include "InputSystem.hpp"

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
    sf::CircleShape mPlayer;

    // Constants and Configuration
    // move this too (later)
    const float playerSpeed = 100.0f;
};