# Examples for core-utils library

# AsyncTimer example
add_executable(async_timer_example async_timer_example.cpp)
target_link_libraries(async_timer_example coreutils ${PLATFORM_LIBS})

# ObjectPool example
add_executable(object_pool_example object_pool_example.cpp)
target_link_libraries(object_pool_example coreutils ${PLATFORM_LIBS})

# Set output directory for examples
set_target_properties(async_timer_example object_pool_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)