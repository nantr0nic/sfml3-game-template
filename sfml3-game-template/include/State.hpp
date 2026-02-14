#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>

#include "AppContext.hpp"

#include <functional>
#include <optional>

struct StateEvents
{
	std::function<void(const sf::Event::KeyPressed&)> onKeyPress = [](const auto&){};
    std::function<void(const sf::Event::MouseButtonPressed&)> onMouseButtonPress = [](const auto&){};
};

enum class TransitionType
{
    LevelLoss,
    LevelWin,
    GameWin
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
    void initTitleText();
    void initMenuButtons();
    void assignStateEvents();
    
private:
    std::optional<sf::Text> m_TitleText;
};

class SettingsMenuState : public State
{
public:
    explicit SettingsMenuState(AppContext& context, bool fromPlayState = false);
    virtual ~SettingsMenuState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;
    
private:
    void initMenuButtons();
    void assignStateEvents();

private:
    sf::RectangleShape m_Background;
    std::optional<sf::Text> m_MusicVolumeText;
    std::optional<sf::Text> m_SfxVolumeText;
    bool m_FromPlayState;
};

class PlayState : public State
{
public:
    PlayState(AppContext& appContext);
    virtual ~PlayState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    sf::Music* m_Music{ nullptr };
    bool m_ShowDebug{ false };
};

class PauseState : public State
{
public:
    PauseState(AppContext& appContext);
    virtual ~PauseState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_PauseText;
};

class GameTransitionState : public State
{
public:
    explicit GameTransitionState(AppContext& context, 
                                TransitionType type = TransitionType::LevelLoss);
    virtual ~GameTransitionState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    std::optional<sf::Text> m_TransitionText;
    
private:
    void initTitleText(TransitionType type);
    void initMenuButtons(TransitionType type);
    void assignStateEvents();
};
