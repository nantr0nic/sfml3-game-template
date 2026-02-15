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
     * @brief Toggle the SFX mute state.
     *
     * Flips the stored SFX mute flag and logs the new state.
     */
    void toggleSfxMute()
    {
        sfxMuted = !sfxMuted;
        logger::Info(std::format("SFX muted: {}", sfxMuted ? "true" : "false"));
    }
    
    /**
     * @brief Retrieve the stored music volume level.
     *
     * Also logs the current music volume via logger::Info.
     *
     * @return float Current music volume in the range 0 to 100.
     */
    float getMusicVolume() const
    {
        logger::Info(std::format("Music volume: {}", musicVolume));
        return musicVolume;
    }
    
    /**
     * @brief Gets the current sound-effect (SFX) volume.
     *
     * @return float Current SFX volume as a float in the range 0 to 100.
     */
    float getSfxVolume() const
    {
        logger::Info(std::format("SFX volume: {}", sfxVolume));
        return sfxVolume;
    }
    
    /**
     * @brief Set and apply the music volume, clamping the value to the range 0–100.
     *
     * Stores the clamped volume in the settings, applies it to the provided SFML
     * Music object via setVolume, and logs the new value.
     *
     * @param volume Desired volume level in percent; values outside 0–100 are clamped.
     * @param music SFML Music instance whose volume will be updated.
     */
    void setMusicVolume(float volume, sf::Music& music)
    {
        musicVolume = std::clamp(volume, 0.0f, 100.0f);
        music.setVolume(musicVolume);
        
        logger::Info(std::format("Music volume set to: {}", musicVolume));
    }
    
    /**
     * @brief Set the sound effects (SFX) volume.
     *
     * Stores the provided volume clamped to the range 0 to 100 and logs the new value.
     *
     * @param volume Desired volume level; values outside [0, 100] will be clamped.
     */
    void setSfxVolume(float volume)
    {
        // SFX volume is stored but not applied to active sounds (cuz they're short-lived)
        sfxVolume = std::clamp(volume, 0.0f, 100.0f);
        
        logger::Info(std::format("SFX volume set to: {}", sfxVolume));
    }
};