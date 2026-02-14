#pragma once

#include <SFML/Audio.hpp>

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
    
    void toggleSfxMute()
    {
        sfxMuted = !sfxMuted;
        logger::Info(std::format("SFX muted: {}", sfxMuted ? "true" : "false"));
    }
    
    float getMusicVolume()
    {
        logger::Info(std::format("Music volume: {}", musicVolume));
        return musicVolume;
    }
    
    float getSfxVolume()
    {
        logger::Info(std::format("SFX volume: {}", sfxVolume));
        return sfxVolume;
    }
    
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