#pragma once

#include <SFML/Graphics.hpp>
#include "AppContext.hpp"

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

    virtual void handleEvent() = 0;
    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;

protected:
    AppContext* m_AppContext;
};

class MenuState : public State
{
public:
    MenuState(AppContext* appContext);
    ~MenuState() override = default;

    void handleEvent() override;
    void update(sf::Time deltaTime) override;
    void render() override;

private:
    //! replace m_Font/loadFont() with a ResourceManager function later
    static sf::Font loadFont();
    sf::Font m_Font;
    sf::Text m_PlayText;
    sf::RectangleShape m_PlayButton;
};

class PlayState : public State
{
public:
    PlayState(AppContext* appContext);
    ~PlayState() override = default;

    void handleEvent() override;
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

    void handleEvent() override;
    void update(sf::Time deltaTime) override;
    void render() override;

private:
    // nothing yet
};
