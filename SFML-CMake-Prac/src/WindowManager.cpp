#include "WindowManager.hpp"
#include <memory>

WindowManager::WindowManager()
    : m_MainWindow(nullptr)
{
}

bool WindowManager::createMainWindow(unsigned int width, unsigned int height, const std::string& title)
{
    // Check if main window already exists, return false if it does
    if (m_MainWindow)
    {
        return false;
    }
    else 
    {
        m_MainWindow = std::make_unique<sf::RenderWindow>(
            // Use .ini settings later for dimensions, etc.
            sf::VideoMode({ 1024, 768 }),
            "SFML CMake Practice",
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
        throw std::runtime_error("Main window has not been created yet.");
    }
    return *m_MainWindow;
}

const sf::RenderWindow& WindowManager::getMainWindow() const
{
    // Check if mainWindow is valid
    if (!m_MainWindow)
    {
        throw std::runtime_error("Main window has not been created yet.");
    }
    return *m_MainWindow;
}

WindowManager::~WindowManager()
{
    if (m_MainWindow)
    {
        m_MainWindow->close();
        m_MainWindow = nullptr;
    }
}