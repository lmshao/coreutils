/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_COREUTILS_INTERNAL_LOGGER_H
#define LMSHAO_COREUTILS_INTERNAL_LOGGER_H

#include "coreutils/coreutils_logger.h"

namespace lmshao::coreutils {

/**
 * @brief Get CoreUtils logger with automatic initialization
 * This version ensures the logger is properly initialized before use.
 * Used internally by CoreUtils modules.
 */
inline Logger &GetCoreUtilsLoggerWithAutoInit()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        LoggerRegistry::RegisterModule<CoreUtilsModuleTag>("CoreUtils");
        InitCoreUtilsLogger();
    });
    return LoggerRegistry::GetLogger<CoreUtilsModuleTag>();
}

// Internal CoreUtils logging macros with auto-initialization and module tagging
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

#endif // LMSHAO_COREUTILS_INTERNAL_LOGGER_H
