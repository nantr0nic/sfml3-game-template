#include "Application.hpp"

Application::Application()
    : windowManager()
    , inputSystem(windowManager)
    , mPlayer(50.0f)
{
    windowManager.createMainWindow(1024, 768, "SFML CMake Practice");
    windowManager.getMainWindow().setFramerateLimit(60);

    mPlayer.setFillColor(sf::Color::Green);
	mPlayer.setOrigin({ 50.0f, 50.0f }); // Center the origin

	float mainWindowCenterX = windowManager.getMainWindow().getSize().x / 2.0f;
	float mainWindowCenterY = windowManager.getMainWindow().getSize().y / 2.0f;
	mPlayer.setPosition({ mainWindowCenterX, mainWindowCenterY }); // Center of the window
}

Application::~Application()
{
    // WindowManager destructor will handle window cleanup
}

void Application::run()
{
    while (windowManager.getMainWindow().isOpen())
    {
        // Calculate delta time (time since last frame)
        sf::Time deltaTime = mainClock.restart();

        // 1. Handle input / events
        processEvents();

        // 2. Update Game logic
        update(deltaTime);

        // 3. Render graphics
        render();
    }
}

void Application::processEvents()
{
    windowManager.getMainWindow().handleEvents(
        inputSystem.getEventHandles().onClose,
        inputSystem.getEventHandles().onKeyPress
    );
}

void Application::update(sf::Time deltaTime)
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

void Application::render()
{
    // can use reference to the pointer to avoid repetition
    //$ can we do this in header or constructor somehow?
    sf::RenderWindow& mainWindow = windowManager.getMainWindow();

    mainWindow.clear(sf::Color::Black);

    // Draw game objects here
    mainWindow.draw(mPlayer);

    mainWindow.display();
}