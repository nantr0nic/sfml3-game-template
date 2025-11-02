#pragma once

#include "WindowManager.hpp"
#include <functional>

struct applicationEvents
{
	std::function<void(const sf::Event::Closed&)> onClose;
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress;
};

class InputSystem
{
public:
    InputSystem(WindowManager& windowManager);
    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;
    ~InputSystem() = default;

    applicationEvents& getEventHandles() { return mEvents; }
    const applicationEvents& getEventHandles() const { return mEvents; }

private:
    void initEventHandlers(WindowManager& windowManager);
    applicationEvents mEvents;
};