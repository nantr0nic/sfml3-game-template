#pragma once

#ifndef LOG_TO_FILE
/* The line below can be uncommented to enable logging to file. This will override
the CMakeLists.txt option */

    // #define LOG_TO_FILE 1
#endif

#include <print>
#include <source_location>
#include <string_view>
#include <string>
#include <cstdio>
#include <format>
#include <utility>
#include <atomic>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

#if LOG_TO_FILE
    #include <fstream>
    #include <chrono>
    #include <filesystem>
#endif


namespace logger
{
    enum class LogLevel
    {
        Info,       // We'll use Green
        Warning,    // We'll use Yellow
        Error,      // We'll use Red
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
    struct LogEntry
    {
        std::string message;
        std::source_location location;
        LogLevel level;
    };
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

    // the detail namespace is for the logger's async worker
    namespace detail
    {
        class LogWorker
        {
        public:
            LogWorker() : logWorker(&LogWorker::processLogs, this) {
                #if LOG_TO_FILE
                std::error_code errorCode;
                std::filesystem::create_directories("logs", errorCode);

                auto now = std::chrono::system_clock::now();
                auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
                auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), now_sec };

                std::string fileName{};

                if (!errorCode)
                {
                    fileName = std::format("logs/{:%Y-%m-%d_%H-%M-%S}.log", local_time);
                }
                else
                {
                    std::println(stderr, "[[ERROR]] Failed to create logs directory: {}",
                        errorCode.message());
                    std::println(stderr, "[[ERROR]] Logger will log to file in root directory.");

                    fileName = std::format("{:%Y-%m-%d_%H-%M-%S}.log", local_time);
                }

                logFile.open(fileName, std::ios::app);
                #endif
            }
            ~LogWorker() {
                stopFlag = true;
                logC_V.notify_all();
                logWorker.join();
                #if LOG_TO_FILE
                logFile.flush();
                logFile.close();
                #endif
            }

            void push(LogEntry entry) {
                if (stopFlag.load(std::memory_order_acquire))
                {
                    return;
                }
                {
                    std::scoped_lock lock(logMutex);
                    logQueue.push(std::move(entry));
                }
                logC_V.notify_one();
            }

        private:
            void processLogs() {
                while (true)
                {
                    LogEntry entry{};
                    {
                        std::unique_lock<std::mutex> lock(logMutex);
                        logC_V.wait(lock, [this](){ return stopFlag || !logQueue.empty(); });
                        if (stopFlag && logQueue.empty())
                        {
                            break;
                        }
                        if (!logQueue.empty())
                        {
                            entry = std::move(logQueue.front());
                            logQueue.pop();
                        }
                    }

                    std::string_view colorStr{};
                    std::string_view levelStr{};

                    switch (entry.level)
                    {
                        case LogLevel::Error:
                            colorStr = Color::Red;
                            levelStr = "ERROR";
                            break;
                        case LogLevel::Warning:
                            colorStr = Color::Yellow;
                            levelStr = "WARNING";
                            break;
                        case LogLevel::Info:
                            colorStr = Color::Green;
                            levelStr = "INFO";
                            break;
                        default:
                            // something went wrong
                            colorStr = Color::White;
                            levelStr = "UNKNOWN";
                            break;
                    }

                    // Select appropriate stream (stderr for error, stdout for others)
                    FILE* stream = (entry.level == LogLevel::Error) ? stderr : stdout;

                    // Prepare the format string
                    constexpr auto fmtString = "[[{}{}{}]] {}({}:{}) --> {}{}{}";
                    // [[Error]] file: file_name(line:column) 'function_name' --> message
                    std::println(stream, fmtString,
                        colorStr,
                        levelStr,
                        Color::Reset,
                        formatPath(entry.location.file_name()),
                        entry.location.line(),
                        entry.location.column(),
                        //loc.function_name(), // too verbose but will leave here for debug
                        colorStr,
                        entry.message,
                        Color::Reset
                    );

                    #if LOG_TO_FILE
                    if (logFile.is_open())
                    {
                        std::println(logFile, "[[{}]] {}({}:{}) -> {}",
                            levelStr,
                            formatPath(entry.location.file_name()),
                            entry.location.line(),
                            entry.location.column(),
                            entry.message);

                        if (entry.level == LogLevel::Error) { logFile.flush(); }
                    }
                    #endif
                }

                #if LOG_TO_FILE
                logFile.flush();
                #endif
            }

        private:
            std::queue<LogEntry> logQueue;
            std::atomic<bool> stopFlag{ false };
            #if LOG_TO_FILE
                std::ofstream logFile;
            #endif
            std::mutex logMutex;
            std::condition_variable logC_V;
            std::jthread logWorker;
        };

        inline LogWorker& getWorker()
        {
            static LogWorker worker;
            return worker;
        }
    }

    // Option for modifying log level
    #ifdef NDEBUG
        // uncomment the line beneath to restrict release builds to print only Error logs
        // inline std::atomic<LogLevel> currentLevel = LogLevel::Error;
        // if you uncomment the line above, then comment the line below
        inline std::atomic<LogLevel> currentLevel = LogLevel::Info;
    #else
    // For non-release builds, we'll print Warning and Info logs too
        inline std::atomic<LogLevel> currentLevel = LogLevel::Info;
    #endif

    inline void setLevel(LogLevel level)
    {
        currentLevel.store(level, std::memory_order_relaxed);
    }

    inline void forceVerbose()
    {
        currentLevel.store(LogLevel::Info, std::memory_order_relaxed);
    }

    inline void Print(LogLevel level, std::string_view message, const std::source_location& loc)
    {
        // Only print current level and above
        if (level < currentLevel.load(std::memory_order_relaxed))
        {
            return;
        }

        LogEntry entry {
            .message = std::string(message),
            .location = loc,
            .level = level
        };

        detail::getWorker().push(std::move(entry));
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
}


    // -------------------------------------------------------------------- //

    //$ This is a more 'elegant' solution where std::format() is not required in logging
    //$ messages if including parameters in the message, but it is difficult for me to
    //$ understand so I'm going to use the simpler non-variadic-template functions and
    //$ just be okay with using std::format() when needed for now :)

    //$ For example: if you want to print the Info message "Player health: 80" then the
    //$ above functions require logger::Info(std::format("Player health: {}", player.health));
    //$ Or, logger::Error(std::format("{}", e.what());

    //$ Whereas, these variadic templates enable a simpler interface:
    //$ logger::Info("Player health: {}", player.health);
    //$ logger::Error("{}", e.what());

    //! The variadic templates beneath haven't been tested with the asynchronous logger

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
