#pragma once

#include <SFML/Graphics.hpp>

#include "WindowManager.hpp"
#include "InputManager.hpp"
#include "Player.hpp"

#include <memory>

class StateManager;

struct AppContext
{
    AppContext() {
        m_WindowManager = std::make_unique<WindowManager>();
        m_MainClock = std::make_unique<sf::Clock>();
        m_InputManager = std::make_unique<InputManager>(*m_WindowManager);
    }

    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    ~AppContext() = default;

    // Resources
    std::unique_ptr<WindowManager> m_WindowManager{ nullptr };
    std::unique_ptr<InputManager> m_InputManager{ nullptr };
    std::unique_ptr<sf::Clock> m_MainClock{ nullptr };
    std::unique_ptr<Player> m_Player{ nullptr };

    // Pointers to Application-level objects
    sf::RenderWindow* m_MainWindow{ nullptr };
    StateManager* m_StateManager{ nullptr };
};