#include "Managers/ConfigManager.hpp"

#include <toml++/toml.hpp>

#include "Utilities/Logger.hpp"

#include <format>
#include <string_view>
#include <string>
#include <vector>

/**
 * @brief Load a TOML configuration file and store its root table under the given config ID.
 *
 * Attempts to parse the TOML file at \p filepath and, on success, inserts or replaces the
 * stored configuration table for \p configID with the parsed root table. If parsing fails,
 * the stored configurations are not modified and an error is reported.
 *
 * @param configID Identifier used as the key for storing the parsed root table.
 * @param filepath Filesystem path to the TOML configuration file to load.
 */
void ConfigManager::loadConfig(std::string_view configID, std::string_view filepath)
{
    toml::parse_result configFile = toml::parse_file(filepath);

    if (!configFile)
    {
        logger::Error(std::format(
            "Error parsing config file --> {}", configFile.error().description()
        ));
        return;
    }

    m_ConfigFiles.insert_or_assign(std::string(configID), std::move(configFile.table()));

    logger::Info(std::format("Config ID \"{}\" loaded from: {}", configID, filepath));
}

/**
 * @brief Retrieve the TOML table associated with a configuration ID.
 *
 * @param configID Identifier of the loaded configuration.
 * @return const toml::table* Pointer to the table for the given `configID`, or `nullptr` if the configuration ID is not found.
 */
const toml::table* ConfigManager::getConfigTable(std::string_view configID) const
{
    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("Config file ID [{}] not found.", configID));
        return nullptr;
    }

    return &it->second;
}

/**
 * @brief Retrieves an array of strings from the TOML config at [section][key] for the given config ID.
 *
 * Returns the string elements found in the array located at the specified section and key of the TOML table
 * identified by `configID`. If the config ID, section, or key is missing, or the target is not an array, an
 * empty vector is returned. Non-string array elements are skipped and generate a warning.
 *
 * @param configID Identifier of the loaded configuration table.
 * @param section Section name inside the TOML table.
 * @param key Key name whose value is expected to be an array of strings.
 * @param loc Source location used for diagnostic logging when the config ID is missing.
 * @return std::vector<std::string> Vector containing the string elements from the array; empty on error or if no string elements are present.
 */
std::vector<std::string> ConfigManager::getStringArray(
    std::string_view configID, std::string_view section, std::string_view key,
    const std::source_location& loc) const
{
    std::vector<std::string> result;

    auto it = m_ConfigFiles.find(configID);
    if (it == m_ConfigFiles.end())
    {
        logger::Error(std::format("File: {}({}:{}) -> Config file ID [{}] not found.",
            logger::formatPath(loc.file_name()), loc.line(), loc.column(), configID));
        return result; // return empty
    }

    auto sectionNode = it->second[section];
    if (!sectionNode)
    {
        logger::Warn(std::format(
            "Section [{}] in Config [{}] not found.", section, configID));
        return result;
    }

    auto node = sectionNode[key];
    if (!node)
    {
        logger::Warn(std::format(
            "Key [{}] in Section [{}] of Config [{}] not found.", key, section, configID));
            return result;
    }

    if (!node.is_array())
    {
        logger::Warn(std::format(
            "Key [{}] in Section [{}] of Config [{}] is not an array.", key, section, configID));
        return result;
    }

    const auto& arr = *node.as_array();
    for (const auto& elem : arr)
    {
        if (auto str = elem.value<std::string>())
        {
            result.push_back(*str);
        }
        else
        {
            logger::Warn(std::format(
                "Non-string element in array [{}][{}] of Config [{}].", section, key, configID));
        }
    }

    return result;
}