/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#include "internal_logger.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace lmshao::coreutils {

// Global log level
static LogLevel global_level_ = LogLevel::kInfo;

// Logger implementation
Logger::Logger(const std::string &module_name)
    : module_name_(module_name), level_(LogLevel::kInfo), output_(LogOutput::CONSOLE)
{
}

void Logger::Log(LogLevel level, const char *file, int line, const char *function, const char *format, ...)
{
    if (!ShouldLog(level)) {
        return;
    }

    va_list args;
    va_start(args, format);

    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    std::string time_str = GetTimeString();
    std::string level_str = GetLevelString(level);
    std::string filename = GetFileName(file);

    std::ostringstream oss;
    oss << "[" << time_str << "] [" << level_str << "] [" << module_name_ << "] " << filename << ":" << line << " "
        << function << "() - " << buffer << std::endl;

    std::string log_message = oss.str();

    if (output_ == LogOutput::CONSOLE || output_ == LogOutput::BOTH) {
        std::cout << log_message;
        std::cout.flush();
    }

    if ((output_ == LogOutput::FILE || output_ == LogOutput::BOTH) && !log_file_.empty()) {
#ifdef _WIN32
        FILE *fp = nullptr;
        errno_t err = fopen_s(&fp, log_file_.c_str(), "a");
        if (err == 0 && fp) {
            fprintf(fp, "%s", log_message.c_str());
            fclose(fp);
        }
#else
        FILE *fp = fopen(log_file_.c_str(), "a");
        if (fp) {
            fprintf(fp, "%s", log_message.c_str());
            fclose(fp);
        }
#endif
    }
}

std::string Logger::GetTimeString() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
#ifdef _WIN32
    struct tm tm_buf;
    errno_t err = localtime_s(&tm_buf, &time_t);
    if (err == 0) {
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    }
#else
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
#endif
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::GetLevelString(LogLevel level) const
{
    static const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    int index = static_cast<int>(level);
    if (index >= 0 && index < 5) {
        return std::string(level_strings[index]);
    }
    return std::string("UNKNOWN");
}

std::string Logger::GetFileName(const char *filepath) const
{
    std::string path(filepath);
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

std::string Logger::GetColorCode(LogLevel level) const
{
#ifdef _WIN32
    return ""; // Windows console colors handled differently
#else
    switch (level) {
        case LogLevel::kDebug:
            return "\033[36m"; // Cyan
        case LogLevel::kInfo:
            return "\033[32m"; // Green
        case LogLevel::kWarn:
            return "\033[33m"; // Yellow
        case LogLevel::kError:
            return "\033[31m"; // Red
        case LogLevel::kFatal:
            return "\033[35m"; // Magenta
        default:
            return "";
    }
#endif
}

std::string Logger::GetResetColor() const
{
#ifdef _WIN32
    return "";
#else
    return "\033[0m";
#endif
}

// LoggerRegistry implementation - using Meyers Singleton pattern for safe initialization
std::unordered_map<std::type_index, std::unique_ptr<Logger>> &LoggerRegistry::GetLoggers()
{
    static std::unordered_map<std::type_index, std::unique_ptr<Logger>> loggers;
    return loggers;
}

std::unordered_map<std::type_index, std::string> &LoggerRegistry::GetModuleNames()
{
    static std::unordered_map<std::type_index, std::string> module_names;
    return module_names;
}

std::mutex &LoggerRegistry::GetRegistryMutex()
{
    static std::mutex registry_mutex;
    return registry_mutex;
}

Logger &LoggerRegistry::GetOrCreateLogger(std::type_index type_id, const std::string &module_name)
{
    std::lock_guard<std::mutex> lock(GetRegistryMutex());
    auto it = GetLoggers().find(type_id);
    if (it == GetLoggers().end()) {
        auto logger = std::make_unique<Logger>(module_name);
        Logger *logger_ptr = logger.get();
        GetLoggers()[type_id] = std::move(logger);
        return *logger_ptr;
    }
    return *it->second;
}

// Global functions
void SetGlobalLogLevel(LogLevel level)
{
    global_level_ = level;
}

LogLevel GetGlobalLogLevel()
{
    return global_level_;
}

} // namespace lmshao::coreutils
