#include <SFML/Graphics.hpp>
#include <functional>

struct gameEvents
{
	std::function<void(const sf::Event::Closed&)> onClose;
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress;
};

class Game
{
public:
	Game();
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	
	void run();

private:
	void processEvents();				// Handle user input
	void processEvents2();				// New event handling method
	void update(sf::Time deltaTime);	// Handle game logic
	void render();						// Handle rendering

	void initEventHandlers();			// Initialize event handlers

	// Resources
	sf::RenderWindow mainWindow;
	sf::CircleShape mPlayer;
	gameEvents mEvents;

	// Constants and Configuration
	const float playerSpeed = 100.0f;	// Pixels per second
};