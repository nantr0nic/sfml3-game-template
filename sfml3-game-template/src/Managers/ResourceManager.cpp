#include "Managers/ResourceManager.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <toml++/toml.hpp>

#include "Utilities/Logger.hpp"

#include <string>
#include <string_view>
#include <format>

/**
 * @brief Loads fonts, textures, sound buffers, and musics defined in a TOML manifest.
 *
 * Parses the TOML file at the given path, reads the "fonts", "textures",
 * "soundbuffers", and "musics" arrays, and loads each entry that provides both
 * an `id` and `path`. If parsing the manifest fails, an error is logged and the
 * function returns without loading any assets; on success an informational
 * message is logged.
 *
 * @param filepath Path to the TOML manifest file describing assets.
 */
void ResourceManager::loadAssetsFromManifest(std::string_view filepath)
{
    toml::parse_result manifestFile = toml::parse_file(filepath);

    if (!manifestFile)
    {
        logger::Error(std::format(
            "Error parsing manifest file --> {}", manifestFile.error().description()
        ));
        return;
    }

    // Load Fonts
    if (auto fonts = manifestFile["fonts"].as_array())
    {
        for (const auto& item : *fonts)
        {
            toml::node_view view(item);

            std::string id = view["id"].value_or("");
            std::string path = view["path"].value_or("");

            if (!id.empty() && !path.empty())
            {
                loadResource<sf::Font>(id, path);
            }
        }
    }

    // Load Textures
    if (auto textures = manifestFile["textures"].as_array())
    {
        for (const auto& item : *textures)
        {
            toml::node_view view(item);

            std::string id = view["id"].value_or("");
            std::string path = view["path"].value_or("");

            if (!id.empty() && !path.empty())
            {
                loadResource<sf::Texture>(id, path);
            }
        }
    }

    // Load SoundBuffers
    if (auto soundBuffers = manifestFile["soundbuffers"].as_array())
    {
        for (const auto& item : *soundBuffers)
        {
            toml::node_view view(item);

            std::string id = view["id"].value_or("");
            std::string path = view["path"].value_or("");

            if (!id.empty() && !path.empty())
            {
                loadResource<sf::SoundBuffer>(id, path);
            }
        }
    }

    // Load Musics
    if (auto musics = manifestFile["musics"].as_array())
    {
        for (const auto& item : *musics)
        {
            toml::node_view view(item);

            std::string id = view["id"].value_or("");
            std::string path = view["path"].value_or("");

            if (!id.empty() && !path.empty())
            {
                loadResource<sf::Music>(id, path);
            }
        }
    }

    logger::Info(std::format("Assets manifest successfully loaded from: {}", filepath));
}