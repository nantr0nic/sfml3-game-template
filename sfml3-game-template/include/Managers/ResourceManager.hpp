#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <toml++/toml.hpp>

#include "Utilities/Logger.hpp"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <functional>
#include <format>

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

template<typename T>
/**
 * @brief Loads a resource of type `T` from the specified file and stores it under the given identifier.
 *
 * Attempts to load the file at `filepath` as a resource of type `T` and inserts or replaces the managed
 * resource in the internal storage using `id` as the key. Supported types for `T` are `sf::Font`,
 * `sf::Texture`, `sf::SoundBuffer`, and `sf::Music`.
 *
 * On successful load the function records an informational log entry; on failure it records an error
 * log entry and does not throw. If `T` is not one of the supported types, an error is logged and no
 * resource is stored.
 *
 * @tparam T Resource type to load (`sf::Font`, `sf::Texture`, `sf::SoundBuffer`, or `sf::Music`).
 * @param id Key under which the loaded resource will be stored.
 * @param filepath Filesystem path to the resource to load.
 */
void ResourceManager::loadResource(std::string_view id, std::string_view filepath)
{
    if constexpr (std::is_same_v<T, sf::Font>) 
    {
        auto font = std::make_unique<sf::Font>();
        if (!font->openFromFile(filepath)) 
        {
            logger::Error(std::format("Failed to load font: {}", filepath));
        }
        m_Fonts.insert_or_assign(std::string(id), std::move(font));
        logger::Info(std::format("Font ID \"{}\" loaded from: {}", id, filepath));
    } 
    else if constexpr (std::is_same_v<T, sf::Texture>) 
    {
        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(filepath)) 
        {
            logger::Error(std::format("Failed to load texture: {}",  filepath));
        }
        m_Textures.insert_or_assign(std::string(id), std::move(texture));
        logger::Info(std::format("Texture ID \"{}\" loaded from: {}", id, filepath));
    } 
    else if constexpr (std::is_same_v<T, sf::SoundBuffer>) 
    {
        auto soundBuffer = std::make_unique<sf::SoundBuffer>();
        if (!soundBuffer->loadFromFile(filepath)) 
        {
            logger::Error(std::format("Failed to load sound buffer: {}", filepath));
        }
        m_SoundBuffers.insert_or_assign(std::string(id), std::move(soundBuffer));
        logger::Info(std::format("SoundBuffer ID \"{}\" loaded from: {}", id, filepath));
    }
    else if constexpr (std::is_same_v<T, sf::Music>) 
    {
        auto music = std::make_unique<sf::Music>();
        if (!music->openFromFile(filepath)) 
        {
            logger::Error(std::format("Failed to load music: {}", filepath));
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