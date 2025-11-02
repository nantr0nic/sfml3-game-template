#pragma once

#include "SFML/Graphics.hpp"

#include <memory>
#include <string>

class WindowManager
{
public:
    WindowManager();
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    ~WindowManager();

    bool createMainWindow(unsigned int width, unsigned int height, const std::string& title);
    sf::RenderWindow& getMainWindow();
    const sf::RenderWindow& getMainWindow() const;

    sf::Vector2u getMainWindowSize() const
    {
        return getMainWindow().getSize();
    }

    // Space here for making additional windows in the future
    // (e.g. a settings window or something)

private:
    std::unique_ptr<sf::RenderWindow> mainWindow;
};