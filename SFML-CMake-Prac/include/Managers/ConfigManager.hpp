#pragma once

#include <toml.hpp>

#include <optional>
#include <string_view>
#include <string>
#include <map>
#include <functional>
#include <format>

#include "Utilities/Logger.hpp"

class ConfigManager
{
public:
    ConfigManager() noexcept = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ~ConfigManager() noexcept = default;

    void loadConfig(std::string_view configID, std::string_view filepath);

    // getConfigValue requires (configID, key) or (configID, at, key)
    template<typename T>
    [[nodiscard]] std::optional<T> getConfigValue(
        std::string_view configID, std::string_view key) const;

    // getConfigValue requires (configID, key) or (configID, at, key)
    template<typename T>
    [[nodiscard]] std::optional<T> getConfigValue(
        std::string_view configID, std::string_view at, std::string_view key) const;

private:
    std::map<std::string, toml::value, std::less<>> m_ConfigFiles;

};


template<typename T>
std::optional<T> ConfigManager::getConfigValue(
    std::string_view configID, std::string_view key) const
{
    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("Config file [{}] not found.", configID));
        return std::nullopt;
    }

    try
    {
        return toml::find<T>(it->second, std::string(key));
    }
    catch (const std::exception& e)
    {
        logger::Error(std::format("Error getting config key [{}]: {}", key, e.what()));
        return std::nullopt;
    }
}

template<typename T>
std::optional<T> ConfigManager::getConfigValue(
    std::string_view configID, std::string_view at, std::string_view key) const
{
    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("Config file [{}] not found.", configID));
        return std::nullopt;
    }

    try
    {
        return toml::find<T>(it->second, std::string(at), std::string(key));
    }
    catch (const std::exception& e)
    {
        logger::Error(std::format("Error getting config key [{}] under [{}]: {}", key, at, e.what()));
        return std::nullopt;
    }
}