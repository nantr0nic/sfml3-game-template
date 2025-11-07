#pragma once

#include "WindowManager.hpp"
#include <functional>

struct applicationEvents
{
	std::function<void(const sf::Event::Closed&)> onClose;
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress;
};

class InputManager
{
public:
    InputManager(WindowManager& windowManager);
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    ~InputManager() = default;

    applicationEvents& getEventHandles() { return m_Events; }
    const applicationEvents& getEventHandles() const { return m_Events; }

private:
    void initEventHandlers(WindowManager& windowManager);
    applicationEvents m_Events;
};