#include "StateManager.hpp"

StateManager::StateManager()
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