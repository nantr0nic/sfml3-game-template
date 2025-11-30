#include "Managers/ConfigManager.hpp"
#include "Utilities/Logger.hpp"

#include <format>

void ConfigManager::loadConfig(std::string_view configID, std::string_view filepath)
{
    try
    {
        //! This defeats the purpose of string_view. Double check toml11 to see if
        //! it can handle it. If not, then just use std::string in arguments.
        toml::value newConfig = toml::parse(std::string(filepath));
        m_ConfigFiles.insert_or_assign(std::string(configID), std::move(newConfig));

        logger::Info(std::format("Config ID \"{}\" loaded from: {}", configID, filepath));
    }
    catch (const toml::syntax_error& e)
    {
        logger::Error(std::format("TOML syntax error in {}: {}", filepath, e.what()));
    }
    catch (const toml::file_io_error& e)
    {
        logger::Error(std::format("Failed to open config file {}: {}", filepath, e.what()));
    }
    catch (const std::exception& e)
    {
        logger::Error(std::format("Error loading config file:", e.what()));
    }
}