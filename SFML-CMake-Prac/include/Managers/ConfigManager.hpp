#pragma once

#include "TomlImp.hpp"

#include <optional>
#include <string_view>
#include <string>
#include <map>
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

    // getConfigValue requires (configID, key) or (configID, section, key)
    template<typename T>
    [[nodiscard]] std::optional<T> getConfigValue(
        std::string_view configID, std::string_view key) const;

        template<typename T>
    [[nodiscard]] std::optional<T> getConfigValue(
        std::string_view configID, std::string_view section, std::string_view key) const;

private:
    std::map<std::string, toml::table, std::less<>> m_ConfigFiles;

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

    auto retValue = it->second[key].value<T>();
    if (!retValue.has_value())
    {
        logger::Warn(std::format("Key [{}] not found in config file [{}].", key, configID));
        return std::nullopt;
    }

    return retValue;
}

template<typename T>
std::optional<T> ConfigManager::getConfigValue(
    std::string_view configID, std::string_view section, std::string_view key) const
{
    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("Config file [{}] not found.", configID));
        return std::nullopt;
    }

    auto retValue = it->second[section][key].value<T>();
    if (!retValue.has_value())
    {
        logger::Warn(std::format(
            "Section [{}] or Key [{}] not found in config file [{}].", section, key, configID
        ));
        return std::nullopt;
    }

    return retValue;
}