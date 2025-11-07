#pragma once

#include <SFML/Graphics.hpp>
#include "AppContext.hpp"

struct StateEvents
{
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress;
    std::function<void(const sf::Event::MouseButtonPressed&)> onMouseButtonPress;
};

class State
{
public:
    State(AppContext* appContext) : m_AppContext(appContext) {}
    virtual ~State() = default;

    /* enter() and exit() are left here as a reminder for the future
    * These can be left empty, but can be used to load context stuff
    * like music, menu options, reading configs from file for menu layout, etc.
    * They can be left empty if not needed.
    */
    //virtual void enter() = 0;
    //virtual void exit() = 0;

    StateEvents& getEventHandlers() { return m_StateEvents; }
    const StateEvents& getEventHandlers() const { return m_StateEvents; }

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;

protected:
    AppContext* m_AppContext;
    StateEvents m_StateEvents;  // each state will have its own StateEvents instance
};

class MenuState : public State
{
public:
    MenuState(AppContext* appContext);
    ~MenuState() override = default;

    void update(sf::Time deltaTime) override;
    void render() override;

private:
    sf::Text m_PlayText;
    sf::RectangleShape m_PlayButton;
};

class PlayState : public State
{
public:
    PlayState(AppContext* appContext);
    ~PlayState() override = default;

    void update(sf::Time deltaTime) override;
    void render() override;

private:
    // nothing yet
};

class PauseState : public State
{
public:
    PauseState(AppContext* appContext);
    ~PauseState() override = default;

    void update(sf::Time deltaTime) override;
    void render() override;

private:
    sf::Text m_PauseText;
};
