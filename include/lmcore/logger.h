/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_LMCORE_LOGGER_H
#define LMSHAO_LMCORE_LOGGER_H

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

namespace lmshao::lmcore {

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
    static class Logger &GetLogger();

    template <typename ModuleTag>
    static class Logger &GetLoggerWithRegistration(const std::string &module_name);

    template <typename ModuleTag>
    static void InitLogger(LogLevel level = LogLevel::kWarn, LogOutput output = LogOutput::CONSOLE,
                           const std::string &filename = "");

    template <typename ModuleTag>
    static std::string GetModuleName()
    {
        std::lock_guard<std::mutex> lock(GetRegistryMutex());
        auto it = GetModuleNames().find(std::type_index(typeid(ModuleTag)));
        return it != GetModuleNames().end() ? it->second : "Unknown";
    }

private:
    static class Logger &GetOrCreateLogger(std::type_index type_id, const std::string &module_name);

    static std::unordered_map<std::type_index, std::unique_ptr<class Logger>> &GetLoggers();
    static std::unordered_map<std::type_index, std::string> &GetModuleNames();
    static std::mutex &GetRegistryMutex();
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

// Template method implementations for LoggerRegistry
template <typename ModuleTag>
class Logger &LoggerRegistry::GetLogger()
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
class Logger &LoggerRegistry::GetLoggerWithRegistration(const std::string &module_name)
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
void LoggerRegistry::InitLogger(LogLevel level, LogOutput output, const std::string &filename)
{
    auto &logger = GetLogger<ModuleTag>();
    logger.SetLevel(level);
    logger.SetOutput(output);
    if (!filename.empty()) {
        logger.SetOutputFile(filename);
    }
}

} // namespace lmshao::lmcore

#endif // LMSHAO_LMCORE_LOGGER_H
