#pragma once

#include <SFML/System/Time.hpp>

#include "State.hpp"
#include "AppContext.hpp"

#include <vector>
#include <memory>

enum class StateAction { None, Push, Pop, Replace };

struct PendingChange
{
    StateAction action;
    std::unique_ptr<State> state = nullptr;
};

class StateManager
{
public:
    explicit StateManager(AppContext& context);
    /**
 * @brief Disables copying of a StateManager.
 *
 * Prevents creation of a new StateManager by copying an existing one to ensure
 * unique ownership of managed states and the application context.
 */
StateManager(const StateManager&) = delete;
    /**
 * @brief Disabled copy assignment to prevent copying of StateManager instances.
 *
 * Copying a StateManager is not allowed because it manages unique ownership of states
 * and holds a reference to AppContext.
 */
StateManager& operator=(const StateManager&) = delete;
    ~StateManager();

    void pushState(std::unique_ptr<State> state);
    void popState();
    void replaceState(std::unique_ptr<State> state);

    void processPending();

    State* getCurrentState() noexcept;
    const State* getCurrentState() const noexcept;

    void update(sf::Time deltaTime);
    void render();

private:
    std::vector<std::unique_ptr<State>> m_States;
    AppContext& m_AppContext;

    std::vector<PendingChange> m_PendingChanges;
};