/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_COREUTILS_LOGGER_H
#define LMSHAO_COREUTILS_LOGGER_H

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace lmshao::coreutils {

/**
 * @brief Log levels
 */
enum class LogLevel {
    kDebug = 0,
    kInfo = 1,
    kWarn = 2,
    kError = 3,
    kFatal = 4
};

/**
 * @brief Log output types
 */
enum class LogOutput {
    CONSOLE = 0,
    FILE = 1,
    BOTH = 2
};

/**
 * @brief Logger class - no longer a template
 */
class Logger {
public:
    Logger(const std::string &module_name = "Unknown");
    ~Logger() = default;

    void SetOutput(LogOutput output) { output_ = output; }
    void SetLogFile(const std::string &filename) { log_file_ = filename; }
    void SetOutputFile(const std::string &filename) { log_file_ = filename; }
    void SetLevel(LogLevel level) { level_ = level; }
    void SetModuleName(const std::string &module) { module_name_ = module; }
    LogLevel GetLevel() const { return level_; }
    std::string GetModuleName() const { return module_name_; }

    void Log(LogLevel level, const char *file, int line, const char *func, const char *fmt, ...);

    template <typename ModuleTag>
    void LogWithModuleTag(LogLevel level, const char *file, int line, const char *func, const char *fmt, ...)
    {
        if (!ShouldLog(level)) {
            return;
        }

        va_list args;
        va_start(args, fmt);

        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), fmt, args);

        va_end(args);

        std::string time_str = GetTimeString();
        std::string level_str = GetLevelString(level);
        std::string filename = GetFileName(file);
        std::string module_name = LoggerRegistry::GetModuleName<ModuleTag>();

        std::ostringstream oss;
        oss << "[" << time_str << "] [" << level_str << "] [" << module_name << "] " << filename << ":" << line << " "
            << func << "() - " << buffer << std::endl;

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

    bool ShouldLog(LogLevel level) const { return level >= level_; }

private:
    std::string GetTimeString() const;
    std::string GetLevelString(LogLevel level) const;
    std::string GetColorCode(LogLevel level) const;
    std::string GetResetColor() const;
    std::string GetFileName(const char *file) const;

    LogLevel level_;
    LogOutput output_;
    std::string module_name_;
    std::string log_file_;
    mutable std::mutex mutex_;
};

/**
 * @brief Logger Registry - manages logger instances for different modules
 */
class LoggerRegistry {
public:
    // Dynamic module registration
    template <typename ModuleTag>
    static void RegisterModule(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(GetRegistryMutex());
        GetModuleNames()[std::type_index(typeid(ModuleTag))] = name;
    }

    template <typename ModuleTag>
    static Logger &GetLogger()
    {
        static std::once_flag flag;
        static Logger *logger = nullptr;

        std::call_once(flag, []() {
            std::string module_name = GetModuleName<ModuleTag>();
            logger = &GetOrCreateLogger(std::type_index(typeid(ModuleTag)), module_name);
        });

        return *logger;
    }

    template <typename ModuleTag>
    static Logger &GetLoggerWithRegistration(const std::string &module_name)
    {
        static std::once_flag flag;
        static Logger *logger = nullptr;

        std::call_once(flag, [module_name]() {
            RegisterModule<ModuleTag>(module_name);
            logger = &GetOrCreateLogger(std::type_index(typeid(ModuleTag)), module_name);
        });

        return *logger;
    }

    template <typename ModuleTag>
    static void InitLogger(LogLevel level =
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
                               LogLevel::kDebug,
#else
                               LogLevel::kWarn,
#endif
                           LogOutput output = LogOutput::CONSOLE, const std::string &filename = "")
    {
        auto &logger = GetLogger<ModuleTag>();
        logger.SetLevel(level);
        logger.SetOutput(output);
        if (!filename.empty()) {
            logger.SetOutputFile(filename);
        }
    }

    template <typename ModuleTag>
    static std::string GetModuleName()
    {
        std::lock_guard<std::mutex> lock(GetRegistryMutex());
        auto it = GetModuleNames().find(std::type_index(typeid(ModuleTag)));
        return it != GetModuleNames().end() ? it->second : "Unknown";
    }

private:
    static Logger &GetOrCreateLogger(std::type_index type_id, const std::string &module_name);

    static std::unordered_map<std::type_index, std::unique_ptr<Logger>> &GetLoggers();
    static std::unordered_map<std::type_index, std::string> &GetModuleNames();
    static std::mutex &GetRegistryMutex();
};

// Module tag for CoreUtils
struct CoreUtilsModuleTag {};

} // namespace lmshao::coreutils

// Forward declaration for NetworkModuleTag
namespace lmshao::coreutils {

// Type alias for CoreUtils logger - now uses Registry
using CoreUtilsLogger = Logger;

// Backward compatibility - get CoreUtils logger instance
inline Logger &GetCoreUtilsLogger()
{
    return LoggerRegistry::GetLogger<CoreUtilsModuleTag>();
}

// Convenience macros - now use Registry pattern
#define LOG_DEBUG(fmt, ...)                                                                                            \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLogger();                                                        \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kDebug)) {                                                   \
            logger.Log(lmshao::coreutils::LogLevel::kDebug, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);     \
        }                                                                                                              \
    } while (0)

#define LOG_INFO(fmt, ...)                                                                                             \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLogger();                                                        \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kInfo)) {                                                    \
            logger.Log(lmshao::coreutils::LogLevel::kInfo, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);      \
        }                                                                                                              \
    } while (0)

#define LOG_WARN(fmt, ...)                                                                                             \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLogger();                                                        \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kWarn)) {                                                    \
            logger.Log(lmshao::coreutils::LogLevel::kWarn, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);      \
        }                                                                                                              \
    } while (0)

#define LOG_ERROR(fmt, ...)                                                                                            \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLogger();                                                        \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kError)) {                                                   \
            logger.Log(lmshao::coreutils::LogLevel::kError, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);     \
        }                                                                                                              \
    } while (0)

#define LOG_FATAL(fmt, ...)                                                                                            \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLogger();                                                        \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kFatal)) {                                                   \
            logger.Log(lmshao::coreutils::LogLevel::kFatal, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);     \
        }                                                                                                              \
    } while (0)

// Initialize CoreUtils logger with smart defaults based on build type
inline void InitCoreUtilsLogger(LogLevel level =
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
                                    LogLevel::kDebug,
#else
                                    LogLevel::kWarn,
#endif
                                LogOutput output = LogOutput::CONSOLE, const std::string &filename = "")
{
    LoggerRegistry::InitLogger<CoreUtilsModuleTag>(level, output, filename);
}

// Auto-initialization version - ensures logger is initialized before use
inline Logger &GetCoreUtilsLoggerWithAutoInit()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        LoggerRegistry::RegisterModule<CoreUtilsModuleTag>("CoreUtils");
        InitCoreUtilsLogger();
    });
    return LoggerRegistry::GetLogger<CoreUtilsModuleTag>();
}

// Simplified macros for CoreUtils - with auto-initialization
#define COREUTILS_LOGD(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kDebug)) {                                                   \
            logger.LogWithModuleTag<lmshao::coreutils::CoreUtilsModuleTag>(                                            \
                lmshao::coreutils::LogLevel::kDebug, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);            \
        }                                                                                                              \
    } while (0)

#define COREUTILS_LOGI(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kInfo)) {                                                    \
            logger.LogWithModuleTag<lmshao::coreutils::CoreUtilsModuleTag>(                                            \
                lmshao::coreutils::LogLevel::kInfo, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);             \
        }                                                                                                              \
    } while (0)

#define COREUTILS_LOGW(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kWarn)) {                                                    \
            logger.LogWithModuleTag<lmshao::coreutils::CoreUtilsModuleTag>(                                            \
                lmshao::coreutils::LogLevel::kWarn, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);             \
        }                                                                                                              \
    } while (0)

#define COREUTILS_LOGE(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kError)) {                                                   \
            logger.LogWithModuleTag<lmshao::coreutils::CoreUtilsModuleTag>(                                            \
                lmshao::coreutils::LogLevel::kError, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);            \
        }                                                                                                              \
    } while (0)

#define COREUTILS_LOGF(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::coreutils::GetCoreUtilsLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::coreutils::LogLevel::kFatal)) {                                                   \
            logger.LogWithModuleTag<lmshao::coreutils::CoreUtilsModuleTag>(                                            \
                lmshao::coreutils::LogLevel::kFatal, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);            \
        }                                                                                                              \
    } while (0)

} // namespace lmshao::coreutils

#endif // LMSHAO_COREUTILS_LOGGER_H