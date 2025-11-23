// This handles global input events for the application
// real-time input handling (real-time polling) is managed separately

#pragma once

#include "Managers/WindowManager.hpp"
#include <functional>

struct ApplicationEvents
{
	std::function<void(const sf::Event::Closed&)> onClose;
};

class GlobalEventManager
{
public:
    GlobalEventManager(WindowManager& windowManager);
    GlobalEventManager(const GlobalEventManager&) = delete;
    GlobalEventManager& operator=(const GlobalEventManager&) = delete;
    ~GlobalEventManager() = default;

    ApplicationEvents& getEventHandles() noexcept { return m_Events; }
    const ApplicationEvents& getEventHandles() const noexcept { return m_Events; }

private:
    ApplicationEvents m_Events;
};