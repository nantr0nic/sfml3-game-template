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

/**
 * @brief Constructs the Application and initializes core systems.
 *
 * Initializes the application context and state manager, creates the main window,
 * loads resources, registers the state manager in the application context, and
 * pushes the initial menu state onto the state stack.
 */
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

/**
 * @brief Creates and configures the application's main window and stores its pointer in the application context.
 *
 * Attempts to create the main window via the WindowManager. On success stores a pointer to the created window
 * in AppContext, sets the frame rate limit to 60, and logs an informational message. On failure sets the
 * AppContext main window pointer to `nullptr` and logs an error.
 */
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
 * @brief Runs the application's main loop until the main window closes.
 *
 * Starts the frame loop that, each iteration, obtains frame delta time from the application's main clock,
 * processes pending state changes, handles input and window events, updates application state, and renders a frame.
 * If no main window is available when called, the function logs an error and returns without entering the loop.
 */
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
        m_StateManager.processPending();
        processEvents();
        update(deltaTime);
        render();
    }
}

/**
 * @brief Process pending window and input events, dispatching them to global and state handlers.
 *
 * Dispatches close, key press, mouse button press, and resize events for the main window.
 * Key press events invoke the global key handler first (if present) and then the current
 * state's key handler. Resize events update the main window's view to match the application's
 * target size while preserving aspect/letterboxing via utils::boxView.
 */
void Application::processEvents()
{
    auto& globalEvents = m_AppContext.m_GlobalEventManager->getEventHandles();
    auto& stateEvents = m_StateManager.getCurrentState()->getEventHandlers();

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

/**
 * @brief Renders the current frame to the main window.
 *
 * Clears the main window to black, delegates drawing of the active state to the
 * StateManager, and presents the rendered frame to the display.
 */
void Application::render()
{
    m_AppContext.m_MainWindow->clear(sf::Color::Black);

    m_StateManager.render();

    m_AppContext.m_MainWindow->display();
}