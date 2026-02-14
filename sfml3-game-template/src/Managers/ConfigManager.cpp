#include <toml++/toml.hpp>

#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"

#include <format>
#include <string_view>
#include <string>
#include <vector>

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
