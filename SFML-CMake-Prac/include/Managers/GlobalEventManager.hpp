// This handles global input events for the application
// real-time input handling (real-time polling) is managed separately

#pragma once

#include <SFML/Window/Event.hpp>

#include <functional>

struct AppContext; // foward declaration

struct ApplicationEvents
{
	std::function<void(const sf::Event::Closed&)> onClose;
    std::function<void(const sf::Event::KeyPressed&)> onGlobalKeyPress;

};

class GlobalEventManager
{
public:
    explicit GlobalEventManager(AppContext* appContext);
    GlobalEventManager(const GlobalEventManager&) = delete;
    GlobalEventManager& operator=(const GlobalEventManager&) = delete;
    ~GlobalEventManager() = default;

    ApplicationEvents& getEventHandles() noexcept { return m_Events; }
    const ApplicationEvents& getEventHandles() const noexcept { return m_Events; }

private:
    ApplicationEvents m_Events;
};