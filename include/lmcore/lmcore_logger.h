/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_LMCORE_LMCORE_LOGGER_H
#define LMSHAO_LMCORE_LMCORE_LOGGER_H

#include "logger.h"

namespace lmshao::lmcore {

// Module tag for LmCore
struct LmCoreModuleTag {};

/**
 * @brief Initialize LmCore logger with specified settings
 * @param level Log level (default: Debug in debug builds, Warn in release builds)
 * @param output Output destination (default: CONSOLE)
 * @param filename Log file name (optional)
 */
inline void InitLmCoreLogger(LogLevel level =
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
                                 LogLevel::kDebug,
#else
                                 LogLevel::kWarn,
#endif
                             LogOutput output = LogOutput::CONSOLE, const std::string &filename = "")
{
    // Register module if not already registered
    LoggerRegistry::RegisterModule<LmCoreModuleTag>("LmCore");
    LoggerRegistry::InitLogger<LmCoreModuleTag>(level, output, filename);
}

} // namespace lmshao::lmcore

#endif // LMSHAO_LMCORE_LMCORE_LOGGER_H
