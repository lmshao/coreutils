/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_LMCORE_INTERNAL_LOGGER_H
#define LMSHAO_LMCORE_INTERNAL_LOGGER_H

#include "lmcore/lmcore_logger.h"

namespace lmshao::lmcore {

/**
 * @brief Get LmCore logger with automatic initialization
 * This version ensures the logger is properly initialized before use.
 * Used internally by LmCore modules.
 */
inline Logger &GetLmCoreLoggerWithAutoInit()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        LoggerRegistry::RegisterModule<LmCoreModuleTag>("LmCore");
        InitLmCoreLogger();
    });
    return LoggerRegistry::GetLogger<LmCoreModuleTag>();
}

// Internal LmCore logging macros with auto-initialization and module tagging
#define LMCORE_LOGD(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::lmcore::GetLmCoreLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::lmcore::LogLevel::kDebug)) {                                                   \
            logger.LogWithModuleTag<lmshao::lmcore::LmCoreModuleTag>(                                            \
                lmshao::lmcore::LogLevel::kDebug, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);            \
        }                                                                                                              \
    } while (0)

#define LMCORE_LOGI(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::lmcore::GetLmCoreLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::lmcore::LogLevel::kInfo)) {                                                    \
            logger.LogWithModuleTag<lmshao::lmcore::LmCoreModuleTag>(                                            \
                lmshao::lmcore::LogLevel::kInfo, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);             \
        }                                                                                                              \
    } while (0)

#define LMCORE_LOGW(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::lmcore::GetLmCoreLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::lmcore::LogLevel::kWarn)) {                                                    \
            logger.LogWithModuleTag<lmshao::lmcore::LmCoreModuleTag>(                                            \
                lmshao::lmcore::LogLevel::kWarn, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);             \
        }                                                                                                              \
    } while (0)

#define LMCORE_LOGE(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::lmcore::GetLmCoreLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::lmcore::LogLevel::kError)) {                                                   \
            logger.LogWithModuleTag<lmshao::lmcore::LmCoreModuleTag>(                                            \
                lmshao::lmcore::LogLevel::kError, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);            \
        }                                                                                                              \
    } while (0)

#define LMCORE_LOGF(fmt, ...)                                                                                       \
    do {                                                                                                               \
        auto &logger = lmshao::lmcore::GetLmCoreLoggerWithAutoInit();                                            \
        if (logger.ShouldLog(lmshao::lmcore::LogLevel::kFatal)) {                                                   \
            logger.LogWithModuleTag<lmshao::lmcore::LmCoreModuleTag>(                                            \
                lmshao::lmcore::LogLevel::kFatal, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);            \
        }                                                                                                              \
    } while (0)

} // namespace lmshao::lmcore

#endif // LMSHAO_LMCORE_INTERNAL_LOGGER_H
