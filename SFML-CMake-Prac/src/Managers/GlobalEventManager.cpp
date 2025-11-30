#include "Managers/GlobalEventManager.hpp"
#include "Managers/ResourceManager.hpp"
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
		else if (event.scancode == sf::Keyboard::Scancode::M)
		{
			if (auto* music = appContext->m_ResourceManager->getResource<sf::Music>("MainSong"))
			{
				if (music->getStatus() == sf::Music::Status::Playing)
				{
					music->pause();
				}
				else
				{
					music->play();
				}
			}
		}
	};

	// Can add stuff to happen on window resize, etc.
	
}