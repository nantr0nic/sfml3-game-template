#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <entt/entt.hpp>

#include "Managers/ConfigManager.hpp"
#include "Managers/WindowManager.hpp"
#include "Managers/GlobalEventManager.hpp"
#include "Managers/ResourceManager.hpp"
#include "AssetKeys.hpp"
#include "AppData.hpp"

#include <memory>

class StateManager;

struct AppContext
{
    AppContext() {
        // make ConfigManager and load config files first
        m_ConfigManager = std::make_unique<ConfigManager>();
        m_ConfigManager->loadConfig(Assets::Configs::Window, "config/WindowConfig.toml");

        // then initialize the stuff that uses those configs
        m_WindowManager = std::make_unique<WindowManager>(*m_ConfigManager);
        m_ResourceManager = std::make_unique<ResourceManager>();
        m_GlobalEventManager = std::make_unique<GlobalEventManager>(this);
        m_MainClock = std::make_unique<sf::Clock>();
        m_Registry = std::make_unique<entt::registry>();

        // Set target width / height
        m_AppSettings.targetWidth = m_ConfigManager->getConfigValue<float>(
                      Assets::Configs::Window, "mainWindow", "X").value_or(1280.0f);
        m_AppSettings.targetHeight = m_ConfigManager->getConfigValue<float>(
                      Assets::Configs::Window, "mainWindow", "Y").value_or(720.0f);
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

    // AppData members
    AppSettings m_AppSettings;
    AppData m_AppData;

    // Pointers to Application-level objects
    sf::RenderWindow* m_MainWindow{ nullptr };
    StateManager* m_StateManager{ nullptr };
};
