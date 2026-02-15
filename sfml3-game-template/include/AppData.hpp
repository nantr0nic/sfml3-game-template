#pragma once

#include <SFML/Audio/Sound.hpp>

#include "Utilities/Logger.hpp"

#include <list>
#include <format>
#include <algorithm>

struct AppData
{
    // Game audio storage: holds sounds while they are playing
    std::list<sf::Sound> activeSounds;
    
    /*
    Below is an example of some game data. It is currently unused in the template.
    Also see how GameTransitionState is used to manage transitions between game states
    and manipulates the data here.
    */
    
    // Game level data 
    bool levelStarted{ false };
    int levelNumber{ 1 };
    int totalLevels{ 1 };
    
    void reset()
    {
        levelStarted = false;
        levelNumber = 1;
        activeSounds.clear();
    }

};

struct AppSettings
{
    // Resolution target settings
    float targetWidth{ 1280.0f };
    float targetHeight{ 720.0f };
    
    // Audio settings
    bool musicMuted{ false };
    bool sfxMuted{ false };
    float musicVolume{ 100.0f };
    float sfxVolume{ 100.0f };
    
    void toggleMusicMute()
    {
        musicMuted = !musicMuted;
        logger::Info(std::format("Music muted: {}", musicMuted ? "true" : "false"));
    }
    
    /**
     * @brief Toggle the sound effects mute state.
     *
     * Flips the AppSettings::sfxMuted flag between true and false and logs the new state.
     */
    void toggleSfxMute()
    {
        sfxMuted = !sfxMuted;
        logger::Info(std::format("SFX muted: {}", sfxMuted ? "true" : "false"));
    }
    
    /**
     * @brief Retrieves the current music volume level.
     *
     * @return float Current music volume, in the range 0 to 100.
     */
    float getMusicVolume() const
    {
        logger::Info(std::format("Music volume: {}", musicVolume));
        return musicVolume;
    }
    
    /**
     * @brief Retrieves the current sound-effect (SFX) volume level.
     *
     * @return float Current SFX volume in the range 0–100.
     */
    float getSfxVolume() const
    {
        logger::Info(std::format("SFX volume: {}", sfxVolume));
        return sfxVolume;
    }
    
    /**
     * @brief Set the music volume, clamping the value to the range [0, 100] and applying it to the given music instance.
     *
     * @param volume Desired volume level (will be clamped to 0–100).
     * @param music Reference to the sf::Music instance whose volume will be updated.
     */
    void setMusicVolume(float volume, sf::Music& music)
    {
        musicVolume = std::clamp(volume, 0.0f, 100.0f);
        music.setVolume(musicVolume);
        
        logger::Info(std::format("Music volume set to: {}", musicVolume));
    }
    
    /**
     * @brief Sets the SFX volume, clamping the value to the range 0–100.
     *
     * @param volume Desired volume level; values less than 0 are treated as 0 and values greater than 100 are treated as 100.
     */
    void setSfxVolume(float volume)
    {
        // SFX volume is stored but not applied to active sounds (cuz they're short-lived)
        sfxVolume = std::clamp(volume, 0.0f, 100.0f);
        
        logger::Info(std::format("SFX volume set to: {}", sfxVolume));
    }
};