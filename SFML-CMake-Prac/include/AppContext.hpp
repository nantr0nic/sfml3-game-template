#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include "WindowManager.hpp"
#include "GlobalEventManager.hpp"
#include "ResourceManager.hpp"

#include <memory>

class StateManager;

struct AppContext
{
    AppContext() noexcept {
        m_WindowManager = std::make_unique<WindowManager>();
        m_ResourceManager = std::make_unique<ResourceManager>();
        m_GlobalEventManager = std::make_unique<GlobalEventManager>(*m_WindowManager);
        m_MainClock = std::make_unique<sf::Clock>();
        m_Registry = std::make_unique<entt::registry>();
    }

    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    ~AppContext() noexcept = default;

    // Resources
    std::unique_ptr<WindowManager> m_WindowManager{ nullptr };
    std::unique_ptr<GlobalEventManager> m_GlobalEventManager{ nullptr };
    std::unique_ptr<ResourceManager> m_ResourceManager{ nullptr };
    std::unique_ptr<sf::Clock> m_MainClock{ nullptr };
    std::unique_ptr<entt::registry> m_Registry{ nullptr };

    // Pointers to Application-level objects
    sf::RenderWindow* m_MainWindow{ nullptr };
    StateManager* m_StateManager{ nullptr };
};