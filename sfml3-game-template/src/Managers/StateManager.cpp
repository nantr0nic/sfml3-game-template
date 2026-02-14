#include "Managers/StateManager.hpp"

#include <memory>
#include <utility>

/**
 * @brief Construct a StateManager that holds the application context.
 *
 * Stores a reference to the application's AppContext so managed states can
 * access shared services and configuration.
 *
 * @param context Reference to the application's AppContext.
 */
StateManager::StateManager(AppContext& context)
    : m_AppContext(context)
{
}

StateManager::~StateManager()
{
}

/**
 * @brief Enqueue a request to push a new state onto the state stack.
 *
 * Adds a pending Push action that will place the provided state on top of the stack when pending changes are processed.
 *
 * @param state Unique pointer to the State to be pushed; ownership is transferred into the manager.
 */
void StateManager::pushState(std::unique_ptr<State> state)
{
    m_PendingChanges.push_back({ StateAction::Push, std::move(state) });
}

/**
 * @brief Schedules removal of the current state from the state stack.
 *
 * Adds a "pop" action to the pending-change queue; the top state will be removed when pending changes are processed.
 */
void StateManager::popState()
{
    m_PendingChanges.push_back({ StateAction::Pop, nullptr });
}

/**
 * @brief Enqueues a replace action to swap the current state with the provided one.
 *
 * The action is queued and will take effect when pending changes are processed.
 *
 * @param state Ownership of the new state; moved into the pending change and used to replace the current state when applied.
 */
void StateManager::replaceState(std::unique_ptr<State> state)
{
    m_PendingChanges.push_back({ StateAction::Replace, std::move(state) });
}

/**
 * @brief Applies all queued state changes to the state stack.
 *
 * Processes each entry in the pending change queue in order and mutates the internal
 * state stack according to the action: `Push` moves the provided state onto the stack,
 * `Pop` removes the top state if one exists, and `Replace` removes the top state if one
 * exists then pushes the provided state. Clears the pending queue after processing.
 */
void StateManager::processPending()
{
    for (auto& change : m_PendingChanges)
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
    m_PendingChanges.clear();
}

/**
 * @brief Retrieve the current top State.
 *
 * @return State* Pointer to the current (top) State, or `nullptr` if the state stack is empty.
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

/**
 * @brief Render all managed states in stack order.
 *
 * Calls each state's render method in the order they are stored (bottom to top),
 * so underlying states are rendered before states above them.
 */
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