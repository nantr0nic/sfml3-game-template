#include "Game.hpp"

// Initialize the game / constructor for Game
Game::Game()
	: mainWindow(
		sf::VideoMode({ 1028, 768 }),
		"SFML CMake Practice",
		sf::Style::Default,
		sf::State::Windowed
	),
	mPlayer(50.0f) // Circle with radius 50
{
	initEventHandlers();

	mainWindow.setFramerateLimit(60);
	mPlayer.setFillColor(sf::Color::Green);
	mPlayer.setOrigin({ 50.0f, 50.0f }); // Center the origin

	float mainWindowCenterX = mainWindow.getSize().x / 2.0f;
	float mainWindowCenterY = mainWindow.getSize().y / 2.0f;
	mPlayer.setPosition({ mainWindowCenterX, mainWindowCenterY }); // Center of the window
}

// Initialize event handlers
// lambda functions declared as std::function in header, then
// passed to handleEvents method
void Game::initEventHandlers()
{
	mEvents.onClose = [this](const sf::Event::Closed&)
	{
		mainWindow.close();
	};

	mEvents.onKeyPress = [this](const sf::Event::KeyPressed& keyPressed)
	{
		if (keyPressed.scancode == sf::Keyboard::Scancode::Escape)
		{
			mainWindow.close();
		}
	};
}

// Main game loop
void Game::run()
{
	sf::Clock mainClock;

	while (mainWindow.isOpen())
	{
		// Calculate delta time (time since last frame)
		sf::Time deltaTime = mainClock.restart();
		
		// 1. Handle input / events
		//processEvents();

		// 1.5 Handle input / events with new method
		processEvents2();

		// 2. Update Game logic
		update(deltaTime);

		// 3. Render graphics
		render();
	}
}

// Process events (input handling)
// SFML 3 allows a new way to handle events -- look at processEvents2()
void Game::processEvents()
{
	while (const std::optional event = mainWindow.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			mainWindow.close();
		}
		// (Global?) Input handling can be expanded here
		else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
			{
				mainWindow.close();
			}
		}
	}
}

// Using the new handleEvents method from SFML 3
void Game::processEvents2()
{
	mainWindow.handleEvents(
		mEvents.onClose,
		mEvents.onKeyPress
	);
}

void Game::update(sf::Time deltaTime)
{
	// Move player based on keyboard input
	sf::Vector2f playerMovement(0.0f, 0.0f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W))
	{
		playerMovement.y -= playerSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A))
	{
		playerMovement.x -= playerSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S))
	{
		playerMovement.y += playerSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
	{
		playerMovement.x += playerSpeed;
	}

	// Normalized movement vector if both horizontal/vertical keys are pressed
	// to prevent faster diagonal movement
	if (playerMovement.x != 0 || playerMovement.y != 0)
	{
		// Optional: Normalize and scale by playerSpeed and deltaTime
	}

	// Apply movement: velocity * time
	mPlayer.move(playerMovement * deltaTime.asSeconds());
}

void Game::render()
{
	mainWindow.clear(sf::Color::Black);	// Black background
	// Draw the player
	mainWindow.draw(mPlayer);
	mainWindow.display();
}