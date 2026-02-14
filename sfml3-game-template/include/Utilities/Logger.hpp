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
    /**
     * @brief Extracts the filename component from a filesystem path.
     *
     * Finds the last path separator ('/' or '\') and returns the substring after it.
     *
     * @param path Path string to extract the filename from.
     * @return std::string_view Filename portion of `path` (text after the last '/' or '\'), or the original `path` if no separator is present.
     */
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
            /**
             * @brief Initializes and starts the asynchronous log worker.
             *
             * Constructs the LogWorker, launching its background thread to run processLogs().
             * When LOG_TO_FILE is enabled, attempts to create a "logs" directory, generates a
             * timestamped log filename, and opens the file in append mode. If directory creation
             * fails, an error is printed to stderr and a timestamped file is created in the
             * process root instead.
             */
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
            /**
             * @brief Stops the log worker, waits for the background thread to finish, and releases resources.
             *
             * Signals the worker to stop, notifies the processing thread so it can exit, joins the worker thread,
             * and when file logging is enabled flushes and closes the log file.
             */
            ~LogWorker() {
                stopFlag = true;
                logC_V.notify_all();
                logWorker.join();
                #if LOG_TO_FILE
                logFile.flush();
                logFile.close();
                #endif
            }

            /**
             * @brief Enqueues a log entry for asynchronous processing by the background log worker.
             *
             * This call is thread-safe and notifies the worker thread to process the entry.
             *
             * @param entry LogEntry to enqueue; ownership is transferred into the worker.
             *
             * @note If the worker is stopping, the entry will be discarded and not processed.
             */
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
            /**
             * @brief Processes queued log entries in the worker thread and emits them to console and optional file.
             *
             * Continuously consumes LogEntry items from the internal queue until the worker is signaled to stop
             * and the queue is empty. Each entry is formatted with level, source file (filename only), line,
             * column, and message; console output is colorized and sent to stderr for errors or stdout for other levels.
             * When file logging is enabled and the file is open, a non-colored log line is written to the file.
             * The log file is flushed for error entries and once more when the processing loop exits.
             */
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

        /**
         * @brief Provides access to the shared log worker used for asynchronous logging.
         *
         * This function returns the single, long-lived LogWorker instance used by the logger to
         * enqueue and process log entries in the background.
         *
         * @return LogWorker& Reference to the shared LogWorker instance (lives for the program's lifetime).
         */
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

    /**
     * @brief Sets the current minimum log level used to filter emitted messages.
     *
     * Adjusts which log messages are allowed to be enqueued and processed: messages
     * with a level lower than `level` will be suppressed.
     *
     * @param level The minimum LogLevel to allow (messages with lower severity are ignored).
     */
    inline void setLevel(LogLevel level)
    {
        currentLevel.store(level, std::memory_order_relaxed);
    }

    /**
     * @brief Set the logger to verbose mode by enabling the Info log level.
     *
     * Updates the global log level so that messages at `LogLevel::Info` and above are emitted.
     */
    inline void forceVerbose()
    {
        currentLevel.store(LogLevel::Info, std::memory_order_relaxed);
    }

    /**
     * @brief Enqueues a log entry for background processing if it meets the current log level.
     *
     * If `level` is lower than the configured current log level, the call has no effect.
     *
     * @param level Log severity level for the message.
     * @param message The textual message to log.
     * @param loc Source location associated with the message (file/line/column); defaults to the caller's location when provided by the caller.
     */
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

    /**
     * @brief Emit an informational log entry.
     *
     * @param message The text of the message to log.
     * @param loc Source location to associate with the log entry; defaults to the call site.
     */
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

    /**
     * @brief Log a message at the Error level including source location information.
     *
     * @param message The text to record in the log.
     * @param loc Source location to associate with the message; defaults to the caller's location.
     */
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