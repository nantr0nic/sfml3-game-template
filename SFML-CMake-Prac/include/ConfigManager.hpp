#include <toml.hpp>

#include <print>
#include <optional>

class ConfigManager
{
public:
    ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ~ConfigManager() = default;

    void loadConfig(const std::string& filepath);

    template<typename T>
    std::optional<T> getConfigValue(const std::string& key);

    template<typename T>
    std::optional<T> getConfigValueAt(const std::string& at, const std::string& key);


private:
    toml::value m_ConfigFile;

};

inline void ConfigManager::loadConfig(const std::string& filepath)
{
    try
    {
        m_ConfigFile = toml::parse(filepath);
        std::println("Config file loaded successfully: {}", filepath);
    }
    catch (const std::exception& e)
    {
        std::println("Error loading config file: {}", e.what());
    }
}

template<typename T>
std::optional<T> ConfigManager::getConfigValue(const std::string& key)
{
    try
    {
        return toml::find<T>(m_ConfigFile, key);
    }
    catch (const std::exception& e)
    {
        std::println("Error getting config key [{}]: {}", key, e.what());
        return std::nullopt;
    }
}

template<typename T>
std::optional<T> ConfigManager::getConfigValueAt(const std::string& at, const std::string& key)
{
    try
    {
        return toml::find<T>(m_ConfigFile, at, key);
    }
    catch (const std::exception& e)
    {
        std::println("Error getting config key [{}]: {}", key, e.what());
        return std::nullopt;
    }
}