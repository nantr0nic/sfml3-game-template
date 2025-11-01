#include "InputSystem.hpp"
#include "WindowManager.hpp"

InputSystem::InputSystem(WindowManager& windowManager)
{
    initEventHandlers(windowManager);
}

void InputSystem::initEventHandlers(WindowManager& windowManager)
{
    mEvents.onClose = [&windowManager](const sf::Event::Closed&)
	{
		windowManager.getMainWindow().close();
	};

	mEvents.onKeyPress = [&windowManager](const sf::Event::KeyPressed& keyPressed)
	{
		if (keyPressed.scancode == sf::Keyboard::Scancode::Escape)
		{
			windowManager.getMainWindow().close();
		}
	};
}