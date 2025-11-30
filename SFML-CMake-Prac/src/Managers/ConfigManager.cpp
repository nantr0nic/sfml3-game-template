#define TOML_IMPLEMENTATION // This is the ONE place/file where we will define this!
#include <toml++/toml.hpp>

#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"

#include <format>

void ConfigManager::loadConfig(std::string_view configID, std::string_view filepath)
{
    try
    {
        toml::table newConfig = toml::parse_file(filepath);
        m_ConfigFiles.insert_or_assign(std::string(configID), std::move(newConfig));

        logger::Info(std::format("Config ID \"{}\" loaded from: {}", configID, filepath));
    }
    catch (const toml::parse_error& e)
    {
        logger::Error(std::format("Error parsing config file: {} ", e.description()));
    }
    catch (const std::exception& e)
    {
        logger::Error(std::format("Error loading config file: {}", e.what()));
    }
}
