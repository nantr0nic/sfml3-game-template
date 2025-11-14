#include "Application.hpp"

Application::Application()
    : m_AppContext()
    , m_StateManager(&m_AppContext)
{
    // Initialize Application Window and data
    initMainWindow();
    initResources();

    // Set the StateManager in AppContext to Application's StateManager
    m_AppContext.m_StateManager = &m_StateManager;

    // Push the initial application state
    auto menuState = std::make_unique<MenuState>(&m_AppContext);
    m_StateManager.pushState(std::move(menuState));
}

Application::~Application()
{
    // WindowManager destructor will handle window cleanup
}

void Application::initMainWindow()
{
    m_AppContext.m_WindowManager->createMainWindow();
    m_AppContext.m_MainWindow = &m_AppContext.m_WindowManager->getMainWindow();
    m_AppContext.m_MainWindow->setFramerateLimit(60);
}

void Application::initResources()
{
    try {
        m_AppContext.m_ResourceManager->loadResource<sf::Font>(
            "MainFont", "resources/fonts/CaesarDressing-Regular.ttf"
        );
        m_AppContext.m_ResourceManager->loadResource<sf::Music>(
            "MainSong", "resources/music/VideoGameAm.ogg"
        );
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading resources: " << e.what() << std::endl;
    }
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
    auto& globalEvents = m_AppContext.m_GlobalEventManager->getEventHandles();
    auto& stateEvents = m_StateManager.getCurrentState()->getEventHandlers();

    m_AppContext.m_MainWindow->handleEvents(
        globalEvents.onClose,
        stateEvents.onKeyPress,
        stateEvents.onMouseButtonPress
    );
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