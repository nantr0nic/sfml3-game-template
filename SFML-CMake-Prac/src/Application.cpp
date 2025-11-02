#include "Application.hpp"

Application::Application()
    : windowManager()
    , inputSystem(windowManager)
{
    windowManager.createMainWindow(1024, 768, "SFML CMake Practice");
    windowManager.getMainWindow().setFramerateLimit(60);

    float mainWinCenterX = (windowManager.getMainWindowSize().x) / 2.0f;
    float mainWinCenterY = (windowManager.getMainWindowSize().y) / 2.0f;

    mPlayer = std::make_unique<Player>(mainWinCenterX, mainWinCenterY);
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

    // Handle player movement input
    mPlayer->handleInput();
}

void Application::update(sf::Time deltaTime)
{
	mPlayer->update(deltaTime);
}

void Application::render()
{
    // can use reference to the pointer to avoid repetition
    //$ can we do this in header or constructor somehow?
    sf::RenderWindow& mainWindow = windowManager.getMainWindow();

    mainWindow.clear(sf::Color::Black);

    // Draw game objects here
    mPlayer->render(mainWindow);

    mainWindow.display();
}