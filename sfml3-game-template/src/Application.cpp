#include "Application.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include "Utilities/Logger.hpp"
#include "Utilities/Utils.hpp"

#include <format>
#include <memory>

Application::Application()
    : m_AppContext()
    , m_StateManager(m_AppContext)
{
    // Initialize Application Window and data
    initMainWindow();
    initResources();

    // Set the StateManager in AppContext to Application's StateManager
    m_AppContext.m_StateManager = &m_StateManager;

    // Push the initial application state
    auto menuState = std::make_unique<MenuState>(m_AppContext);
    m_StateManager.pushState(std::move(menuState));

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

/**
 * @brief Runs the application's main loop until the main window is closed.
 *
 * @details Iterates while the main window is open: computes the per-frame elapsed time
 * from the application's main clock, processes pending state changes, handles input
 * and window events, updates application state using the elapsed time, and renders
 * the current frame. If no main window is available, the method logs an error and returns without running.
 */
void Application::run()
{
    if (!m_AppContext.m_MainWindow)
    {
        logger::Error("No main window; aborting run().");
        return;
    }

    auto& mainClock = *m_AppContext.m_MainClock;

    while (m_AppContext.m_MainWindow->isOpen())
    {
        sf::Time deltaTime = mainClock.restart();
        m_StateManager.processPending();
        processEvents();
        update(deltaTime);
        render();
    }
}

/**
 * @brief Polls and dispatches window events to global and state-specific handlers.
 *
 * Retrieves global and current-state event handlers, merges global and state key-press handling
 * so global handlers run first, and forwards mouse and close events to the appropriate handlers.
 * On window resize, computes a boxed view that matches the configured target dimensions and
 * applies it to the main window.
 *
 * If no current state is available, the function logs an error and returns without processing events.
 */
void Application::processEvents()
{
    auto& globalEvents = m_AppContext.m_GlobalEventManager->getEventHandles();
    auto* currentState = m_StateManager.getCurrentState();
    if (!currentState)
    {
        logger::Error("No current state; aborting processEvents().");
        return;
    }
    auto& stateEvents = currentState->getEventHandlers();

    auto onKeyPressMerged = [&](const sf::Event::KeyPressed& event) {
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
    
    auto onResized = [&](const sf::Event::Resized& event) {
        sf::Vector2f targetSize = {m_AppContext.m_AppSettings.targetWidth, 
                                    m_AppContext.m_AppSettings.targetHeight};

        sf::View view(sf::FloatRect({0.0f, 0.0f}, targetSize));
        utils::boxView(view, event.size.x, event.size.y);
        m_AppContext.m_MainWindow->setView(view);
    };

    m_AppContext.m_MainWindow->handleEvents(
        globalEvents.onClose,
        onKeyPressMerged,
        stateEvents.onMouseButtonPress,
        onResized
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