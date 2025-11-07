#include "State.hpp"
#include "StateManager.hpp"
#include <iostream>

//$ ----- MenuState Implementation -----
MenuState::MenuState(AppContext* appContext)
    : State(appContext)
    , m_PlayText(m_AppContext->m_ResourceManager->getResource<sf::Font>("MainFont"), "Play", 50)
{
    sf::Vector2u windowSize = m_AppContext->m_MainWindow->getSize();
    sf::Vector2f center(windowSize.x / 2.0f, windowSize.y / 2.0f);

    // Setup Play Button
    m_PlayButton.setSize({200.f, 100.f});
    m_PlayButton.setFillColor(sf::Color::Blue);
    m_PlayButton.setOrigin(m_PlayButton.getSize() / 2.0f);
    m_PlayButton.setPosition(center);

    // Setup Play Text
    m_PlayText.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = m_PlayText.getLocalBounds();
    m_PlayText.setOrigin(textBounds.getCenter());
    m_PlayText.setPosition(center);

    // Lambdas to handle input
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event)
    {
        if (event.button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePos = static_cast<sf::Vector2f>(event.position);
            if (m_PlayButton.getGlobalBounds().contains(mousePos))
            {
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext->m_StateManager->pushState(std::move(playState));
            }
        }
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext->m_MainWindow->close();
        }
    };
}

void MenuState::handleEvent()
{
    // Handle menu-specific events here
}

void MenuState::update(sf::Time deltaTime)
{
    // Update menu logic here
}

void MenuState::render()
{
    // Render menu here
    m_AppContext->m_MainWindow->draw(m_PlayButton);
    m_AppContext->m_MainWindow->draw(m_PlayText);
}


//$ ----- PlayState Implementation -----
PlayState::PlayState(AppContext* appContext)
    : State(appContext)
{
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
            //auto playState = std::make_unique<PlayState>(m_AppContext);
            // we'll make it go back to MenuState for now
            m_AppContext->m_StateManager->popState();
        }
    };
}

void PlayState::handleEvent()
{
    // handle playstate events here
}

void PlayState::update(sf::Time deltaTime)
{
    m_AppContext->m_Player->handleInput();
    m_AppContext->m_Player->update(deltaTime);
}

void PlayState::render()
{
    m_AppContext->m_Player->render(*m_AppContext->m_MainWindow);
}


//$ ----- PauseState Implementation -----
PauseState::PauseState(AppContext* appContext)
    : State(appContext)
{
}

void PauseState::handleEvent()
{
    // Handle pause-specific events here
}

void PauseState::update(sf::Time deltaTime)
{
    // Update pause logic here
}

void PauseState::render()
{
    // Render pause screen here
}