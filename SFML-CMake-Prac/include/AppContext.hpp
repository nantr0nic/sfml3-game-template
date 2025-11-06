#pragma once

#include <SFML/Graphics.hpp>

#include "WindowManager.hpp"
#include "InputSystem.hpp"
#include "Player.hpp"

#include <memory>

class StateManager;

struct AppContext
{
    AppContext() {
        m_WindowManager = std::make_unique<WindowManager>();
        m_MainClock = std::make_unique<sf::Clock>();
        m_InputManager = std::make_unique<InputSystem>(*m_WindowManager);
    }

    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    ~AppContext() {
        // explicit cleanup to practice good habits :)
        // (though unique_ptr will handle this automatically)
        m_WindowManager = nullptr;
        m_InputManager = nullptr;
        m_MainClock = nullptr;
        m_Player = nullptr;
    }

    // Resources
    std::unique_ptr<WindowManager> m_WindowManager{ nullptr };
    std::unique_ptr<InputSystem> m_InputManager{ nullptr };
    std::unique_ptr<sf::Clock> m_MainClock{ nullptr };
    std::unique_ptr<Player> m_Player{ nullptr };

    // Pointers to Application-level objects
    sf::RenderWindow* m_MainWindow{ nullptr };
    StateManager* m_StateManager{ nullptr };
};