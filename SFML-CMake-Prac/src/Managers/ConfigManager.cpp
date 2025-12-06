#include <toml++/toml.hpp>

#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"

#include <format>

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
