#pragma once

#include <toml.hpp>

#include <print>
#include <iostream>
#include <optional>
#include <string_view>
#include <string>
#include <map>
#include <functional>

class ConfigManager
{
public:
    ConfigManager() noexcept = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ~ConfigManager() noexcept = default;

    void loadConfig(std::string_view configID, std::string_view filepath);

    // getConfigValue can accept either a bare key in a .toml file,
    // or it can handle bracket headings with keys.
    template<typename T>
    std::optional<T> getConfigValue(std::string_view configID, std::string_view key);

    template<typename T>
    std::optional<T> getConfigValue(std::string_view configID, std::string_view at, std::string_view key);

private:
    std::map<std::string, toml::value, std::less<>> m_ConfigFiles;

};


template<typename T>
std::optional<T> ConfigManager::getConfigValue(std::string_view configID, std::string_view key)
{
    auto it = m_ConfigFiles.find(std::string(configID));
    if (it == m_ConfigFiles.end())
    {
        std::println(std::cerr, "Config file [{}] not found.", configID);
        return std::nullopt;
    }

    try
    {
        return toml::find<T>(it->second, std::string(key));
    }
    catch (const std::exception& e)
    {
        std::println("Error getting config key [{}]: {}", key, e.what());
        return std::nullopt;
    }
}

template<typename T>
std::optional<T> ConfigManager::getConfigValue(std::string_view configID, std::string_view at, std::string_view key)
{
    auto it = m_ConfigFiles.find(std::string(configID));
    if (it == m_ConfigFiles.end())
    {
        std::println(std::cerr, "Config file [{}] not found.", configID);
        return std::nullopt;
    }

    try
    {
        return toml::find<T>(it->second, std::string(at), std::string(key));
    }
    catch (const std::exception& e)
    {
        std::println(std::cerr, "Error getting config key [{}] under [{}]: {}", key, at, e.what());
        return std::nullopt;
    }
}