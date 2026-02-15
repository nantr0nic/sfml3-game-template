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
 * @brief Consumes and applies all queued state change requests.
 *
 * Processes every pending StateAction in the queue and updates the internal
 * state stack accordingly.
 *
 * Actions:
 * - Push: pushes the provided state onto the top of the stack.
 * - Pop: removes the top state if the stack is not empty.
 * - Replace: removes the top state if the stack is not empty, then pushes the provided state.
 *
 * Unknown actions are ignored. After this call, the pending changes queue is emptied.
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
 * @brief Retrieve the current active state.
 *
 * @return State* Pointer to the top `State` on the stack, or `nullptr` if the stack is empty.
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
