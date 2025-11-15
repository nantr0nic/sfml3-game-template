#include "GlobalEventManager.hpp"
#include "WindowManager.hpp"

GlobalEventManager::GlobalEventManager(WindowManager& windowManager) noexcept
{
    m_Events.onClose = [&windowManager](const sf::Event::Closed&)
	{
		windowManager.getMainWindow().close();
	};
	// Can add stuff to happen on window resize, etc. (like scaling?)
}