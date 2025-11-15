#pragma once

#include <toml.hpp>

#include <print>
#include <optional>
#include <string_view>
#include <string>

class ConfigManager
{
public:
    ConfigManager() noexcept = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ~ConfigManager() noexcept = default;

    void loadConfig(std::string_view filepath);

    // getConfigValue can accept either a bare key in a .toml file,
    // or it can handle bracket headings with keys.
    template<typename T>
    std::optional<T> getConfigValue(std::string_view key);

    template<typename T>
    std::optional<T> getConfigValue(std::string_view at, std::string_view key);

private:
    toml::value m_ConfigFile;

};

inline void ConfigManager::loadConfig(std::string_view filepath)
{
    try
    {
        m_ConfigFile = toml::parse(std::string(filepath));
        std::println("Config file loaded successfully: {}", filepath);
    }
    catch (const std::exception& e)
    {
        std::println("Error loading config file: {}", e.what());
    }
}

template<typename T>
std::optional<T> ConfigManager::getConfigValue(std::string_view key)
{
    try
    {
        return toml::find<T>(m_ConfigFile, key);
    }
    catch (const std::exception& e)
    {
        std::println("Error getting config key [{}]: {}", key, e.what());
        return std::nullopt;
    }
}

template<typename T>
std::optional<T> ConfigManager::getConfigValue(std::string_view at, std::string_view key)
{
    try
    {
        return toml::find<T>(m_ConfigFile, at, key);
    }
    catch (const std::exception& e)
    {
        std::println("Error getting config key [{}]: {}", key, e.what());
        return std::nullopt;
    }
}