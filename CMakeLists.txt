cmake_minimum_required(VERSION 3.10)
project(coreutils VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include/core-utils)
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error -Wno-format-truncation")
    # Debug-specific flags for GCC/Clang
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    # Release-specific flags for GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DRELEASE")
endif()

# Platform-specific libraries
if(WIN32)
    # Windows socket libraries
    set(PLATFORM_LIBS ws2_32 wsock32)
else()
    # Unix/Linux libraries
    find_package(Threads REQUIRED)
    set(PLATFORM_LIBS Threads::Threads)
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
    set_target_properties(${PROJECT_NAME}-shared PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    target_include_directories(${PROJECT_NAME}-shared PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(${PROJECT_NAME}-shared PUBLIC ${PLATFORM_LIBS})

    # Create alias for shared library
    add_library(${PROJECT_NAME}::shared ALIAS ${PROJECT_NAME}-shared)
    message(STATUS "Building shared library: lib${PROJECT_NAME}.so")
endif()

# Create a default alias (prefer static for tests)
if(BUILD_STATIC_LIBS)
    add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}-static)
else()
    add_library(${PROJECT_NAME} ALIAS ${PROJECT_NAME}-shared)
endif()

# Add subdirectories
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print build summary
message(STATUS "")
message(STATUS "========== Core Utils Build Configuration ==========")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build static library: ${BUILD_STATIC_LIBS}")
message(STATUS "Build shared library: ${BUILD_SHARED_LIBS}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "================================================")
message(STATUS "")