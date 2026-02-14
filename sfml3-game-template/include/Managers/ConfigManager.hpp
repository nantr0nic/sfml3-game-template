#pragma once

#include <toml++/toml.hpp>

#include "Utilities/Logger.hpp"

#include <optional>
#include <string_view>
#include <string>
#include <map>
#include <format>
#include <source_location>
#include <vector>

class ConfigManager
{
public:
    ConfigManager() noexcept = default;
    ConfigManager(const ConfigManager&) = delete;
    /**
 * @brief Deleted copy assignment operator to make ConfigManager non-copyable.
 *
 * Ensures instances cannot be copy-assigned, preventing duplication of internal configuration state.
 */
ConfigManager& operator=(const ConfigManager&) = delete;
    /**
 * @brief Defaulted destructor for ConfigManager.
 *
 * Releases resources held by the ConfigManager implementation.
 */
~ConfigManager() noexcept = default;

    /**
 * @brief Provides a copy of all loaded configuration files mapped by their config IDs.
 *
 * @return std::map<std::string, toml::table, std::less<>> A copy of the internal map where each key is a configuration ID and each value is the corresponding parsed TOML table.
 */
const std::map<std::string, toml::table, std::less<>> getConfigFiles() const { return m_ConfigFiles; }

    void loadConfig(std::string_view configID, std::string_view filepath);

    [[nodiscard]] const toml::table* getConfigTable(std::string_view configID) const;

    std::vector<std::string> getStringArray(
        std::string_view configID, std::string_view section,
        std::string_view key,
        const std::source_location& loc = std::source_location::current()) const;

    // getConfigValue requires (configID, key) or (configID, section, key)
    template<typename T>
    [[nodiscard]] std::optional<T> getConfigValue(
        std::string_view configID, std::string_view key,
        const std::source_location& loc = std::source_location::current()) const;

        template<typename T>
    [[nodiscard]] std::optional<T> getConfigValue(
        std::string_view configID, std::string_view section, std::string_view key,
        const std::source_location& loc = std::source_location::current()) const;

private:
    std::map<std::string, toml::table, std::less<>> m_ConfigFiles;

};


template<typename T>
/**
 * @brief Retrieve a value of type `T` from a top-level key in the specified configuration.
 *
 * Attempts to look up `configID` in the loaded configuration files and read the value at the top-level `key`.
 *
 * @tparam T The expected type of the configuration value.
 * @param configID Identifier of the configuration file to query.
 * @param key Top-level key within the configuration table.
 * @param loc Source location used for diagnostic logging (defaults to call site).
 * @return std::optional<T> `std::optional` containing the value if the configuration and key are found; `std::nullopt` if the config ID is missing or the key is not present.
 */
std::optional<T> ConfigManager::getConfigValue(
    std::string_view configID, std::string_view key, const std::source_location& loc) const
{
    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("File: {}({}:{}) -> Config file ID [{}] not found.",
            logger::formatPath(loc.file_name()), loc.line(), loc.column(), configID));
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
/**
 * @brief Retrieve a typed value from a nested section/key in a loaded TOML config.
 *
 * Attempts to read a value of type `T` from the specified `section` and `key`
 * inside the configuration identified by `configID`. Logs an error if the
 * config ID is not found and logs a warning if the section or key is missing.
 *
 * @tparam T The expected type of the configuration value.
 * @param configID Identifier of the loaded configuration to query.
 * @param section Top-level table name within the configuration.
 * @param key Key name inside `section` whose value will be retrieved.
 * @param loc Source location used for error reporting (defaults to call site).
 * @return std::optional<T> Containing the value if present and convertible to `T`, `std::nullopt` otherwise.
 */
std::optional<T> ConfigManager::getConfigValue(
    std::string_view configID, std::string_view section, std::string_view key,
    const std::source_location& loc) const
{
    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("File: {}({}:{}) -> Config file ID [{}] not found.",
            logger::formatPath(loc.file_name()), loc.line(), loc.column(), configID));
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