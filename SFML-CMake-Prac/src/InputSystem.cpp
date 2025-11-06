#include "InputSystem.hpp"
#include "WindowManager.hpp"

InputSystem::InputSystem(WindowManager& windowManager)
{
    initEventHandlers(windowManager);
}

void InputSystem::initEventHandlers(WindowManager& windowManager)
{
    m_Events.onClose = [&windowManager](const sf::Event::Closed&)
	{
		windowManager.getMainWindow().close();
	};

	m_Events.onKeyPress = [&windowManager](const sf::Event::KeyPressed& keyPressed)
	{
		if (keyPressed.scancode == sf::Keyboard::Scancode::Escape)
		{
			windowManager.getMainWindow().close();
		}
	};
}