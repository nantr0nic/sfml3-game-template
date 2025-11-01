#include "Application.hpp"

Application::Application()
    : windowManager()
    , mPlayer(50.0f)
{
    windowManager.createMainWindow(1024, 768, "SFML CMake Practice");
    windowManager.getMainWindow().setFramerateLimit(60);

    initEventHandlers();

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

// Initialize event handlers
// lambda functions declared as std::function in header, then
// passed to handleEvents method
void Application::initEventHandlers()
{
	mEvents.onClose = [this](const sf::Event::Closed&)
	{
		windowManager.getMainWindow().close();
	};

	mEvents.onKeyPress = [this](const sf::Event::KeyPressed& keyPressed)
	{
		if (keyPressed.scancode == sf::Keyboard::Scancode::Escape)
		{
			windowManager.getMainWindow().close();
		}
	};
}

void Application::run()
{
    sf::Clock mainClock;

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
        mEvents.onClose,
        mEvents.onKeyPress
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