#include "AppContext.hpp"
#include "State.hpp"
#include "StateManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/Systems.hpp"
#include "Utils.hpp"

#include <print>
#include <iostream>
#include <memory>


//$ ----- MenuState Implementation ----- //
MenuState::MenuState(AppContext* appContext)
    : State(appContext)
{
    sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Play button entity
    sf::Font* font = m_AppContext->m_ResourceManager->getResource<sf::Font>("MainFont");
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
        std::println(std::cerr, "<MenuState> Error: Couldn't load font.");
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

    m_MainMusic = m_AppContext->m_ResourceManager->getResource<sf::Music>("MainSong");

    // Start music
    if (m_MainMusic)
    {
        m_MainMusic->setLooping(true);
        m_MainMusic->play();
    }
    else 
    {
        std::println(std::cerr, "<PlayState> Error: MainSong not found, not playing music.");
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
            if (m_MainMusic)
            {
                m_MainMusic->pause();
            }
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
    CoreSystems::handlePlayerInput(m_AppContext);
    CoreSystems::facingSystem(*m_AppContext->m_Registry);
    CoreSystems::animationSystem(*m_AppContext->m_Registry, deltaTime);
    CoreSystems::movementSystem(*m_AppContext->m_Registry, deltaTime);
}

void PlayState::render()
{
    CoreSystems::renderSystem(*m_AppContext->m_Registry, *m_AppContext->m_MainWindow);
}


//$ ----- PauseState Implementation -----
PauseState::PauseState(AppContext* appContext)
    : State(appContext)
{
    sf::Font* font = m_AppContext->m_ResourceManager->getResource<sf::Font>("MainFont");

    if (!font)
    {
        std::println(std::cerr, "<PauseState> Error: MainFont not found! Can't make pause text.");
    }
    else 
    {
        m_PauseText.emplace(*font, "Paused", 100);
        m_PauseText->setFillColor(sf::Color::Red);

        Utils::centerOrigin(*m_PauseText);

        sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
        sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);
        m_PauseText->setPosition(center);
    }

    // Lambda to handle pause
    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext->m_MainWindow->close();
        }
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            if (auto* music = m_AppContext->m_ResourceManager->getResource<sf::Music>("MainSong"))
            {
                music->play();
            }
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
    if (m_PauseText)
    {
        m_AppContext->m_MainWindow->draw(*m_PauseText);
    }
}