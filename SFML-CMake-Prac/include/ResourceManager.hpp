//$ I heard on a podcast header-only stuff like this is bad practice but because of the
//$ templated functions there's nothing else to implement here so I guess this is it for now

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <map>
#include <memory>
#include <string>
#include <iostream>

class ResourceManager 
{
public:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ~ResourceManager() = default;

    template<typename T>
    void loadResource(const std::string& id, const std::string& filepath);

    template<typename T>
    T& getResource(const std::string& id);

    template<typename T>
    const T& getResource(const std::string& id) const;

private:
    std::map<std::string, std::unique_ptr<sf::Font>> m_Fonts;
    std::map<std::string, std::unique_ptr<sf::Texture>> m_Textures;
    std::map<std::string, std::unique_ptr<sf::SoundBuffer>> m_SoundBuffers;
    std::map<std::string, std::unique_ptr<sf::Music>> m_Musics;

};

//! VVV This is not good error handling, rework this later VVV

template<typename T>
void ResourceManager::loadResource(const std::string& id, const std::string& filepath)
{
    if constexpr (std::is_same_v<T, sf::Font>) 
    {
        auto font = std::make_unique<sf::Font>();
        if (!font->openFromFile(filepath)) 
        {
            std::cerr << "Failed to load font: " << filepath << std::endl;
            return;
        }
        m_Fonts[id] = std::move(font);
    } 
    else if constexpr (std::is_same_v<T, sf::Texture>) 
    {
        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(filepath)) 
        {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            return;
        }
        m_Textures[id] = std::move(texture);
    } 
    else if constexpr (std::is_same_v<T, sf::SoundBuffer>) 
    {
        auto soundBuffer = std::make_unique<sf::SoundBuffer>();
        if (!soundBuffer->loadFromFile(filepath)) 
        {
            std::cerr << "Failed to load sound buffer: " << filepath << std::endl;
            return;
        }
        m_SoundBuffers[id] = std::move(soundBuffer);
    }
    else if constexpr (std::is_same_v<T, sf::Music>) 
    {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(filepath)) 
        {
            std::cerr << "Failed to load music file: " << filepath << std::endl;
            return;
        }
        m_Musics[id] = std::move(music);
    }
    else 
    {
        std::cerr << "Unsupported resource type." << std::endl;
        return;
    }
}

template<typename T>
T& ResourceManager::getResource(const std::string& id)
{
    if constexpr (std::is_same_v<T, sf::Font>) 
    {
        return *m_Fonts.at(id);
    } 
    else if constexpr (std::is_same_v<T, sf::Texture>) 
    {
        return *m_Textures.at(id);
    } 
    else if constexpr (std::is_same_v<T, sf::SoundBuffer>) 
    {
        return *m_SoundBuffers.at(id);
    }
    else if constexpr (std::is_same_v<T, sf::Music>) 
    {
        return *m_Musics.at(id);
    }
    else 
    {
        std::cerr << "Something went wrong retrieving resource.\nWrong ID?" << std::endl;
    }
}

template<typename T>
const T& ResourceManager::getResource(const std::string& id) const
{
    if constexpr (std::is_same_v<T, sf::Font>) 
    {
        return *m_Fonts.at(id);
    } 
    else if constexpr (std::is_same_v<T, sf::Texture>) 
    {
        return *m_Textures.at(id);
    } 
    else if constexpr (std::is_same_v<T, sf::SoundBuffer>) 
    {
        return *m_SoundBuffers.at(id);
    }
    else if constexpr (std::is_same_v<T, sf::Music>) 
    {
        return *m_Musics.at(id);
    }
    else 
    {
        std::cerr << "Something went wrong retrieving resource.\nWrong ID?" << std::endl;
    }
}