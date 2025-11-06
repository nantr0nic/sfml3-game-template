#pragma once

#include "State.hpp"

#include <stack>
#include <memory>

class StateManager
{
public:
    StateManager();
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    ~StateManager();

    void pushState(std::unique_ptr<State> state);
    void popState();
    void replaceState(std::unique_ptr<State> state);

    State* getCurrentState();

private:
    std::stack<std::unique_ptr<State>> m_States;
};