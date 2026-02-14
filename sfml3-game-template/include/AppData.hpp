#pragma once

#include <SFML/Audio/Sound.hpp>

#include "Utilities/Logger.hpp"

#include <list>
#include <format>

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
    
    /**
     * @brief Reinitializes the level state and clears currently active sounds.
     *
     * Resets `levelStarted` to false, sets `levelNumber` to 1, and clears `activeSounds`.
     */
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
    
    /**
     * @brief Toggle the music mute state.
     *
     * Flips the `musicMuted` flag and logs the new state.
     */
    void toggleMusicMute()
    {
        musicMuted = !musicMuted;
        logger::Info(std::format("Music muted: {}", musicMuted ? "true" : "false"));
    }
    
    /**
     * @brief Toggle the sound effects mute state.
     *
     * Flips the `sfxMuted` flag and logs the resulting state using the logger.
     */
    void toggleSfxMute()
    {
        sfxMuted = !sfxMuted;
        logger::Info(std::format("SFX muted: {}", sfxMuted ? "true" : "false"));
    }
    
    /**
     * @brief Retrieve the current music volume.
     *
     * Logs the current music volume and returns its value.
     *
     * @return Current music volume in the range 0.0 to 100.0.
     */
    float getMusicVolume()
    {
        logger::Info(std::format("Music volume: {}", musicVolume));
        return musicVolume;
    }
    
    /**
     * @brief Gets the current sound effects volume level.
     *
     * @return float Current SFX volume in the range 0.0 to 100.0.
     */
    float getSfxVolume()
    {
        logger::Info(std::format("SFX volume: {}", sfxVolume));
        return sfxVolume;
    }
    
    /**
     * @brief Update the stored music volume, clamp it to the valid range, and apply it to the provided music instance.
     *
     * Sets the object's music volume to the given value constrained to the range 0.0 to 100.0, applies that volume to
     * the supplied sf::Music object, and logs the resulting volume.
     *
     * @param volume Desired volume level; values below 0.0 will be treated as 0.0 and values above 100.0 will be treated as 100.0.
     * @param music Reference to the sf::Music instance that will receive the updated volume via its setVolume method.
     */
    void setMusicVolume(float volume, sf::Music& music)
    {
        musicVolume = volume;
        
        if (musicVolume <= 0.0f)
        {
            musicVolume = 0.0f;
        }
        else if (musicVolume > 100.0f)
        {
            musicVolume = 100.0f;
        }
        music.setVolume(musicVolume);
        
        logger::Info(std::format("Music volume set to: {}", musicVolume));
    }
    
    /**
     * @brief Set the sound effects (SFX) volume, clamped to the range [0, 100].
     *
     * @param volume Desired volume level; values less than 0 are applied as 0, values greater than 100 are applied as 100.
     */
    void setSfxVolume(float volume)
    {
        sfxVolume = volume;
        
        if (sfxVolume <= 0.0f)
        {
            sfxVolume = 0.0f;
        }
        else if (sfxVolume > 100.0f)
        {
            sfxVolume = 100.0f;
        }
        
        logger::Info(std::format("SFX volume set to: {}", sfxVolume));
    }
};