#include "State.hpp"
#include "StateManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/Systems.hpp"
#include "Utils.hpp"

//$ ----- MenuState Implementation ----- //
MenuState::MenuState(AppContext* appContext)
    : State(appContext)
{
    sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Play button entity
    EntityFactory::createButton(
        *m_AppContext->m_Registry,
        m_AppContext->m_ResourceManager->getResource<sf::Font>("MainFont"),
        "Play",
        center,
        // lambda for when button is clicked
        [this]() {
            auto playState = std::make_unique<PlayState>(m_AppContext);
            m_AppContext->m_StateManager->replaceState(std::move(playState));
        }
    );


    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        // using ECS system here instead of previous SFML event response

        UISystems::uiClickSystem(*m_AppContext->m_Registry, event);

    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext->m_MainWindow->close();
        }
    };
}

MenuState::~MenuState()
{
    // clean up EnTT entities on leaving MenuState
    auto& registry = *m_AppContext->m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

void MenuState::update(sf::Time deltaTime)
{
    // Call the UI hover system here
    UISystems::uiHoverSystem(*m_AppContext->m_Registry, *m_AppContext->m_MainWindow);
}

void MenuState::render()
{
    // Render menu here
    UISystems::uiRenderSystem(*m_AppContext->m_Registry, *m_AppContext->m_MainWindow);
}


//$ ----- PlayState Implementation ----- //
PlayState::PlayState(AppContext* appContext)
    : State(appContext)
{
    // We create the player entity here
    float mainWinCenterX = (m_AppContext->m_MainWindow->getSize().x) / 2.0f;
    float mainWinCenterY = (m_AppContext->m_MainWindow->getSize().y) / 2.0f;

    EntityFactory::createPlayer(*m_AppContext->m_Registry, { mainWinCenterX, mainWinCenterY });

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        // "Global" Escape key
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext->m_MainWindow->close();
        }
        // State-specific Pause key
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            auto pauseState = std::make_unique<PauseState>(m_AppContext);
            m_AppContext->m_StateManager->pushState(std::move(pauseState));
        }
    };

    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        // empty on purpose
    };
}

PlayState::~PlayState()
{
    // Clean up all player entities
    auto& registry = *m_AppContext->m_Registry;
    auto view = registry.view<PlayerTag>();
    registry.destroy(view.begin(), view.end());
    
    // Here you would also clean up enemies, bullets, etc.
    // (e.g., registry.clear<EnemyTag, BulletTag>();)
}

void PlayState::update(sf::Time deltaTime)
{
    // Call game logic systems
    CoreSystems::handlePlayerInput(*m_AppContext->m_Registry);
    CoreSystems::movementSystem(*m_AppContext->m_Registry, deltaTime);
}

void PlayState::render()
{
    CoreSystems::renderSystem(*m_AppContext->m_Registry, *m_AppContext->m_MainWindow);
}


//$ ----- PauseState Implementation -----
PauseState::PauseState(AppContext* appContext)
    : State(appContext)
    , m_PauseText(m_AppContext->m_ResourceManager->getResource<sf::Font>("MainFont"), "Paused", 100)
{
    sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Setup Pause Text
    m_PauseText.setFillColor(sf::Color::Red);
    Utils::centerOrigin(m_PauseText);
    m_PauseText.setPosition(center);

    // Lambda to handle pause
    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::P)
        {
            m_AppContext->m_StateManager->popState();
        }
    };

    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        // empty on purpose
    };
}


void PauseState::update(sf::Time deltaTime)
{
    // Update pause logic here
}

void PauseState::render()
{
    m_AppContext->m_MainWindow->draw(m_PauseText);
}