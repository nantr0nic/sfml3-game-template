#include "WindowManager.hpp"
#include <functional>

struct gameEvents
{
	std::function<void(const sf::Event::Closed&)> onClose;
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress;
};

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

    // Move this to InputManager later
    void initEventHandlers();

    // Resources
    WindowManager windowManager;
    sf::CircleShape mPlayer;
    gameEvents mEvents;

    // Constants and Configuration
    // move this too (later)
    const float playerSpeed = 100.0f;
};