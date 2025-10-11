cmake_minimum_required(VERSION 3.10)
project(lmcore VERSION 0.2.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for tools like clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" ON)
option(INSTALL_TO_USER_LOCAL "Install to ~/.local instead of system-wide" OFF)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Display configuration options for developers
message(STATUS "")
message(STATUS "========== Core Utils Configuration Options ==========")
message(STATUS "")
message(STATUS "Build Type Configuration:")
message(STATUS "  Current build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Available types: Debug, Release, RelWithDebInfo, MinSizeRel")
message(STATUS "  Change with: cmake -DCMAKE_BUILD_TYPE=Debug ..")
message(STATUS "")
message(STATUS "Library Build Options:")
message(STATUS "  BUILD_STATIC_LIBS: Build static libraries (current: ${BUILD_STATIC_LIBS})")
message(STATUS "  BUILD_SHARED_LIBS: Build shared libraries (current: ${BUILD_SHARED_LIBS})")
message(STATUS "  BUILD_TESTS: Build unit tests (current: ${BUILD_TESTS})")
message(STATUS "")
message(STATUS "Installation Options:")
message(STATUS "  CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  INSTALL_TO_USER_LOCAL: Install to ~/.local (current: ${INSTALL_TO_USER_LOCAL})")
message(STATUS "")
message(STATUS "Usage Examples:")
message(STATUS "  Debug build:     cmake -DCMAKE_BUILD_TYPE=Debug ..")
message(STATUS "  User install:    cmake -DINSTALL_TO_USER_LOCAL=ON ..")
message(STATUS "  Custom prefix:   cmake -DCMAKE_INSTALL_PREFIX=/opt/lmcore ..")
message(STATUS "  Tests only:      cmake -DBUILD_STATIC_LIBS=OFF -DBUILD_SHARED_LIBS=OFF ..")
message(STATUS "")
message(STATUS "========================================================")

# Handle user-local installation
if(INSTALL_TO_USER_LOCAL AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "User local install path" FORCE)
    message(STATUS "Installing to user directory: ${CMAKE_INSTALL_PREFIX}")
endif()

# Provide install path information to users
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Using default install prefix: ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "You can customize it with: cmake -DCMAKE_INSTALL_PREFIX=/your/path ..")
endif()

# Source files and include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

# Collect source files
file(GLOB SOURCE_FILES "src/*.cpp")

# Compiler flags
if(MSVC)
    # MSVC specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    # Debug-specific flags for MSVC
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Od /DDEBUG")
    # Release-specific flags for MSVC
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /DNDEBUG /DRELEASE")
else()
    # GCC/Clang flags for Linux/Unix
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-truncation")
    endif()
    # Debug-specific flags for GCC/Clang
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    # Release-specific flags for GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DRELEASE")
    
    # Enable position independent code for shared library compatibility
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Platform-specific libraries
if(WIN32)
    # Windows socket libraries
    set(PLATFORM_LIBS ws2_32 wsock32)
else()
    # Unix/Linux libraries - use pthread directly
    set(PLATFORM_LIBS pthread)
endif()

# Static library
if(BUILD_STATIC_LIBS)
    add_library(${PROJECT_NAME}-static STATIC ${SOURCE_FILES})
    set_target_properties(${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    target_include_directories(${PROJECT_NAME}-static PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(${PROJECT_NAME}-static PUBLIC ${PLATFORM_LIBS})
    
    # Create alias for static library
    add_library(${PROJECT_NAME}::static ALIAS ${PROJECT_NAME}-static)
    message(STATUS "Building static library: lib${PROJECT_NAME}.a")
endif()

# Shared library
if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME}-shared SHARED ${SOURCE_FILES})
    set_target_properties(${PROJECT_NAME}-shared PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    target_include_directories(${PROJECT_NAME}-shared PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(${PROJECT_NAME}-shared PUBLIC ${PLATFORM_LIBS})

    # Create alias for shared library
    add_library(${PROJECT_NAME}::shared ALIAS ${PROJECT_NAME}-shared)
    message(STATUS "Building shared library: lib${PROJECT_NAME}.so.${PROJECT_VERSION}")
endif()

# Create a default alias (prefer static for tests)
if(BUILD_STATIC_LIBS)
    add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}-static)
else()
    add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}-shared)
endif()

# Add subdirectories (only for development, not installed)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Add examples (only for development, not installed)
add_subdirectory(examples)

# Install and export functionality (Unix/Linux only)
if(NOT WIN32)
    # Install headers
    install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

    # Install libraries and create export targets
    set(INSTALL_TARGETS)
    if(BUILD_STATIC_LIBS)
        list(APPEND INSTALL_TARGETS ${PROJECT_NAME}-static)
    endif()
    if(BUILD_SHARED_LIBS)
        list(APPEND INSTALL_TARGETS ${PROJECT_NAME}-shared)
    endif()

    if(INSTALL_TARGETS)
        install(TARGETS ${INSTALL_TARGETS}
            EXPORT lmcoreTargets
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
        )
    endif()

    # Export targets for find_package
    install(EXPORT lmcoreTargets
        FILE lmcoreTargets.cmake
        NAMESPACE lmcore::
        DESTINATION lib/cmake/lmcore
    )

    # Create and install config files
    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lmcoreConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/lmcoreConfig.cmake"
        INSTALL_DESTINATION lib/cmake/lmcore
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/lmcoreConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lmcoreConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/lmcoreConfigVersion.cmake"
        DESTINATION lib/cmake/lmcore
    )

    # Add uninstall target
    if(NOT TARGET uninstall)
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)

        add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
    endif()
else()
    message(STATUS "Install/uninstall targets are disabled on Windows platform")
    message(STATUS "Use the library directly from the build directory")
endif()

# Print build summary
message(STATUS "")
message(STATUS "========== Core Utils Build Configuration ==========")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build static library: ${BUILD_STATIC_LIBS}")
message(STATUS "Build shared library: ${BUILD_SHARED_LIBS}")
message(STATUS "Build tests: ${BUILD_TESTS}")
if(NOT WIN32)
    message(STATUS "Install support: ON")
    message(STATUS "Uninstall support: ON (make uninstall)")
else()
    message(STATUS "Install support: DISABLED (Windows platform)")
    message(STATUS "Uninstall support: DISABLED (Windows platform)")
endif()
message(STATUS "================================================")
message(STATUS "")