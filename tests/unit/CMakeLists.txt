# Unit tests for individual components

# DataBuffer unit tests
add_executable(test_data_buffer test_data_buffer.cpp)
target_link_libraries(test_data_buffer lmcore)
add_test(NAME DataBufferTests COMMAND $<TARGET_FILE:test_data_buffer>)
set_tests_properties(DataBufferTests PROPERTIES LABELS "unit")

# TaskQueue unit tests
add_executable(test_task_queue test_task_queue.cpp)
target_link_libraries(test_task_queue lmcore)
add_test(NAME TaskQueueTests COMMAND $<TARGET_FILE:test_task_queue>)
set_tests_properties(TaskQueueTests PROPERTIES LABELS "unit")

# ThreadPool unit tests
add_executable(test_thread_pool test_thread_pool.cpp)
target_link_libraries(test_thread_pool lmcore ${PLATFORM_LIBS})
add_test(NAME ThreadPoolTests COMMAND $<TARGET_FILE:test_thread_pool>)
set_tests_properties(ThreadPoolTests PROPERTIES LABELS "unit")

# AsyncTimer unit tests
add_executable(test_async_timer test_async_timer.cpp)
target_link_libraries(test_async_timer lmcore ${PLATFORM_LIBS})
add_test(NAME AsyncTimerTests COMMAND $<TARGET_FILE:test_async_timer>)
set_tests_properties(AsyncTimerTests PROPERTIES LABELS "unit")

# ObjectPool unit tests
add_executable(test_object_pool test_object_pool.cpp)
target_link_libraries(test_object_pool lmcore ${PLATFORM_LIBS})
add_test(NAME ObjectPoolTests COMMAND $<TARGET_FILE:test_object_pool>)
set_tests_properties(ObjectPoolTests PROPERTIES LABELS "unit")