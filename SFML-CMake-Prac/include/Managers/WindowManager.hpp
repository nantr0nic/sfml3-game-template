#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>

class ConfigManager; // forward-declaration

class WindowManager
{
public:
    WindowManager(ConfigManager& configManager);
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    ~WindowManager();

    // Returns true on success, false if config is invalid or MainWindow already exists
    // May throw toml::type_error if config values have wrong types
    bool createMainWindow(/* uses config file data */);

    // Returns true on success, false if MainWindow already exists
    bool createMainWindow(unsigned int width, unsigned int height, const std::string& title);
    
    sf::RenderWindow& getMainWindow();
    const sf::RenderWindow& getMainWindow() const;

    // Space here for making additional windows in the future
    // (e.g. a settings window or something)

private:
    std::unique_ptr<sf::RenderWindow> m_MainWindow { nullptr };
    ConfigManager& m_ConfigManager;

};