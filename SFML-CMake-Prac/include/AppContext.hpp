#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include "Managers/ConfigManager.hpp"
#include "Managers/WindowManager.hpp"
#include "Managers/GlobalEventManager.hpp"
#include "Managers/ResourceManager.hpp"

#include <memory>

class StateManager;

struct AppContext
{
    AppContext() {
        // make ConfigManager and load config files first
        m_ConfigManager = std::make_unique<ConfigManager>();
        m_ConfigManager->loadConfig("window", "config/WindowConfig.toml");

        // then initialize the stuff that uses those configs
        m_WindowManager = std::make_unique<WindowManager>(*m_ConfigManager);
        m_ResourceManager = std::make_unique<ResourceManager>();
        m_GlobalEventManager = std::make_unique<GlobalEventManager>(this);
        m_MainClock = std::make_unique<sf::Clock>();
        m_Registry = std::make_unique<entt::registry>();
    }

    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    ~AppContext() noexcept = default;

    // Resources
    std::unique_ptr<ConfigManager> m_ConfigManager{ nullptr };
    std::unique_ptr<WindowManager> m_WindowManager{ nullptr };
    std::unique_ptr<GlobalEventManager> m_GlobalEventManager{ nullptr };
    std::unique_ptr<ResourceManager> m_ResourceManager{ nullptr };
    std::unique_ptr<sf::Clock> m_MainClock{ nullptr };
    std::unique_ptr<entt::registry> m_Registry{ nullptr };

    // Pointers to Application-level objects
    sf::RenderWindow* m_MainWindow{ nullptr };
    StateManager* m_StateManager{ nullptr };
};