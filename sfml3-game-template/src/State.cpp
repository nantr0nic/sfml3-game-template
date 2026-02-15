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

//$ ----- MenuState Implementation ----- //
MenuState::MenuState(AppContext& appContext)
    : State(appContext)
{
    initTitleText();
    initMenuButtons();
    assignStateEvents();

    logger::Info("MenuState initialized.");
}

/**
 * @brief Removes Menu UI entities from the ECS registry.
 *
 * Destroys all entities whose UITagID equals UITags::Menu to clean up menu-related UI.
 */
MenuState::~MenuState()
{
    auto& registry = *m_AppContext.m_Registry;
    // Clean up MenuState UI entities
    std::vector<entt::entity> entitiesToRemove;
    auto view = registry.view<UITagID>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.id == UITags::Menu)
        {
            entitiesToRemove.push_back(entity);
        }
    }
    for (auto entity : entitiesToRemove)
    {
        registry.destroy(entity);
    }
}

/**
 * @brief Updates hover interactions for the menu UI.
 *
 * Invokes the UI hover system to update hover state and visual feedback for menu UI elements.
 */
void MenuState::update([[maybe_unused]] sf::Time deltaTime)
{
    // Call the UI hover system here
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void MenuState::render()
{
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    if (m_TitleText)
    {
        m_AppContext.m_MainWindow->draw(*m_TitleText);
    }
}

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

//$ ----- Settings Menu State ----- //

SettingsMenuState::SettingsMenuState(AppContext& context, bool fromPlayState)
    : State(context), m_FromPlayState(fromPlayState)
{
    initMenuButtons();
    assignStateEvents();

    logger::Info("SettingsMenuState initialized.");
}

/**
 * @brief Destroys all UI entities associated with the settings menu.
 *
 * The destructor removes any entities tagged with `UITags::Settings` from the application's registry to clean up settings-menu UI state.
 */
SettingsMenuState::~SettingsMenuState()
{
    auto& registry = *m_AppContext.m_Registry;
    // Clean up SettingsMenu UI entities
    std::vector<entt::entity> entitiesToRemove;
    auto view = registry.view<UITagID>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.id == UITags::Settings)
        {
            entitiesToRemove.push_back(entity);
        }
    }
    for (auto entity : entitiesToRemove)
    {
        registry.destroy(entity);
    }
}

/**
 * @brief Update UI interactions and synchronize displayed volume values with current settings.
 *
 * Runs the UI hover and settings-check systems, then updates the optional music and SFX
 * volume text objects to match the current AppSettings volumes (formatted as integers).
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
 * @brief Create and position all UI controls for the settings menu and configure their actions.
 *
 * Initializes the semi-transparent settings panel and constructs SFX controls (volume display,
 * decrease/increase buttons, and a mute toggle). If the main music resource is available, also
 * constructs music controls (volume display, decrease/increase buttons, and a mute toggle);
 * otherwise skips music controls and emits a warning. Adds a Back button that replaces the
 * current state with PauseState when the settings were opened from play, or with MenuState
 * otherwise.
 *
 * This function creates entities tagged with UITags::Settings, sets m_Background, and populates
 * m_SfxVolumeText and m_MusicVolumeText when those text elements are created. It also attaches
 * UIToggleCond components for the mute toggles. If required font or button textures are missing
 * the function logs an error and returns without constructing the UI.
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
    if (!buttonBackground || !leftArrowButton || !rightArrowButton)
    {
        logger::Error("Couldn't load a button texture. Can't draw Settings buttons.");
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
        float stepAmount = 10.0f;
        m_AppContext.m_AppSettings.setSfxVolume(currentVolume - stepAmount);
    };
    auto increaseSfxVolume = [this]() {
        float currentVolume = m_AppContext.m_AppSettings.sfxVolume;
        float stepAmount = 10.0f;
        m_AppContext.m_AppSettings.setSfxVolume(currentVolume + stepAmount);
    };

    auto leftSfxArrow = EntityFactory::createLabeledButton(m_AppContext, *leftArrowButton,
                                            leftSfxArrowPos, decreaseSfxVolume, *font,
                                            UITags::Settings, "SFX Volume: ", 36);
    auto rightSfxArrow = EntityFactory::createGUIButton(m_AppContext, *rightArrowButton,
                                            rightSfxArrowPos, increaseSfxVolume,
                                            UITags::Settings);

    // Mute SFX button
    auto toggleSfxMute = [this]() { m_AppContext.m_AppSettings.toggleSfxMute(); };
    auto muteSfxButton = EntityFactory::createLabeledButton(m_AppContext, *buttonBackground,
                            muteSfxPos, toggleSfxMute, *font, UITags::Settings, "Mute SFX",
                            36, sf::Color::White);
    m_AppContext.m_Registry->emplace<UIToggleCond>(muteSfxButton, [this]() {
        return m_AppContext.m_AppSettings.sfxMuted;
    });

    if (music)
    {
        // Current music volume text
        std::string musicVolumeText = std::to_string(
                                    static_cast<int>(m_AppContext.m_AppSettings.musicVolume));
        m_MusicVolumeText.emplace(*font, musicVolumeText, 48);
        utils::centerOrigin(*m_MusicVolumeText);
        m_MusicVolumeText->setPosition(musicVolumeTextPos);
        m_MusicVolumeText->setFillColor(sf::Color(250, 250, 250));

        // Adjust Music Volume button functions
        auto decreaseMusicVolume = [this, music]() {
            float currentVolume = m_AppContext.m_AppSettings.musicVolume;
            float stepAmount = 10.0f;
            m_AppContext.m_AppSettings.setMusicVolume((currentVolume - stepAmount), *music);
            };
        auto increaseMusicVolume = [this, music]() {
            float currentVolume = m_AppContext.m_AppSettings.musicVolume;
            float stepAmount = 10.0f;
            m_AppContext.m_AppSettings.setMusicVolume((currentVolume + stepAmount), *music);
            };

        // Adjust music arrow buttons
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
    }
    else
    {
        logger::Warn("Settings UI: Music resource not found. Skipping music controls.");
    }

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

//$ ----- PlayState Implementation ----- //

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
 * @brief Destroys the PlayState and removes all entities tagged as PlayerTag from the ECS registry.
 *
 * The destructor removes every player entity from the application's registry. It does not remove other entity groups
 * (for example enemies, bullets, or HUD) — those should be cleaned up separately where appropriate.
 */
PlayState::~PlayState()
{
    // Clean up all player entities
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<PlayerTag>();
    registry.destroy(view.begin(), view.end());

    // Here you would also clean up enemies, bullets, HUD entities, etc.
    // (e.g., registry.clear<EnemyTag, BulletTag>();)
}

void PlayState::update(sf::Time deltaTime)
{
    // Call game logic systems
    CoreSystems::handlePlayerInput(m_AppContext);
    CoreSystems::facingSystem(*m_AppContext.m_Registry);
    CoreSystems::animationSystem(*m_AppContext.m_Registry, deltaTime);
    CoreSystems::movementSystem(*m_AppContext.m_Registry, deltaTime, *m_AppContext.m_MainWindow);
}

void PlayState::render()
{
    CoreSystems::renderSystem(
        *m_AppContext.m_Registry,
        *m_AppContext.m_MainWindow,
        m_ShowDebug
    );
}


/**
 * @brief Construct a PauseState, pause game audio, and build pause UI.
 *
 * Pauses the main music (if playing), records whether music should resume when unpausing,
 * creates centered "Paused" text, and constructs "Settings" and "Back" buttons.
 * Installs input handlers: mouse clicks are forwarded to the UI click system;
 * pressing Escape closes the main window; pressing 'P' or clicking "Back" will
 * pop this state (and resume music if it was paused here).
 *
 * @param context Application context used to access resources, window, state manager, and registry.
 */
PauseState::PauseState(AppContext& context)
    : State(context)
{
    sf::Vector2f windowSize = { m_AppContext.m_AppSettings.targetWidth,
                                m_AppContext.m_AppSettings.targetHeight };
    sf::Vector2f center = getWindowCenter();

    sf::Font* font = context.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    sf::Font* backFont = context.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::ScoreFont);
    
    // Handle music
    sf::Music* music = context.m_ResourceManager->getResource<sf::Music>(Assets::Musics::MainSong);
    if (music && music->getStatus() == sf::Music::Status::Playing)
    {
        music->pause();
    }
    bool musicShouldResume = (music && !m_AppContext.m_AppSettings.musicMuted
                               && music->getStatus() == sf::Music::Status::Paused);

    if (!font || !backFont)
    {
        logger::Error("MainFont or ScoreFont not found! Can't render certain text.");
    }
    else
    {
        // Paused text
        m_PauseText.emplace(*font, "Paused", 100);
        m_PauseText->setFillColor(sf::Color::Red);
        utils::centerOrigin(*m_PauseText);
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
        
        // Back button
        sf::Vector2f backButtonPos = { center.x, windowSize.y - 75.0f };
        sf::Vector2f backButtonSize = { 150.0f, 50.0f };
        auto backButton = EntityFactory::createButton(m_AppContext, *backFont, "Back",
            backButtonPos,
            [this, music, musicShouldResume]() {
                if (musicShouldResume)
                {
                    music->play();
                }
                m_AppContext.m_StateManager->popState();
                logger::Info("Game unpaused.");
            },
            UITags::Pause,
            backButtonSize
        );
    }

    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
            UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
        };

    m_StateEvents.onKeyPress = [this, music, musicShouldResume](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
        {
            m_AppContext.m_MainWindow->close();
        }
        else if (event.scancode == sf::Keyboard::Scancode::P)
        {
            if (musicShouldResume)
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
 * @brief Cleans up UI entities created for the pause state.
 *
 * Destroys all entities in the application's registry that are tagged with UITags::Pause to remove pause-menu UI elements.
 */
PauseState::~PauseState()
{
    auto& registry = *m_AppContext.m_Registry;
    // Clean up PauseState UI entities
    auto view = registry.view<UITagID>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.id == UITags::Pause)
        {
            registry.destroy(entity);
        }
    }
}

/**
 * @brief Updates hover interactions for the pause state's UI.
 *
 * Invokes the UI hover system to process mouse-over and hover effects for UI
 * elements associated with this state's registry and main window.
 *
 * @param deltaTime Unused.
 */
void PauseState::update([[maybe_unused]] sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void PauseState::render()
{
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    if (m_PauseText)
    {
        m_AppContext.m_MainWindow->draw(*m_PauseText);
    }
}

//$ ----- Game Transition State ----- //

/*
    This is currently unused in the template but is here for use.
    This state, as written, is intended for transitioning between level wins, losses, etc.
    Look at the Breakdown example game for an example of how it can be implemented.
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
 * @brief Destroys UI entities associated with the transition state.
 *
 * Iterates the registry and removes any entity tagged with `UITags::Transition`,
 * ensuring transition-specific UI is cleaned up when the state is destroyed.
 */
GameTransitionState::~GameTransitionState()
{
    auto& registry = *m_AppContext.m_Registry;
    // Clean up GameTransitionState UI elements
    std::vector<entt::entity> entitiesToRemove;
    auto view = registry.view<UITagID>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.id == UITags::Transition)
        {
            entitiesToRemove.push_back(entity);
        }
    }
    for (auto entity : entitiesToRemove)
    {
        registry.destroy(entity);
    }
}

/**
 * @brief Updates UI hover interactions for the transition state.
 *
 * Runs the UI hover system to update hover states for UI elements using the state's
 * entity registry and main window.
 */
void GameTransitionState::update([[maybe_unused]] sf::Time deltaTime)
{
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

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
 * @brief Creates and places the transition menu buttons for the current game transition.
 *
 * Constructs up to three buttons ("Try Again"/"Next Level"/"Restart" depending on the transition type,
 * "Main Menu", and "Quit") and registers their actions with the state manager or main window.
 * If the main font resource cannot be loaded, the function logs an error and aborts without creating any buttons.
 *
 * @param type The transition type that determines the top button's label and behavior:
 *             - TransitionType::LevelLoss -> "Try Again" (restarts current level)
 *             - TransitionType::LevelWin  -> "Next Level" (advances level if available)
 *             - TransitionType::GameWin   -> "Restart" (resets game state)
 */
void GameTransitionState::initMenuButtons(TransitionType type)
{
    auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (!font)
    {
        logger::Error("Couldn't load font. Can't make transition buttons text.");
        return;
    }

    sf::Vector2f center = getWindowCenter();

    // Button positions
    sf::Vector2f topButtonPos = { center.x, center.y - 70.0f };
    sf::Vector2f middleButtonPos = { center.x, center.y + 50} ;
    sf::Vector2f bottomButtonPos = { center.x, center.y + 200.0f };

    bool nextLevelExists = (m_AppContext.m_AppData.levelNumber < m_AppContext.m_AppData.totalLevels);

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
