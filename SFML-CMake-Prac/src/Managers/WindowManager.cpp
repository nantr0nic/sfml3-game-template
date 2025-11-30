#include "Managers/WindowManager.hpp"
#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"

#include <memory>

WindowManager::WindowManager(ConfigManager& configManager)
    : m_MainWindow(nullptr)
    , m_ConfigManager(configManager)
{
}

WindowManager::~WindowManager()
{
    // unique_ptr and sf::RenderWindow handle what's needed here
    // space reserved for future stuff
}

bool WindowManager::createMainWindow()
{
    if (m_MainWindow)
    {
        logger::Error("createMainWindow Failed: Main window already exists.");
        return false;
    }
    else 
    {
        unsigned int width = m_ConfigManager.getConfigValue<unsigned int>(
            "window", "mainWindow", "X").value_or(800u);
        unsigned int height = m_ConfigManager.getConfigValue<unsigned int>(
            "window", "mainWindow", "Y").value_or(600u);
        std::string title = m_ConfigManager.getConfigValue<std::string>(
            "window", "mainWindow", "Title").value_or("Error parsing title");
        
        m_MainWindow = std::make_unique<sf::RenderWindow>(
            sf::VideoMode({ width, height }),
            title,
            // Always use default style + windowed for now
            sf::Style::Default,
            sf::State::Windowed
        );
    }

    return m_MainWindow->isOpen();
}

bool WindowManager::createMainWindow(unsigned int width, unsigned int height, const std::string& title)
{
    // Check if main window already exists, return false if it does
    if (m_MainWindow)
    {
        logger::Error("createMainWindow Failed: Main window already exists.");
        return false;
    }
    else 
    {
        m_MainWindow = std::make_unique<sf::RenderWindow>(
            // Use .ini settings later for dimensions, etc.
            sf::VideoMode({ width, height }),
            title,
            // Always use default style + windowed for now
            sf::Style::Default,
            sf::State::Windowed
        );
    }

    // will return true if window was created successfully
    return m_MainWindow->isOpen();
}

sf::RenderWindow& WindowManager::getMainWindow()
{
    // Check if mainWindow is valid
    if (!m_MainWindow)
    {
        throw std::runtime_error(
            "<WindowManager> getMainWindow() failed: Main window has not been created yet."
        );
    }
    return *m_MainWindow;
}

const sf::RenderWindow& WindowManager::getMainWindow() const
{
    // Check if mainWindow is valid
    if (!m_MainWindow)
    {
        throw std::runtime_error(
            "<WindowManager> getMainWindow() failed: Main window has not been created yet."
        );
    }
    return *m_MainWindow;
}