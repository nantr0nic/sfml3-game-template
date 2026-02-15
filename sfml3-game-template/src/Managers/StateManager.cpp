#include "Managers/StateManager.hpp"

#include <memory>
#include <utility>

StateManager::StateManager(AppContext& context)
    : m_AppContext(context)
{
}

StateManager::~StateManager()
{
}

void StateManager::pushState(std::unique_ptr<State> state)
{
    m_PendingChanges.push_back({ StateAction::Push, std::move(state) });
}

void StateManager::popState()
{
    m_PendingChanges.push_back({ StateAction::Pop, nullptr });
}

void StateManager::replaceState(std::unique_ptr<State> state)
{
    m_PendingChanges.push_back({ StateAction::Replace, std::move(state) });
}

/**
 * @brief Apply all queued state changes to the managed state stack.
 *
 * Processes a snapshot of pending actions, clears the pending queue, and applies each
 * action in order to the internal state stack.
 *
 * - Push: appends the queued state to the top of the stack.
 * - Pop: removes the top state if the stack is not empty.
 * - Replace: removes the top state if present, then appends the queued state.
 *
 * After this call, the pending changes container is empty and the state stack reflects
 * all applied actions.
 */
void StateManager::processPending()
{
    auto pending = std::move(m_PendingChanges);
    m_PendingChanges.clear();
    for (auto& change : pending)
    {
        switch (change.action)
        {
            case StateAction::Push:
                m_States.push_back(std::move(change.state));
                break;
            case StateAction::Pop:
                if (!m_States.empty())
                {
                    m_States.pop_back();
                }
                break;
            case StateAction::Replace:
                if (!m_States.empty())
                {
                    m_States.pop_back();
                }
                m_States.push_back(std::move(change.state));
                break;
            default:
                break;
        }
    }
}

/**
 * @brief Get the active state at the top of the stack.
 *
 * @return State* Pointer to the top `State`, or `nullptr` if no states are present.
 */
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