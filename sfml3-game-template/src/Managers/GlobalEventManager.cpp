#include "Managers/GlobalEventManager.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "Utilities/Logger.hpp"
#include "AppContext.hpp"

GlobalEventManager::GlobalEventManager(AppContext* appContext)
{
    m_Events.onClose = [appContext](const sf::Event::Closed&)
	{
		appContext->m_MainWindow->close();
	};

	m_Events.onGlobalKeyPress = [appContext](const sf::Event::KeyPressed& event)
	{
		if (event.scancode == sf::Keyboard::Scancode::Escape)
		{
			// We will want to remove this if we want escape to exit an inventory window etc.
			logger::Info("Escape key pressed! Exiting.");
			appContext->m_MainWindow->close();
		}
	};

	// Can add stuff to happen on window resize, etc.
	
}