#include "Managers/StateManager.hpp"

#include <utility>

StateManager::StateManager(AppContext* appContext)
    : m_AppContext(*appContext)
{
}

StateManager::~StateManager()
{
}

void StateManager::pushState(std::unique_ptr<State> state)
{
    m_States.push_back(std::move(state));
}

void StateManager::popState()
{
    if (!m_States.empty())
    {
        m_States.pop_back();
    }
}

void StateManager::replaceState(std::unique_ptr<State> state)
{
    if (!m_States.empty())
    {
        m_States.pop_back();
    }
    m_States.push_back(std::move(state));
}

State* StateManager::getCurrentState() noexcept
{
    if (m_States.empty())
    {
        return nullptr;
    }
    return m_States.back().get();
}

const State* StateManager::getCurrentState() const noexcept
{
    if (m_States.empty())
    {
        return nullptr;
    }
    return m_States.back().get();
}

void StateManager::update(sf::Time deltaTime)
{
    if (!m_States.empty())
    {
        m_States.back()->update(deltaTime);
    }
}

void StateManager::render()
{
    if (!m_States.empty())
    {
        for (auto& state : m_States)
        {
            state->render();
        }
    }
}