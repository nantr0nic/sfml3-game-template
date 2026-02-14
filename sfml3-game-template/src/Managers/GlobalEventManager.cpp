#include "Managers/GlobalEventManager.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "Utilities/Logger.hpp"
#include "AppContext.hpp"

/**
 * @brief Register global SFML event handlers for application-level actions.
 *
 * Sets up handlers that respond to window close events and the Escape key by closing
 * the application's main window. The constructor stores these callbacks in the
 * internal event container so they are invoked by the application's event loop.
 *
 * @param appContext Pointer to the application context providing access to the main window;
 *                   must be non-null and have a valid `m_MainWindow`.
 */
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