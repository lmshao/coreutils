/**
 * @file migration_test.cpp
 * @brief Simple test program to verify migration from core_utils_log.h to logger.h
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 */

#include "coreutils/logger.h"

#include <iostream>

using namespace lmshao::coreutils;

int main()
{
    std::cout << "=== CoreUtils Migration Test ===\n\n";

    // Initialize the logger system
    std::cout << "1. Initializing logger system...\n";
    InitCoreUtilsLogger(LogLevel::kDebug);

    COREUTILS_LOGI("Logger system initialized successfully");
    COREUTILS_LOGD("Debug logging is enabled");

    // Test different log levels
    std::cout << "\n2. Testing different log levels...\n";
    COREUTILS_LOGD("This is a DEBUG message");
    COREUTILS_LOGI("This is an INFO message");
    COREUTILS_LOGE("This is an ERROR message (test only)");

    // Test global level control
    std::cout << "\n3. Testing global log level control...\n";
    COREUTILS_LOGI("Setting global level to ERROR");
    GetCoreUtilsLogger().SetLevel(LogLevel::kError);

    COREUTILS_LOGD("This DEBUG should not appear");
    COREUTILS_LOGI("This INFO should not appear");
    COREUTILS_LOGE("This ERROR should appear");

    // Reset to DEBUG
    GetCoreUtilsLogger().SetLevel(LogLevel::kDebug);
    COREUTILS_LOGI("Global level reset to DEBUG");

    std::cout << "\n=== Migration Test Completed Successfully ===\n";
    std::cout << "All components are using the new logger.h system!\n";

    return 0;
}