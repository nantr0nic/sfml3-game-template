#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <functional>
#include <optional>

class AppContext;

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

    StateEvents& getEventHandlers() noexcept { return m_StateEvents; }
    const StateEvents& getEventHandlers() const noexcept { return m_StateEvents; }

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
    virtual ~MenuState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    // Empty -- replaced previous data members with ECS components
};

class PlayState : public State
{
public:
    PlayState(AppContext* appContext);
    virtual ~PlayState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    sf::Music* m_MainMusic{ nullptr };
    bool m_ShowDebug{ false };
};

class PauseState : public State
{
public:
    PauseState(AppContext* appContext);
    virtual ~PauseState() override = default;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_PauseText;
};
