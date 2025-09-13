/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_COREUTILS_COREUTILS_LOGGER_H
#define LMSHAO_COREUTILS_COREUTILS_LOGGER_H

#include "logger.h"

namespace lmshao::coreutils {

// Module tag for CoreUtils
struct CoreUtilsModuleTag {};

/**
 * @brief Initialize CoreUtils logger with specified settings
 * @param level Log level (default: Debug in debug builds, Warn in release builds)
 * @param output Output destination (default: CONSOLE)
 * @param filename Log file name (optional)
 */
inline void InitCoreUtilsLogger(LogLevel level =
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
                                    LogLevel::kDebug,
#else
                                    LogLevel::kWarn,
#endif
                                LogOutput output = LogOutput::CONSOLE, const std::string &filename = "")
{
    // Register module if not already registered
    LoggerRegistry::RegisterModule<CoreUtilsModuleTag>("CoreUtils");
    LoggerRegistry::InitLogger<CoreUtilsModuleTag>(level, output, filename);
}

} // namespace lmshao::coreutils

#endif // LMSHAO_COREUTILS_COREUTILS_LOGGER_H
