#pragma once

#include "State.hpp"
#include "AppContext.hpp"

#include <stack>
#include <memory>

class StateManager
{
public:
    StateManager(AppContext* appContext);
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    ~StateManager();

    void pushState(std::unique_ptr<State> state);
    void popState();
    void replaceState(std::unique_ptr<State> state);

    State* getCurrentState();

    void handleEvent();
    void update(sf::Time deltaTime);
    void render();

private:
    std::stack<std::unique_ptr<State>> m_States;
    AppContext& m_AppContext;
};