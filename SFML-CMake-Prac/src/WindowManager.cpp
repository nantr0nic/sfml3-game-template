#include "WindowManager.hpp"
#include <memory>

WindowManager::WindowManager()
    : mainWindow(nullptr)
{
}

bool WindowManager::createMainWindow(unsigned int width, unsigned int height, const std::string& title)
{
    // Check if main window already exists, return false if it does
    if (mainWindow)
    {
        return false;
    }
    else 
    {
        mainWindow = std::make_unique<sf::RenderWindow>(
            // Use .ini settings later for dimensions, etc.
            sf::VideoMode({ 1024, 768 }),
            "SFML CMake Practice",
            // Always use default style + windowed for now
            sf::Style::Default,
            sf::State::Windowed
        );
    }

    // will return true if window was created successfully
    return mainWindow->isOpen();
}

sf::RenderWindow& WindowManager::getMainWindow()
{
    // Check if mainWindow is valid
    if (!mainWindow)
    {
        throw std::runtime_error("Main window has not been created yet.");
    }
    return *mainWindow;
}

const sf::RenderWindow& WindowManager::getMainWindow() const
{
    // Check if mainWindow is valid
    if (!mainWindow)
    {
        throw std::runtime_error("Main window has not been created yet.");
    }
    return *mainWindow;
}

WindowManager::~WindowManager()
{
    if (mainWindow)
    {
        mainWindow->close();
        mainWindow = nullptr;
    }
}