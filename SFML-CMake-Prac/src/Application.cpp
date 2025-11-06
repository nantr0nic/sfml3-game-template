#include "Application.hpp"

Application::Application()
    : m_WindowManager()
    , m_InputSystem(m_WindowManager)
{
    m_WindowManager.createMainWindow(1024, 768, "SFML CMake Practice");
    m_WindowManager.getMainWindow().setFramerateLimit(60);

    float mainWinCenterX = (m_WindowManager.getMainWindowSize().x) / 2.0f;
    float mainWinCenterY = (m_WindowManager.getMainWindowSize().y) / 2.0f;

    m_Player = std::make_unique<Player>(mainWinCenterX, mainWinCenterY);
}

Application::~Application()
{
    // WindowManager destructor will handle window cleanup
}

void Application::run()
{
    while (m_WindowManager.getMainWindow().isOpen())
    {
        // Calculate delta time (time since last frame)
        sf::Time deltaTime = m_MainClock.restart();

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
    m_WindowManager.getMainWindow().handleEvents(
        m_InputSystem.getEventHandles().onClose,
        m_InputSystem.getEventHandles().onKeyPress
    );

    // Handle player movement input
    m_Player->handleInput();
}

void Application::update(sf::Time deltaTime)
{
	m_Player->update(deltaTime);
}

void Application::render()
{
    // can use reference to the pointer to avoid repetition
    //$ can we do this in header or constructor somehow?
    sf::RenderWindow& mainWindow = m_WindowManager.getMainWindow();

    mainWindow.clear(sf::Color::Black);

    // Draw game objects here
    m_Player->render(mainWindow);

    mainWindow.display();
}