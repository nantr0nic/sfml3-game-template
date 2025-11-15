#include "ConfigManager.hpp"

#include <format>
#include <print>
#include <iostream>

void ConfigManager::loadConfig(std::string_view configID, std::string_view filepath)
{
    try
    {
        toml::value newConfig = toml::parse(std::string(filepath));
        m_ConfigFiles.insert_or_assign(std::string(configID), std::move(newConfig));
        std::println("Config file loaded successfully: {}", filepath);
    }
    catch (const std::exception& e)
    {
        std::println(std::cerr, "Error loading config file: {}", e.what());
    }
}