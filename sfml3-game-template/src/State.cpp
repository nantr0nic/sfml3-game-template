#include "State.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "AppContext.hpp"
#include "AppData.hpp"
#include "AssetKeys.hpp"
#include "Managers/StateManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/Systems.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"

#include <memory>
#include <format>
#include <string>

/**
 * @brief Constructs the main menu state and initializes its UI and input handlers.
 *
 * Initializes the title text, menu buttons, and state-specific event bindings.
 *
 * @param appContext Reference to the application's shared context (window, registry, resources, and state manager) used by the state.
 */
MenuState::MenuState(AppContext& appContext)
    : State(appContext)
{
    initTitleText();
    initMenuButtons();
    assignStateEvents();

    logger::Info("MenuState initialized.");
}

/**
 * @brief Destroys all menu UI entities created by this state.
 *
 * Removes every entity with the MenuUITag from the application's ECS registry to
 * ensure menu UI resources are cleaned up when the MenuState is destroyed.
 */
MenuState::~MenuState()
{
    // clean up EnTT entities on leaving MenuState
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

/**
 * @brief Updates UI interaction state for the menu each frame.
 *
 * Runs the UI hover handling for menu UI elements. The provided `deltaTime`
 * parameter is accepted for interface compatibility but is not used.
 *
 * @param deltaTime Frame time elapsed since the last update (ignored).
 */
void MenuState::update([[maybe_unused]] sf::Time deltaTime)
{
    // Call the UI hover system here
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

/**
 * @brief Render the state's UI elements and title text to the main window.
 *
 * Runs the UI render system for the state's registry and window, then draws
 * the stored title text if one exists.
 */
void MenuState::render()
{
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    if (m_TitleText)
    {
        m_AppContext.m_MainWindow->draw(*m_TitleText);
    }
}

/**
 * @brief Initializes the menu title text and positions it centered near the top of the window.
 *
 * Creates the title text using the ScoreFont with the string "Game Template", centers its origin,
 * positions it at the window center offset upward, sets a near-white fill color, and applies italic style.
 * If the ScoreFont resource is not available, the function does nothing.
 */
void MenuState::initTitleText()
{
    sf::Vector2f center = getWindowCenter();

    sf::Font* titleFont = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::ScoreFont);
    if (!titleFont)
    {
        logger::Error("Couldn't load ScoreFont. Not drawing title.");
        return;
    }

    m_TitleText.emplace(*titleFont, "Game Template", 120);
    utils::centerOrigin(*m_TitleText);
    m_TitleText->setPosition({ center.x, center.y - 150.0f });
    m_TitleText->setFillColor(sf::Color(250, 250, 250));
    m_TitleText->setStyle(sf::Text::Style::Italic);
}

/**
 * @brief Creates the main menu buttons ("Play" and "Settings") and wires their actions.
 *
 * Loads the main UI font and, if available, creates a centered "Play" button and a "Settings"
 * button 150 pixels below it. The "Play" button replaces the current state with a new
 * PlayState; the "Settings" button replaces the current state with a new SettingsMenuState.
 *
 * If the main font cannot be loaded, logs an error and does not create the buttons.
 */
void MenuState::initMenuButtons()
{
    sf::Vector2f center = getWindowCenter();

    // Main menu buttons
    sf::Font* buttonFont = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (!buttonFont)
    {
        logger::Error("Couldn't load MainFont. Not drawing text to buttons.");
        return;
    }

    EntityFactory::createButton(m_AppContext, *buttonFont, "Play", center,
        [this]() {
            auto playState = std::make_unique<PlayState>(m_AppContext);
            m_AppContext.m_StateManager->replaceState(std::move(playState));
        }
    );
    EntityFactory::createButton(m_AppContext, *buttonFont, "Settings",
        {center.x, center.y + 150.0f},
        [this]() {
            auto settingsState = std::make_unique<SettingsMenuState>(m_AppContext);
            m_AppContext.m_StateManager->replaceState(std::move(settingsState));
        }
    );
}

/**
 * @brief Register input event handlers used by the menu state.
 *
 * Sets the state's mouse-button handler to invoke the UI click system and
 * sets the key-press handler to close the main window when the Escape key is pressed.
 */
void MenuState::assignStateEvents()
{
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };
}

/**
 * @brief Constructs the settings menu state and prepares its UI and input handlers.
 *
 * Initializes settings-related UI controls, positions elements, and wires state input events.
 *
 * @param fromPlayState When true, the settings menu was opened from the running game and the Back action will resume play; when false, Back returns to the main menu.
 */

SettingsMenuState::SettingsMenuState(AppContext& context, bool fromPlayState)
    : State(context), m_FromPlayState(fromPlayState)
{
    initMenuButtons();
    assignStateEvents();

    logger::Info("SettingsMenuState initialized.");
}

/**
 * @brief Destroys all entities that belong to the settings UI.
 *
 * Removes every entity with the `SettingsUITag` from the application's ECS registry
 * to clean up settings-related UI resources when the state is torn down.
 */
SettingsMenuState::~SettingsMenuState()
{
    // Clean up Menu UI entities
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<SettingsUITag>();
    registry.destroy(view.begin(), view.end());
}

/**
 * @brief Updates UI interactions and synchronizes displayed audio volume values.
 *
 * Runs the UI hover and settings check systems, then refreshes the on-screen
 * music and SFX volume text to match the current application settings.
 */
void SettingsMenuState::update([[maybe_unused]] sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    UISystems::uiSettingsChecks(m_AppContext);

    // Update volume text
    if (m_MusicVolumeText.has_value())
    {
        m_MusicVolumeText->setString(std::to_string(
                            static_cast<int>(m_AppContext.m_AppSettings.musicVolume)));
    }
    if (m_SfxVolumeText.has_value())
    {
        m_SfxVolumeText->setString(std::to_string(
                            static_cast<int>(m_AppContext.m_AppSettings.sfxVolume)));
    }
}

/**
 * @brief Render the settings menu UI to the main window.
 *
 * Draws the settings background, invokes the UI render system, and draws the current
 * music and SFX volume texts if they exist.
 */
void SettingsMenuState::render()
{
    m_AppContext.m_MainWindow->draw(m_Background);

    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);

    if (m_MusicVolumeText)
    {
        m_AppContext.m_MainWindow->draw(*m_MusicVolumeText);
    }
    if (m_SfxVolumeText)
    {
        m_AppContext.m_MainWindow->draw(*m_SfxVolumeText);
    }
}

/**
 * @brief Create and configure the Settings menu UI elements.
 *
 * Initializes the settings menu background, volume display texts, increment/decrement
 * controls for music and SFX volumes, mute toggles, and the Back button; wires each
 * control to update application settings, music playback where applicable, and UI toggle
 * conditions in the registry. If required resources (fonts, textures, or main music)
 * are missing, logs an error and aborts initialization.
 */
void SettingsMenuState::initMenuButtons()
{
    sf::Vector2f windowSize = { m_AppContext.m_AppSettings.targetWidth,
                                m_AppContext.m_AppSettings.targetHeight };
    sf::Vector2f center = getWindowCenter();

    // Add semi-transparent background so buttons are visible if accessed during PauseState
    m_Background.setSize({ windowSize.x - 250.f, windowSize.y - 50.0f });
    utils::centerOrigin(m_Background);
    m_Background.setFillColor(sf::Color(0, 0, 0, 150)); // Black with ~60% alpha
    m_Background.setPosition(center);

    auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(
                                                        Assets::Fonts::ScoreFont);
    auto* buttonBackground = m_AppContext.m_ResourceManager->getResource<sf::Texture>(
                                                            Assets::Textures::ButtonBackground);
    auto* leftArrowButton = m_AppContext.m_ResourceManager->getResource<sf::Texture>(
                                                            Assets::Textures::ButtonLeftArrow);
    auto* rightArrowButton = m_AppContext.m_ResourceManager->getResource<sf::Texture>(
                                                            Assets::Textures::ButtonRightArrow);
    auto* music = m_AppContext.m_ResourceManager->getResource<sf::Music>(
                                                        Assets::Musics::MainSong);

    if (!font)
    {
        logger::Error("Couldn't load ScoreFont. Can't draw Settings buttons.");
        return;
    }
    if (!buttonBackground || !leftArrowButton)
    {
        logger::Error("Couldn't load ButtonBackground. Can't draw Settings buttons.");
        return;
    }
    if (!music)
    {
        logger::Error("No MainSong. Can't adjust music settings.");
        return;
    }

    //$ --- Settings Buttons --- //
    // Button positions
    sf::Vector2f sfxVolumeTextPos = { center.x, center.y - 130.0f };
    sf::Vector2f leftSfxArrowPos = { center.x - 90, center.y - 150.0f };
    sf::Vector2f rightSfxArrowPos = { center.x + 50, center.y - 150.0f };

    sf::Vector2f musicVolumeTextPos = { center.x, center.y - 80.0f };
    sf::Vector2f leftMusicArrowPos = { center.x - 90, center.y - 100.0f };
    sf::Vector2f rightMusicArrowPos = { center.x + 50, center.y - 100.0f };

    sf::Vector2f muteSfxPos = { center.x, center.y };
    sf::Vector2f muteMusicPos = { center.x, center.y + 100.0f };
    sf::Vector2f backButtonPos = { center.x, windowSize.y - 75.0f };

    // Current sfx volume text
    std::string sfxVolumeText = std::to_string(
                                  static_cast<int>(m_AppContext.m_AppSettings.sfxVolume));
    m_SfxVolumeText.emplace(*font, sfxVolumeText, 48);
    utils::centerOrigin(*m_SfxVolumeText);
    m_SfxVolumeText->setPosition(sfxVolumeTextPos);
    m_SfxVolumeText->setFillColor(sf::Color(250, 250, 250));

    // Adjust sfx volume buttons
    auto decreaseSfxVolume = [this]() {
        float currentVolume = m_AppContext.m_AppSettings.sfxVolume;
        m_AppContext.m_AppSettings.setSfxVolume(currentVolume - 10.0f);
        m_SfxVolumeText->setString(std::to_string(static_cast<int>(m_AppContext.m_AppSettings.sfxVolume)));
    };
    auto increaseSfxVolume = [this]() {
        float currentVolume = m_AppContext.m_AppSettings.sfxVolume;
        m_AppContext.m_AppSettings.setSfxVolume(currentVolume + 10.0f);
        m_SfxVolumeText->setString(std::to_string(static_cast<int>(m_AppContext.m_AppSettings.sfxVolume)));
    };

    auto leftSfxArrow = EntityFactory::createLabeledButton(m_AppContext, *leftArrowButton,
                                            leftSfxArrowPos, decreaseSfxVolume, *font,
                                            UITags::Settings, "SFX Volume: ", 36);
    auto rightSfxArrow = EntityFactory::createGUIButton(m_AppContext, *rightArrowButton,
                                            rightSfxArrowPos, increaseSfxVolume,
                                            UITags::Settings);

    // Current music volume text
    std::string musicVolumeText = std::to_string(
                                  static_cast<int>(m_AppContext.m_AppSettings.musicVolume));
    m_MusicVolumeText.emplace(*font, musicVolumeText, 48);
    utils::centerOrigin(*m_MusicVolumeText);
    m_MusicVolumeText->setPosition(musicVolumeTextPos);
    m_MusicVolumeText->setFillColor(sf::Color(250, 250, 250));

    // Adjust Music Volume buttons
    auto decreaseMusicVolume = [this, music]() {
        float currentVolume = m_AppContext.m_AppSettings.musicVolume;
        float decAmount = 10.0f;
        m_AppContext.m_AppSettings.setMusicVolume((currentVolume - decAmount), *music);
        };
    auto increaseMusicVolume = [this, music]() {
        float currentVolume = m_AppContext.m_AppSettings.musicVolume;
        float decAmount = 10.0f;
        m_AppContext.m_AppSettings.setMusicVolume((currentVolume + decAmount), *music);
        };
    // Adjust music arrows
    auto leftMusicArrow = EntityFactory::createLabeledButton(m_AppContext, *leftArrowButton,
                                            leftMusicArrowPos, decreaseMusicVolume, *font,
                                            UITags::Settings, "Music Volume: ", 36);
    auto rightMusicArrow = EntityFactory::createGUIButton(m_AppContext, *rightArrowButton,
                                            rightMusicArrowPos, increaseMusicVolume,
                                            UITags::Settings);



    // Mute music button
    auto toggleMusicMute = [this]() { m_AppContext.m_AppSettings.toggleMusicMute(); };
    auto muteMusicButton = EntityFactory::createLabeledButton(m_AppContext, *buttonBackground,
                            muteMusicPos, toggleMusicMute, *font, UITags::Settings, "Mute Music",
                            36, sf::Color::White);
    m_AppContext.m_Registry->emplace<UIToggleCond>(muteMusicButton, [this]() {
        return m_AppContext.m_AppSettings.musicMuted;
    });

    // Mute SFX button
    auto toggleSfxMute = [this]() { m_AppContext.m_AppSettings.toggleSfxMute(); };
    auto muteSfxButton = EntityFactory::createLabeledButton(m_AppContext, *buttonBackground,
                            muteSfxPos, toggleSfxMute, *font, UITags::Settings, "Mute SFX",
                            36, sf::Color::White);
    m_AppContext.m_Registry->emplace<UIToggleCond>(muteSfxButton, [this]() {
        return m_AppContext.m_AppSettings.sfxMuted;
    });

    // Back button
    sf::Vector2f backButtonSize = { 150.0f, 50.0f };
    auto backButton = EntityFactory::createButton(m_AppContext, *font, "Back",
        backButtonPos,
        [this]() {
            if (m_FromPlayState)
            {
                auto pauseState = std::make_unique<PauseState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(pauseState));
            }
            else
            {
                auto menuState = std::make_unique<MenuState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(menuState));
            }
        },
        UITags::Settings,
        backButtonSize
    );
}

/**
 * @brief Configure input event handlers for the settings menu state.
 *
 * Sets the mouse-button-press handler to process UI click interactions and
 * sets the key-press handler to close the main window when the Escape key is pressed.
 */
void SettingsMenuState::assignStateEvents()
{
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };
}

/**
 * @brief Initializes the gameplay state: spawns the player, configures background music, and wires keyboard controls.
 *
 * Creates the player entity, attempts to load and play the main background music (respecting the app's mute setting),
 * and installs key handlers: Escape closes the main window, 'P' pushes the pause state, and F12 toggles debug mode.
 *
 * @param appContext Reference to the application context used for resources, window access, settings, and state management.
 */

PlayState::PlayState(AppContext& appContext)
    : State(appContext)
{
    // We create the player entity here
    sf::Vector2f center = getWindowCenter();
    EntityFactory::createPlayer(m_AppContext, { center.x, center.y });

    // Handle Music
    m_Music = appContext.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    if (!m_Music)
    {
        logger::Error("Couldn't load MainSong! Music will not be played.");
    }
    else
    {
        if (appContext.m_AppSettings.musicMuted)
        {
            logger::Info("Music muted, not playing MainSong.");
        }
        else
        {
            m_Music->setLooping(true);
            m_Music->play();
            logger::Info("Playing MainSong");
        }
    }

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event)
    {
        // "Global" Escape key
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
        // State-specific Pause key
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            auto pauseState = std::make_unique<PauseState>(m_AppContext);
            m_AppContext.m_StateManager->pushState(std::move(pauseState));
        }
        else if (event.scancode == sf::Keyboard::Scancode::F12)
        {
            m_ShowDebug = !m_ShowDebug;
            logger::Warn(std::format("Debug mode toggled: {}", m_ShowDebug ? "On" : "Off"));
        }
    };

    logger::Info("PlayState initialized.");
}

/**
 * @brief Cleans up player entities when the play state is destroyed.
 *
 * Removes all entities tagged as players from the application's ECS registry.
 */
PlayState::~PlayState()
{
    // Clean up all player entities
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<PlayerTag>();
    registry.destroy(view.begin(), view.end());

    // Here you would also clean up enemies, bullets, etc.
    // (e.g., registry.clear<EnemyTag, BulletTag>();)
}

/**
 * @brief Advances the play state's game world by processing core gameplay systems.
 *
 * Processes player input and updates entity facing, animations, and movement using the provided time step.
 *
 * @param deltaTime Time elapsed since the last update; used to advance time-dependent systems.
 */
void PlayState::update(sf::Time deltaTime)
{
    // Call game logic systems
    CoreSystems::handlePlayerInput(m_AppContext);
    CoreSystems::facingSystem(*m_AppContext.m_Registry);
    CoreSystems::animationSystem(*m_AppContext.m_Registry, deltaTime);
    CoreSystems::movementSystem(*m_AppContext.m_Registry, deltaTime, *m_AppContext.m_MainWindow);
}

/**
 * @brief Renders the current game scene to the main window.
 *
 * Draws world entities, UI, and other visual elements managed by the rendering system.
 * When debug mode is enabled, additional debug overlays (e.g., collision bounds and entity info) are drawn.
 */
void PlayState::render()
{
    CoreSystems::renderSystem(
        *m_AppContext.m_Registry,
        *m_AppContext.m_MainWindow,
        m_ShowDebug
    );
}


/**
 * @brief Constructs the pause state, displays pause UI, and installs input handlers.
 *
 * Creates a centered "Paused" label and a Settings button, pauses any currently playing main song,
 * and sets event handlers so mouse clicks dispatch UI clicks, Escape closes the main window,
 * and the 'P' key resumes the game (and resumes music if applicable) by popping the pause state.
 *
 * @param context Reference to the application context used for resources, window, registry, and state management.
 */
PauseState::PauseState(AppContext& context)
    : State(context)
{
    sf::Font* font = context.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);

    if (!font)
    {
        logger::Error("MainFont not found! Can't make pause text.");
    }
    else
    {
        // Paused text
        m_PauseText.emplace(*font, "Paused", 100);
        m_PauseText->setFillColor(sf::Color::Red);
        utils::centerOrigin(*m_PauseText);
        sf::Vector2f center = getWindowCenter();
        m_PauseText->setPosition(center);

        // Settings button
        sf::Vector2f buttonSize{ 200.0f, 50.0f };
        EntityFactory::createButton(context, *font, "Settings",
            { center.x, center.y + 100.0f },
            [this]() {
                auto settingsState = std::make_unique<SettingsMenuState>(m_AppContext, true);
                m_AppContext.m_StateManager->replaceState(std::move(settingsState));
            },
            UITags::Pause,
            buttonSize
        );
    }

    // Handle music
    auto* music = context.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);

    if (music && music->getStatus() == sf::Music::Status::Playing)
    {
        music->pause();
    }

    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
            UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
        };

    m_StateEvents.onKeyPress = [this, music](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            bool shouldResume = (music && !m_AppContext.m_AppSettings.musicMuted
                                       && music->getStatus() == sf::Music::Status::Paused);
            if (shouldResume && music)
            {
                music->play();
            }
            m_AppContext.m_StateManager->popState();
            logger::Info("Game unpaused.");
        }
    };

    logger::Info("Game paused.");
}

/**
 * @brief Removes all entities associated with the pause state's UI from the ECS registry.
 *
 * Destroys every entity carrying the PauseUITag so UI resources created for the pause state are cleaned up.
 */
PauseState::~PauseState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<PauseUITag>();
    registry.destroy(view.begin(), view.end());
}

/**
 * @brief Update hover states of the pause menu UI elements.
 *
 * Invokes the UI hover system to refresh which pause-menu widgets are hovered
 * based on the current registry and window state.
 */
void PauseState::update([[maybe_unused]] sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

/**
 * @brief Renders the pause state's UI and the centered "Paused" text when present.
 *
 * Invokes the UI render system using the application's registry and main window, then draws
 * the pause text entity to the main window if it has been created.
 */
void PauseState::render()
{
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    if (m_PauseText)
    {
        m_AppContext.m_MainWindow->draw(*m_PauseText);
    }
}

//$ ----- Game Transition State ----- //

/**
 * @brief Initialize a state used to present level transition screens (win, loss, game complete).
 *
 * This constructs the transition UI (title and buttons), wires state event handlers, and stops
 * any currently playing main background music so the transition screen can control audio.
 *
 * @param context Reference to the application context providing access to resources, window, registry, and state stack.
 * @param type Specifies the kind of transition to display (e.g., LevelWin, LevelLoss, GameWin) which determines text, colors, and button behavior.
 */

GameTransitionState::GameTransitionState(AppContext& context, TransitionType type)
    : State(context)
{
    initTitleText(type);
    initMenuButtons(type);
    assignStateEvents();

    // Handle music stuff
    auto* music = context.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    bool wasMusicPlaying = (music && music->getStatus() == sf::Music::Status::Playing);

    if (wasMusicPlaying)
    {
        music->stop();
    }

    logger::Info("Game transition state initialized.");
}

/**
 * @brief Cleans up transition UI entities created for this state.
 *
 * Destroys all entities in the ECS registry that are tagged with `TransUITag`.
 */
GameTransitionState::~GameTransitionState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<TransUITag>();
    registry.destroy(view.begin(), view.end());
}

/**
 * @brief Processes hover interactions for this state's UI elements.
 *
 * Updates hover state for interactive UI components using the application's registry and main window.
 */
void GameTransitionState::update([[maybe_unused]] sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

/**
 * @brief Renders the transition state's UI elements.
 *
 * Draws all UI buttons using the UI render system and, if a transition text is set,
 * draws that text to the main window.
 */
void GameTransitionState::render()
{
    // Render buttons
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    // Render the text
    if (m_TransitionText)
    {
        m_AppContext.m_MainWindow->draw(*m_TransitionText);
    }
}

/**
 * @brief Creates and positions the transition title text for the given transition type.
 *
 * Initializes m_TransitionText with a message and color determined by the provided TransitionType,
 * centers its origin, sets its character size to 100, and positions it at the window center shifted
 * 200 pixels upward.
 *
 * @param type Specifies which transition message and color to display:
 *             - TransitionType::LevelLoss → "Oops! Level lost." (red)
 *             - TransitionType::LevelWin  → "Level Complete!" (green)
 *             - TransitionType::GameWin   → "You beat the game! Woo!" (yellow)
 *
 * If the main font cannot be loaded or an invalid transition type is supplied, an error is logged
 * and the method returns without creating or modifying the transition text.
 */
void GameTransitionState::initTitleText(TransitionType type)
{
    auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (!font)
    {
        logger::Error("Couldn't load font. Can't make transition state title text.");
        return;
    }

    sf::Vector2f center = getWindowCenter();
    sf::Vector2f textPosition(center.x, center.y - 200.0f);
    std::string stateMessage{};
    sf::Color stateMessageColor = sf::Color::White;

    switch (type)
    {
        case TransitionType::LevelLoss:
            stateMessage = "Oops! Level lost.";
            stateMessageColor = sf::Color::Red;
            break;
        case TransitionType::LevelWin:
            stateMessage = "Level Complete!";
            stateMessageColor = sf::Color::Green;
            break;
        case TransitionType::GameWin:
            stateMessage = "You beat the game! Woo!";
            stateMessageColor = sf::Color::Yellow;
            break;
        default:
            logger::Error("Invalid transition type.");
            break;
    }

    m_TransitionText.emplace(*font, stateMessage, 100);
    m_TransitionText->setFillColor(stateMessageColor);
    utils::centerOrigin(*m_TransitionText);
    m_TransitionText->setPosition(textPosition);
}

/**
 * @brief Creates and places the transition state's menu buttons.
 *
 * Initializes three vertically arranged buttons (top, "Main Menu", "Quit") centered in the window.
 * The top button's label and action depend on the provided TransitionType:
 * - LevelLoss: "Try Again" — restarts the current level.
 * - LevelWin: "Next Level" — advances to the next level if available, then starts play.
 * - GameWin: "Restart" — resets application progress and restarts play.
 *
 * The "Main Menu" button resets application progress and switches to the main menu state.
 * The "Quit" button closes the main window.
 *
 * If the required MainFont resource cannot be loaded, the function logs an error and returns without creating any buttons.
 *
 * @param type The transition kind that determines the top button's label and action.
 */
void GameTransitionState::initMenuButtons(TransitionType type)
{
    auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (!font)
    {
        logger::Error("Couldn't load font. Can't make transition state title text.");
        return;
    }

    sf::Vector2f center = getWindowCenter();

    // Button positions
    sf::Vector2f topButtonPos = { center.x, center.y - 70.0f };
    sf::Vector2f middleButtonPos = { center.x, center.y + 50} ;
    sf::Vector2f bottomButtonPos = { center.x, center.y + 200.0f };

    bool nextLevelExists = (m_AppContext.m_AppData.levelNumber < m_AppContext.m_AppData.totalLevels
                            ? true : false);

    std::string topButtonText{};
    // Set button tag to TransUITag
    UITags buttonTag = UITags::Transition;

    // Create top button
    switch (type)
    {
        case TransitionType::LevelLoss:
            topButtonText = "Try Again";
            EntityFactory::createButton(
                m_AppContext,
                *font,
                topButtonText,
                topButtonPos,
                [this]() {
                    logger::Info("Try Again button pressed.");
                    m_AppContext.m_AppData.levelStarted = false;
                    auto playState = std::make_unique<PlayState>(m_AppContext);
                    m_AppContext.m_StateManager->replaceState(std::move(playState));
                },
                buttonTag
            );
            break;
        case TransitionType::LevelWin:
            topButtonText = "Next Level";
            EntityFactory::createButton(
                m_AppContext,
                *font,
                topButtonText,
                topButtonPos,
                [this, nextLevelExists]() {
                    logger::Info("Next Level button pressed.");
                    m_AppContext.m_AppData.levelStarted = false;
                    if (nextLevelExists)
                    {
                        m_AppContext.m_AppData.levelNumber++;
                    }
                    auto playState = std::make_unique<PlayState>(m_AppContext);
                    m_AppContext.m_StateManager->replaceState(std::move(playState));
                },
                buttonTag
            );
            break;
        case TransitionType::GameWin:
            topButtonText = "Restart";
            EntityFactory::createButton(
                m_AppContext,
                *font,
                topButtonText,
                topButtonPos,
                [this]() {
                    logger::Info("Restart button pressed.");
                    m_AppContext.m_AppData.reset();
                    auto playState = std::make_unique<PlayState>(m_AppContext);
                    m_AppContext.m_StateManager->replaceState(std::move(playState));
                },
                buttonTag
            );
            break;
    }

    // make the "Main Menu" button
    EntityFactory::createButton(
        m_AppContext,
        *font,
        "Main Menu",
        middleButtonPos,
        [this]() {
            logger::Info("Main menu button pressed.");
            m_AppContext.m_AppData.reset();
            auto menuState = std::make_unique<MenuState>(m_AppContext);
            m_AppContext.m_StateManager->replaceState(std::move(menuState));
        },
        buttonTag
    );

    // make the "Quit" button
    EntityFactory::createButton(
        m_AppContext,
        *font,
        "Quit",
        bottomButtonPos,
        [this]() {
            logger::Info("Quit button pressed.");
            m_AppContext.m_MainWindow->close();
        },
        buttonTag
    );
}

/**
 * @brief Assigns input event handlers used while the transition state is active.
 *
 * Sets the mouse-button-press handler to invoke the UI click system and the key-press handler to close the main window when Escape is pressed.
 */
void GameTransitionState::assignStateEvents()
{
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };

    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
    };

}