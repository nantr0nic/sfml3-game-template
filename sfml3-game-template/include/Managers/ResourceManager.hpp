#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <toml++/toml.hpp>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <stdexcept>
#include <functional>
#include <format>

#include "Utilities/Logger.hpp"

class ResourceManager 
{
public:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ~ResourceManager() = default;

    void loadAssetsFromManifest(std::string_view filepath);

    template<typename T>
    void loadResource(std::string_view id, std::string_view filepath);

    template<typename T>
    [[nodiscard]] T* getResource(std::string_view id);

    template<typename T>
    [[nodiscard]] const T* getResource(std::string_view id) const;

private:
    std::map<std::string, std::unique_ptr<sf::Font>, std::less<>> m_Fonts;
    std::map<std::string, std::unique_ptr<sf::Texture>, std::less<>> m_Textures;
    std::map<std::string, std::unique_ptr<sf::SoundBuffer>, std::less<>> m_SoundBuffers;
    std::map<std::string, std::unique_ptr<sf::Music>, std::less<>> m_Musics;

};

//! VVV Try error handling that doesn't use throw/catch?

template<typename T>
void ResourceManager::loadResource(std::string_view id, std::string_view filepath)
{
    if constexpr (std::is_same_v<T, sf::Font>) 
    {
        auto font = std::make_unique<sf::Font>();
        if (!font->openFromFile(filepath)) 
        {
            throw std::runtime_error(std::format("Failed to load font: {}", filepath));
        }
        m_Fonts.insert_or_assign(std::string(id), std::move(font));
        logger::Info(std::format("Font ID \"{}\" loaded from: {}", id, filepath));
    } 
    else if constexpr (std::is_same_v<T, sf::Texture>) 
    {
        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(filepath)) 
        {
            throw std::runtime_error(std::format("Failed to load texture: {}",  filepath));
        }
        m_Textures.insert_or_assign(std::string(id), std::move(texture));
        logger::Info(std::format("Texture ID \"{}\" loaded from: {}", id, filepath));
    } 
    else if constexpr (std::is_same_v<T, sf::SoundBuffer>) 
    {
        auto soundBuffer = std::make_unique<sf::SoundBuffer>();
        if (!soundBuffer->loadFromFile(filepath)) 
        {
            throw std::runtime_error(std::format("Failed to load sound buffer: {}", filepath));
        }
        m_SoundBuffers.insert_or_assign(std::string(id), std::move(soundBuffer));
        logger::Info(std::format("SoundBuffer ID \"{}\" loaded from: {}", id, filepath));
    }
    else if constexpr (std::is_same_v<T, sf::Music>) 
    {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(filepath)) 
        {
            throw std::runtime_error(std::format("Failed to load music: {}", filepath));
        }
        m_Musics.insert_or_assign(std::string(id), std::move(music));
        logger::Info(std::format("Music ID \"{}\" loaded from: {}", id, filepath));
    }
    else 
    {
        logger::Error(std::format(
            "Couldn't load resource. Possibly: Unsupported type or wrong ID? (ID: {})",
            id));
        return;
    }
}

template<typename T>
T* ResourceManager::getResource(std::string_view id)
{
    if constexpr (std::is_same_v<T, sf::Font>) 
    {
        auto it = m_Fonts.find(id);
        return (it != m_Fonts.end()) ? it->second.get() : nullptr;
    } 
    else if constexpr (std::is_same_v<T, sf::Texture>) 
    {
        auto it = m_Textures.find(id);
        return (it != m_Textures.end()) ? it->second.get() : nullptr;
    } 
    else if constexpr (std::is_same_v<T, sf::SoundBuffer>) 
    {
        auto it = m_SoundBuffers.find(id);
        return (it != m_SoundBuffers.end()) ? it->second.get() : nullptr;
    }
    else if constexpr (std::is_same_v<T, sf::Music>) 
    {
        auto it = m_Musics.find(id);
        return (it != m_Musics.end()) ? it->second.get() : nullptr;
    }
    else 
    {
        logger::Error(std::format(
            "Couldn't get resource. Possibly: Unsupported type or wrong ID? (ID: {})",
            id));
        return nullptr;
    }
}

template<typename T>
const T* ResourceManager::getResource(std::string_view id) const
{
    if constexpr (std::is_same_v<T, sf::Font>) 
    {
        auto it = m_Fonts.find(id);
        return (it != m_Fonts.end()) ? it->second.get() : nullptr;
    } 
    else if constexpr (std::is_same_v<T, sf::Texture>) 
    {
        auto it = m_Textures.find(id);
        return (it != m_Textures.end()) ? it->second.get() : nullptr;
    } 
    else if constexpr (std::is_same_v<T, sf::SoundBuffer>) 
    {
        auto it = m_SoundBuffers.find(id);
        return (it != m_SoundBuffers.end()) ? it->second.get() : nullptr;
    }
    else if constexpr (std::is_same_v<T, sf::Music>) 
    {
        auto it = m_Musics.find(id);
        return (it != m_Musics.end()) ? it->second.get() : nullptr;
    }
    else 
    {
        logger::Error(std::format(
            "Couldn't get resource. Possibly: Unsupported type or wrong ID? (ID: {})",
            id));
        return nullptr;
    }
}