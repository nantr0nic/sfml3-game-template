#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Event.hpp>

#include "AppContext.hpp"
#include "AppData.hpp"
#include "AssetKeys.hpp"
#include "State.hpp"
#include "Managers/StateManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/EntityFactory.hpp"
#include "ECS/Systems.hpp"
#include "Utilities/Utils.hpp"
#include "Utilities/Logger.hpp"

#include <memory>
#include <format>

//$ ----- MenuState Implementation ----- //
MenuState::MenuState(AppContext& appContext)
    : State(appContext)
{
    initTitleText();
    initMenuButtons();
    assignStateEvents();

    logger::Info("MenuState initialized.");
}

MenuState::~MenuState()
{
    // clean up EnTT entities on leaving MenuState
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}

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

SettingsMenuState::~SettingsMenuState()
{
    // Clean up Menu UI entities
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<SettingsUITag>();
    registry.destroy(view.begin(), view.end());
}

void SettingsMenuState::update(sf::Time deltaTime)
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

PlayState::~PlayState()
{
    // Clean up all player entities
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<PlayerTag>();
    registry.destroy(view.begin(), view.end());

    // Here you would also clean up enemies, bullets, etc.
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


//$ ----- PauseState Implementation -----
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

PauseState::~PauseState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<PauseUITag>();
    registry.destroy(view.begin(), view.end());
}

void PauseState::update(sf::Time deltaTime)
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
