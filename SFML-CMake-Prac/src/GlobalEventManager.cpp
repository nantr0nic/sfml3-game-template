#include "GlobalEventManager.hpp"
#include "WindowManager.hpp"

GlobalEventManager::GlobalEventManager(WindowManager& windowManager)
{
    initEventHandlers(windowManager);
}

void GlobalEventManager::initEventHandlers(WindowManager& windowManager)
{
    m_Events.onClose = [&windowManager](const sf::Event::Closed&)
	{
		windowManager.getMainWindow().close();
	};
}