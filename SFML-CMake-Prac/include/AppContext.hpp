#pragma once

#include <SFML/Graphics.hpp>

#include "WindowManager.hpp"
#include "GlobalEventManager.hpp"
#include "Player.hpp"
#include "ResourceManager.hpp"

#include <memory>

class StateManager;

struct AppContext
{
    AppContext() {
        m_WindowManager = std::make_unique<WindowManager>();
        m_ResourceManager = std::make_unique<ResourceManager>();
        m_GlobalEventManager = std::make_unique<GlobalEventManager>(*m_WindowManager);
        m_MainClock = std::make_unique<sf::Clock>();
    }

    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    ~AppContext() = default;

    // Resources
    std::unique_ptr<WindowManager> m_WindowManager{ nullptr };
    std::unique_ptr<GlobalEventManager> m_GlobalEventManager{ nullptr };
    std::unique_ptr<ResourceManager> m_ResourceManager{ nullptr };
    std::unique_ptr<sf::Clock> m_MainClock{ nullptr };

    std::unique_ptr<Player> m_Player{ nullptr };

    // Pointers to Application-level objects
    sf::RenderWindow* m_MainWindow{ nullptr };
    StateManager* m_StateManager{ nullptr };
};