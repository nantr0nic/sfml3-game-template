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
    /**
 * @brief Constructs a State tied to the shared application context.
 *
 * Stores a reference to the provided AppContext for use by the state and its
 * derived classes.
 *
 * @param appContext Reference to the application's shared context.
 */
explicit State(AppContext& appContext) : m_AppContext(appContext) {}
    /**
 * @brief Ensures proper polymorphic destruction of State-derived objects.
 *
 * Declared virtual and defaulted so deleting a derived instance through a
 * pointer-to-State invokes the correct destructor chain.
 */
virtual ~State() = default;

    /**
 * @brief Accesses the state's modifiable event handlers.
 *
 * Provides writable access to the state's stored StateEvents so callers can
 * assign or modify input callback handlers for this state.
 *
 * @return Reference to the state's StateEvents allowing modification of its callbacks.
 */
StateEvents& getEventHandlers() noexcept { return m_StateEvents; }
    /**
 * @brief Provides read-only access to the state's event handlers.
 *
 * @return const StateEvents& Reference to the state's collection of event callbacks.
 */
const StateEvents& getEventHandlers() const noexcept { return m_StateEvents; }

    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;

protected:
    AppContext& m_AppContext;
    StateEvents m_StateEvents; 
    
    /**
     * @brief Computes the center point of the application window.
     *
     * Uses the configured target width and height from the application's settings to determine the window center.
     *
     * @return sf::Vector2f The (x, y) coordinates of the window center.
     */
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