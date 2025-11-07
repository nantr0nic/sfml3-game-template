#include "Application.hpp"

Application::Application()
    : m_AppContext()
    , m_StateManager(&m_AppContext)
{
    m_AppContext.m_WindowManager->createMainWindow(1024, 768, "SFML CMake Practice");
    m_AppContext.m_MainWindow = &m_AppContext.m_WindowManager->getMainWindow();
    m_AppContext.m_StateManager = &m_StateManager;

    m_AppContext.m_MainWindow->setFramerateLimit(60);

    float mainWinCenterX = (m_AppContext.m_MainWindow->getSize().x) / 2.0f;
    float mainWinCenterY = (m_AppContext.m_MainWindow->getSize().y) / 2.0f;

    m_AppContext.m_Player = std::make_unique<Player>(mainWinCenterX, mainWinCenterY);

    m_AppContext.m_ResourceManager->loadResource<sf::Font>(
        "MainFont", "resources/fonts/CaesarDressing-Regular.ttf"
    );

    auto menuState = std::make_unique<MenuState>(&m_AppContext);
    m_StateManager.pushState(std::move(menuState));
}

Application::~Application()
{
    // WindowManager destructor will handle window cleanup
}

void Application::run()
{
    sf::Clock mainClock = *m_AppContext.m_MainClock;

    while (m_AppContext.m_MainWindow->isOpen())
    {
        sf::Time deltaTime = mainClock.restart();
        processEvents();
        update(deltaTime);
        render();
    }
}

void Application::processEvents()
{
    m_AppContext.m_MainWindow->handleEvents(
        m_AppContext.m_InputManager->getEventHandles().onClose,
        m_AppContext.m_InputManager->getEventHandles().onKeyPress
    );

    m_StateManager.handleEvent();
}

void Application::update(sf::Time deltaTime)
{
	m_StateManager.update(deltaTime);
}

void Application::render()
{
    m_AppContext.m_MainWindow->clear(sf::Color::Black);

    m_StateManager.render();

    m_AppContext.m_MainWindow->display();
}