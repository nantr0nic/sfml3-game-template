#pragma once

#include <SFML/Graphics.hpp>

#include "ConfigManager.hpp"

#include <memory>
#include <string>

class WindowManager
{
public:
    WindowManager();
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    ~WindowManager();

    bool createMainWindow();
    bool createMainWindow(unsigned int width, unsigned int height, const std::string& title);
    sf::RenderWindow& getMainWindow();
    const sf::RenderWindow& getMainWindow() const;

    // Space here for making additional windows in the future
    // (e.g. a settings window or something)

private:
    std::unique_ptr<sf::RenderWindow> m_MainWindow;
    std::unique_ptr<ConfigManager> m_WindowConfig;
};