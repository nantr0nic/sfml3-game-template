#pragma once

#include <print>
#include <source_location>
#include <string_view>
#include <cstdio>
#include <format>

namespace logger
{
    enum class LogLevel 
    {
        Info,       // We'll use Green
        Warning,    // We'll use yellow
        Error,      // We'll use red
        None
    };
    namespace Color
    {
        constexpr std::string_view Reset = "\033[0m"; // Use for in-message reset
        constexpr std::string_view Red = "\033[31m";
        constexpr std::string_view Green = "\033[32m";
        constexpr std::string_view Yellow = "\033[33m";
        constexpr std::string_view Blue = "\033[34m";
        constexpr std::string_view Magenta = "\033[35m";
        constexpr std::string_view Cyan = "\033[36m";
        constexpr std::string_view White = "\033[37m";
    }

    // For release builds, only print Error logs
    #ifdef NDEBUG
        inline LogLevel currentLevel = LogLevel::Error;
    #else
    // For non-release builds, we'll print Warning and Info logs too
        inline LogLevel currentLevel = LogLevel::Info;
    #endif

    inline void setLevel(LogLevel level)
    {
        currentLevel = level;
    }

    inline void forceVerbose()
    {
        currentLevel = LogLevel::Info;
    }

    // format file path to just the filename instead of printing the absolute path
    constexpr std::string_view formatPath(std::string_view path)
    {
        auto lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string_view::npos)
        {
            return path.substr(lastSlash + 1);
        }

        return path;
    }

    inline void Print(LogLevel level, std::string_view message, const std::source_location& loc)
    {
        // Only print current level and above
        if (level < currentLevel)
        {
            return;
        }

        std::string_view colorStr{};
        std::string_view levelStr{};

        if (level == LogLevel::Error)
        {
            colorStr = Color::Red;
            levelStr = "ERROR";
        }
        else if (level == LogLevel::Warning)
        {
            colorStr = Color::Yellow;
            levelStr = "WARNING";
        }
        else if (level == LogLevel::Info)
        {
            colorStr = Color::Green;
            levelStr = "INFO";
        }
        else
        {
            //something went wrong
            return;
        }

        // Select appropriate stream (stderr for error, stdout for others)
        FILE* stream = (level == LogLevel::Error) ? stderr : stdout;

        // Prepare the format string
        constexpr auto fmtString = "[[{}{}{}]] {}({}:{}) --> {}{}{}";
        // [[Error]] file: file_name(line:column) 'function_name' --> message
        std::println(stream, fmtString, 
            colorStr, 
            levelStr, 
            Color::Reset, 
            formatPath(loc.file_name()), 
            loc.line(), 
            loc.column(), 
            //loc.function_name(), // too verbose but will leave here for debug 
            colorStr, 
            message,
            Color::Reset
        );
    }

    inline void Info(std::string_view message, 
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Info, message, loc);
    }

    inline void Warn(std::string_view message,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Warning, message, loc);
    }

    inline void Error(std::string_view message,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Error, message, loc);
    }

    // -------------------------------------------------------------------- //

    //$ This is a more 'elegant' solution where std::format() is not required in logging
    //$ messages if including parameters in the message, but it is difficult for me to
    //$ understand so I'm going to use the simpler non-variadic-template functions and
    //$ just be okay with using std::format() when needed :)

    //$ For example: if you want to print the Info message "Player health: 80" then the 
    //$ above functions require logger::Info(std::format("Player health: {}", player.health));
    //$ Or, logger::Error(std::format("{}", e.what());

    //$ Whereas, these variadic templates enable a simpler interface:
    //$ logger::Info("Player health: {}", player.health);
    //$ logger::Error("{}", e.what());
    /*
    // Public Helpers (Templates)
    template<typename... Args>
    void Info(std::format_string<Args...> fmt, Args&&... args,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...), loc);
    }

    template<typename... Args>
    void Warn(std::format_string<Args...> fmt, Args&&... args,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...), loc);
    }

    template<typename... Args>
    void Error(std::format_string<Args...> fmt, Args&&... args,
        const std::source_location& loc = std::source_location::current())
    {
        Print(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...), loc);
    }
    */
}