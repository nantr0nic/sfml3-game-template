#include "Managers/WindowManager.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"
#include "AssetKeys.hpp"

#include <memory>
#include <string>

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
            Assets::Configs::Window, "mainWindow", "X").value_or(800u);
        unsigned int height = m_ConfigManager.getConfigValue<unsigned int>(
            Assets::Configs::Window, "mainWindow", "Y").value_or(600u);
        std::string title = m_ConfigManager.getConfigValue<std::string>(
            Assets::Configs::Window, "mainWindow", "Title").value_or("Error parsing title");

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

/**
 * @brief Accesses the primary application RenderWindow.
 *
 * If the main window has not been created, logs a fatal error and terminates the program.
 *
 * @return sf::RenderWindow& Reference to the main RenderWindow.
 */
sf::RenderWindow& WindowManager::getMainWindow()
{
    if (!m_MainWindow)
    {
        logger::Fatal("getMainWindow failed! m_MainWindow is null! Terminating...");
    }
    return *m_MainWindow;
}

/**
 * @brief Returns a const reference to the application's main window.
 *
 * If the main window has not been created, logs a fatal error and terminates the process.
 *
 * @return const sf::RenderWindow& Reference to the main render window.
 */
const sf::RenderWindow& WindowManager::getMainWindow() const
{
    if (!m_MainWindow)
    {
        logger::Fatal("getMainWindow failed! m_MainWindow is null! Terminating...");
    }
    return *m_MainWindow;
}
