#include "AppContext.hpp"
#include "State.hpp"
#include "Managers/StateManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/Systems.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"
#include "AssetKeys.hpp"

#include <memory>
#include <format>

//$ ----- MenuState Implementation ----- //
MenuState::MenuState(AppContext* appContext)
    : State(appContext)
{
    sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Play button entity
    sf::Font* font = m_AppContext->m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (font)
    {
        EntityFactory::createButton(
            *m_AppContext,
            *font,
            "Play",
            center,
            // lambda for when button is clicked
            [this]() {
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext->m_StateManager->replaceState(std::move(playState));
            }
        );
    }
    else 
    {
        logger::Error("Couldn't load font.");
    }

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

    logger::Info("MenuState initialized.");
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
    sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);
    EntityFactory::createPlayer(*m_AppContext, { center.x, center.y });

    m_MainMusic = m_AppContext->m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);

    // Start music
    if (m_MainMusic)
    {
        m_MainMusic->setLooping(true);
        m_MainMusic->play();
    }
    else 
    {
        logger::Error("MainSong not found, not playing music.");
    }
    
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
        else if (event.scancode == sf::Keyboard::Scancode::F12)
        {
            m_ShowDebug = !m_ShowDebug;
            logger::Warn(std::format("Debug mode toggled: {}", m_ShowDebug ? "On" : "Off"));
        }
    };

    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        // empty on purpose, it was crashing otherwise
    };

    logger::Info("PlayState initialized.");
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
    CoreSystems::handlePlayerInput(m_AppContext);
    CoreSystems::facingSystem(*m_AppContext->m_Registry);
    CoreSystems::animationSystem(*m_AppContext->m_Registry, deltaTime);
    CoreSystems::movementSystem(*m_AppContext->m_Registry, deltaTime, *m_AppContext->m_MainWindow);
}

void PlayState::render()
{
    CoreSystems::renderSystem(
        *m_AppContext->m_Registry, 
        *m_AppContext->m_MainWindow,
        m_ShowDebug
    );
}


//$ ----- PauseState Implementation -----
PauseState::PauseState(AppContext* appContext)
    : State(appContext)
{
    sf::Font* font = m_AppContext->m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);

    if (!font)
    {
        logger::Error("MainFont not found! Can't make pause text.");
    }
    else 
    {
        m_PauseText.emplace(*font, "Paused", 100);
        m_PauseText->setFillColor(sf::Color::Red);

        utils::centerOrigin(*m_PauseText);

        sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
        sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);
        m_PauseText->setPosition(center);
    }

    // Handle music stuff
    auto* music = m_AppContext->m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool wasMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (wasMusicPlaying)
    {
        music->pause();
    }

    // Lambda to handle pause
    m_StateEvents.onKeyPress = [this, music, wasMusicPlaying](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext->m_MainWindow->close();
        }
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            if (wasMusicPlaying && music)
            {
                music->play();
            }
            m_AppContext->m_StateManager->popState();
            logger::Info("Game unpaused.");
        }
    };

    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        // empty on purpose
    };

    logger::Info("Game paused.");
}


void PauseState::update(sf::Time deltaTime)
{
    // Update pause logic here
}

void PauseState::render()
{
    if (m_PauseText)
    {
        m_AppContext->m_MainWindow->draw(*m_PauseText);
    }
}