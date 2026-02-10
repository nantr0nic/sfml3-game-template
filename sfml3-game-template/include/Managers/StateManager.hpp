#pragma once

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
    StateManager(const StateManager&) = delete;
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