#include "Logging.h"
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger::Logger() {
    // Check if logging is enabled
    const char* logging_env = std::getenv("DFM_LOGGING");
    if (logging_env) {
        std::string logging_str = logging_env;
        std::transform(logging_str.begin(), logging_str.end(), logging_str.begin(), ::tolower);
        logging_enabled_ = (logging_str == "1" || logging_str == "true");
    } else {
        logging_enabled_ = false;
    }

    // Parse log level
    const char* log_level_env = std::getenv("DFM_LOG_LEVEL");
    if (log_level_env) {
        log_level_ = parseLogLevel(log_level_env);
    } else {
        log_level_ = LogLevel::INFO; // Default
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARN: return "WARN";
        case LogLevel::INFO: return "INFO";
        case LogLevel::LOG_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

LogLevel Logger::parseLogLevel(const std::string& level_str) const {
    std::string lower_str = level_str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    
    if (lower_str == "error" || lower_str == "0") return LogLevel::ERROR;
    if (lower_str == "warn" || lower_str == "1") return LogLevel::WARN;
    if (lower_str == "info" || lower_str == "2") return LogLevel::INFO;
    if (lower_str == "debug" || lower_str == "3") return LogLevel::LOG_DEBUG;
    
    return LogLevel::INFO; // Default if invalid
}

void Logger::log(LogLevel level, const std::string& message) const {
    if (!logging_enabled_ || level > log_level_) return;

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count()
        << " [" << levelToString(level) << "] " << message;

    std::lock_guard<std::mutex> lock(mtx_);
    std::cout << oss.str() << std::endl;
}
