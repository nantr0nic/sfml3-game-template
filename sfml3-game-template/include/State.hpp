#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "AppContext.hpp"

#include <functional>
#include <optional>

struct StateEvents
{
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress;
    std::function<void(const sf::Event::MouseButtonPressed&)> onMouseButtonPress;
};

class State
{
public:
    explicit State(AppContext& appContext) : m_AppContext(appContext) {}
    virtual ~State() = default;

    StateEvents& getEventHandlers() noexcept { return m_StateEvents; }
    const StateEvents& getEventHandlers() const noexcept { return m_StateEvents; }

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;

protected:
    AppContext& m_AppContext;
    StateEvents m_StateEvents; 
    
    sf::Vector2f getWindowCenter() const noexcept
    {
        sf::Vector2f windowSize = { m_AppContext.m_AppSettings.targetWidth,
                                    m_AppContext.m_AppSettings.targetHeight };
        return { windowSize.x / 2.0f, windowSize.y / 2.0f };
    }
};

class MenuState : public State
{
public:
    MenuState(AppContext& appContext);
    virtual ~MenuState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    // Empty -- replaced previous data members with ECS components
};

class PlayState : public State
{
public:
    PlayState(AppContext& appContext);
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
    PauseState(AppContext& appContext);
    virtual ~PauseState() override = default;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_PauseText;
};
