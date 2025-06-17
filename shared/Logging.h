#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <iostream>
#include <mutex>

// Logging verbosity levels
enum class LogLevel {
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    LOG_DEBUG = 3
};

class Logger {
public:
    static Logger& getInstance();
    void log(LogLevel level, const std::string& message) const;
    bool isLoggingEnabled() const { return logging_enabled_; }
    LogLevel getLogLevel() const { return log_level_; }

private:
    Logger();
    bool logging_enabled_;
    LogLevel log_level_;
    mutable std::mutex mtx_;
    std::string levelToString(LogLevel level) const;
    LogLevel parseLogLevel(const std::string& level_str) const;
};

// Logging macros
#define LOG_ERROR(message) \
    do { \
        if (Logger::getInstance().isLoggingEnabled() && \
            Logger::getInstance().getLogLevel() >= LogLevel::ERROR) { \
            Logger::getInstance().log(LogLevel::ERROR, message); \
        } \
    } while (0)

#define LOG_WARN(message) \
    do { \
        if (Logger::getInstance().isLoggingEnabled() && \
            Logger::getInstance().getLogLevel() >= LogLevel::WARN) { \
            Logger::getInstance().log(LogLevel::WARN, message); \
        } \
    } while (0)

#define LOG_INFO(message) \
    do { \
        if (Logger::getInstance().isLoggingEnabled() && \
            Logger::getInstance().getLogLevel() >= LogLevel::INFO) { \
            Logger::getInstance().log(LogLevel::INFO, message); \
        } \
    } while (0)

#define LOG_DEBUG(message) \
    do { \
        if (Logger::getInstance().isLoggingEnabled() && \
            Logger::getInstance().getLogLevel() >= LogLevel::LOG_DEBUG) { \
            Logger::getInstance().log(LogLevel::LOG_DEBUG, message); \
        } \
    } while (0);

#define LOG_FUNCTION() \
    LOG_DEBUG("Entering " + std::string(__FUNCTION__))

#endif // LOGGING_H
