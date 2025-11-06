#pragma once

#include <SFML/Graphics.hpp>

class State
{
public:
    State() = default;
    virtual ~State() = default;

    /*
    * These can be left empty, but can be used to load context stuff
    * like music, menu options, reading configs from file for menu layout, etc.
    * They can be left empty if not needed.
    */
    virtual void enter() = 0;
    virtual void exit() = 0;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(sf::Time deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};

class MenuState : public State
{
public:
    MenuState() = default;
    ~MenuState() override = default;

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    // nothing yet
};

class PlayState : public State
{
public:
    PlayState() = default;
    ~PlayState() override = default;

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    // nothing yet
};

class PauseState : public State
{
public:
    PauseState() = default;
    ~PauseState() override = default;

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    // nothing yet
};
