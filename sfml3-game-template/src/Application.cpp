#include "Application.hpp"
#include "Utilities/Logger.hpp"

#include <format>
#include <memory>

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

    // Debug
    logger::Info("Application initialized.");
}

Application::~Application()
{
    // WindowManager destructor will handle window cleanup
}

void Application::initMainWindow()
{
    if (m_AppContext.m_WindowManager->createMainWindow())
    {
        m_AppContext.m_MainWindow = &m_AppContext.m_WindowManager->getMainWindow();
        m_AppContext.m_MainWindow->setFramerateLimit(60);

        logger::Info(std::format("Main window created."));
    }
    else 
    {
        logger::Error("Error creating main window.");
        m_AppContext.m_MainWindow = nullptr;
    }
}

void Application::initResources()
{
    m_AppContext.m_ResourceManager->loadAssetsFromManifest("config/AssetsManifest.toml");
}

void Application::run()
{
    if (!m_AppContext.m_MainWindow)
    {
        logger::Error("No main window; aborting run().");
        return;
    }
    
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

    auto onKeyPressMerged = [&](const sf::Event::KeyPressed& event)
    {
        // run global logic first
        if (globalEvents.onGlobalKeyPress)
        {
            globalEvents.onGlobalKeyPress(event);
        }
        // then run state-specific
        if (stateEvents.onKeyPress)
        {
            stateEvents.onKeyPress(event);
        }
    };

    m_AppContext.m_MainWindow->handleEvents(
        globalEvents.onClose,
        onKeyPressMerged,
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