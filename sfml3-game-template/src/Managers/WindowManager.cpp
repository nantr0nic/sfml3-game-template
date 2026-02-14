#include "Managers/WindowManager.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"
#include "AssetKeys.hpp"

#include <cstdlib>
#include <memory>
#include <string>

/**
 * @brief Create a WindowManager bound to a configuration source.
 *
 * Initializes the manager with no active main window and retains a reference
 * to the provided ConfigManager for later window creation and configuration lookups.
 *
 * @param configManager Reference to the ConfigManager used for retrieving window settings.
 */
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

/**
 * @brief Creates the application's main SFML render window using configuration values.
 *
 * Reads width, height, and title from the configuration (falling back to 800x600 and "Error parsing title" if missing) and constructs the main window.
 *
 * @return `true` if the created main window is open, `false` if creation failed or a main window already exists (an error is logged when a window already exists).
 */
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

/**
 * @brief Creates the main application window with the specified size and title.
 *
 * Constructs and stores the primary sf::RenderWindow using the provided dimensions and title.
 *
 * @param width Window width in pixels.
 * @param height Window height in pixels.
 * @param title Window title text.
 * @return true if the window was created and is open, false if a main window already exists or creation failed.
 */
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
 * @brief Accesses the main SFML render window.
 *
 * If the main window has not been created, the function terminates the process using std::abort().
 *
 * @return sf::RenderWindow& Reference to the main render window.
 */
sf::RenderWindow& WindowManager::getMainWindow()
{
    // Check if mainWindow is valid
    if (!m_MainWindow)
    {
        logger::Error("FATAL: getMainWindow failed! m_MainWindow is null! Terminating...");
        std::abort();
    }
    return *m_MainWindow;
}

/**
 * @brief Returns a const reference to the application's main SFML render window.
 *
 * If the main window has not been created (internal pointer is null), logs a fatal error and aborts the process.
 *
 * @return const sf::RenderWindow& Reference to the main render window.
 */
const sf::RenderWindow& WindowManager::getMainWindow() const
{
    // Check if mainWindow is valid
    if (!m_MainWindow)
    {
        logger::Error("FATAL: getMainWindow failed! m_MainWindow is null! Terminating...");
        std::abort();
    }
    return *m_MainWindow;
}