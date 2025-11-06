#include "StateManager.hpp"

StateManager::StateManager(AppContext* appContext)
    : m_AppContext(*appContext)
{
}

StateManager::~StateManager()
{
}

void StateManager::pushState(std::unique_ptr<State> state)
{
    m_States.push(std::move(state));
}

void StateManager::popState()
{
    if (!m_States.empty())
    {
        m_States.pop();
    }
}

void StateManager::replaceState(std::unique_ptr<State> state)
{
    if (!m_States.empty())
    {
        m_States.pop();
    }
    m_States.push(std::move(state));
}

State* StateManager::getCurrentState()
{
    if (m_States.empty())
    {
        return nullptr;
    }
    return m_States.top().get();
}

void StateManager::handleEvent()
{
    if (!m_States.empty())
    {
        m_States.top()->handleEvent();
    }
}

void StateManager::update(sf::Time deltaTime)
{
    if (!m_States.empty())
    {
        m_States.top()->update(deltaTime);
    }
}

void StateManager::render()
{
    if (!m_States.empty())
    {
        m_States.top()->render();
    }
}