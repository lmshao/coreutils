/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "../test_framework.h"
#include "async_timer.h"

using namespace lmshao;

TEST(AsyncTimerTest, StartAndStop)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_FALSE(timer->IsRunning());

    EXPECT_EQ(0, timer->Start());
    EXPECT_TRUE(timer->IsRunning());

    EXPECT_EQ(0, timer->Stop());
    EXPECT_FALSE(timer->IsRunning());
}

TEST(AsyncTimerTest, DoubleStart)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());
    EXPECT_EQ(0, timer->Start()); // Should return 0 (already running)
}

TEST(AsyncTimerTest, ScheduleOnceTimer)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    std::atomic<bool> executed{false};
    auto callback = [&executed]() { executed.store(true); };

    auto timerId = timer->ScheduleOnce(callback, 100);
    EXPECT_NE(0, timerId);
    EXPECT_EQ(1, timer->GetActiveTimerCount());

    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_TRUE(executed.load());
    EXPECT_EQ(0, timer->GetActiveTimerCount());
}

TEST(AsyncTimerTest, ScheduleRepeatingTimer)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    std::atomic<int> counter{0};
    auto callback = [&counter]() { counter.fetch_add(1); };

    auto timerId = timer->ScheduleRepeating(callback, 50);
    EXPECT_NE(0, timerId);
    EXPECT_EQ(1, timer->GetActiveTimerCount());

    // Wait for multiple executions
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_GE(counter.load(), 3);               // Should execute at least 3 times
    EXPECT_EQ(1, timer->GetActiveTimerCount()); // Still active
}

TEST(AsyncTimerTest, CancelTimer)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    std::atomic<bool> executed{false};
    auto callback = [&executed]() { executed.store(true); };

    auto timerId = timer->ScheduleOnce(callback, 100);
    EXPECT_NE(0, timerId);

    // Cancel before execution
    EXPECT_TRUE(timer->Cancel(timerId));

    // Wait to ensure it doesn't execute
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_FALSE(executed.load());
}

TEST(AsyncTimerTest, CancelNonExistentTimer)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    EXPECT_FALSE(timer->Cancel(999)); // Non-existent timer ID
}

TEST(AsyncTimerTest, CancelAllTimers)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    std::atomic<int> counter1{0};
    std::atomic<int> counter2{0};

    auto callback1 = [&counter1]() { counter1.fetch_add(1); };
    auto callback2 = [&counter2]() { counter2.fetch_add(1); };

    timer->ScheduleRepeating(callback1, 50);
    timer->ScheduleRepeating(callback2, 75);

    EXPECT_EQ(2, timer->GetActiveTimerCount());

    // Let them run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    timer->CancelAll();

    // Wait a bit more to ensure they don't execute after cancellation
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int count1 = counter1.load();
    int count2 = counter2.load();

    // Wait more and verify counters don't increase
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(count1, counter1.load());
    EXPECT_EQ(count2, counter2.load());
}

TEST(AsyncTimerTest, MultipleTimers)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    std::atomic<int> counter1{0};
    std::atomic<int> counter2{0};
    std::atomic<int> counter3{0};

    auto callback1 = [&counter1]() { counter1.fetch_add(1); };
    auto callback2 = [&counter2]() { counter2.fetch_add(1); };
    auto callback3 = [&counter3]() { counter3.fetch_add(1); };

    // Schedule timers with different delays
    timer->ScheduleOnce(callback1, 50);
    timer->ScheduleOnce(callback2, 100);
    timer->ScheduleRepeating(callback3, 75);

    EXPECT_EQ(3, timer->GetActiveTimerCount());

    // Wait for execution
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(1, counter1.load()); // One-time timer
    EXPECT_EQ(1, counter2.load()); // One-time timer
    EXPECT_GE(counter3.load(), 2); // Repeating timer should execute multiple times

    EXPECT_EQ(1, timer->GetActiveTimerCount()); // Only repeating timer should remain
}

TEST(AsyncTimerTest, InvalidCallback)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    AsyncTimer::TimerCallback nullCallback = nullptr;

    auto timerId1 = timer->ScheduleOnce(nullCallback, 100);
    EXPECT_EQ(0, timerId1); // Should fail

    auto timerId2 = timer->ScheduleRepeating(nullCallback, 100);
    EXPECT_EQ(0, timerId2); // Should fail
}

TEST(AsyncTimerTest, InvalidInterval)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    auto callback = []() {};

    // Try to schedule repeating timer with 0 interval
    auto timerId = timer->ScheduleRepeating(callback, 0);
    EXPECT_EQ(0, timerId); // Should fail
}

TEST(AsyncTimerTest, ScheduleWithoutStart)
{
    auto timer = std::make_unique<AsyncTimer>();
    // Timer not started
    EXPECT_FALSE(timer->IsRunning());

    auto callback = []() {};

    auto timerId = timer->ScheduleOnce(callback, 100);
    EXPECT_EQ(0, timerId); // Should fail

    timerId = timer->ScheduleRepeating(callback, 100);
    EXPECT_EQ(0, timerId); // Should fail
}

TEST(AsyncTimerTest, RepeatingTimerWithInitialDelay)
{
    auto timer = std::make_unique<AsyncTimer>();
    EXPECT_EQ(0, timer->Start());

    std::atomic<int> counter{0};
    std::atomic<bool> firstExecution{true};
    auto startTime = std::chrono::steady_clock::now();

    auto callback = [&counter, &firstExecution, startTime]() {
        counter.fetch_add(1);
        if (firstExecution.exchange(false)) {
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime)
                    .count();
            // First execution should happen after initial delay (200ms)
            EXPECT_GE(elapsed, 180); // Allow some tolerance
        }
    };

    // Schedule with 200ms initial delay, then 100ms interval
    auto timerId = timer->ScheduleRepeating(callback, 100, 200);
    EXPECT_NE(0, timerId);

    // Wait for multiple executions
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_GE(counter.load(), 3); // Should execute multiple times
}

TEST(AsyncTimerTest, ThreadPoolIntegration)
{
    auto timer = std::make_unique<AsyncTimer>(2); // 2 threads in pool
    EXPECT_EQ(0, timer->Start());

    std::atomic<int> counter{0};
    std::atomic<int> maxConcurrent{0};
    std::atomic<int> currentConcurrent{0};

    auto callback = [&counter, &maxConcurrent, &currentConcurrent]() {
        int current = currentConcurrent.fetch_add(1) + 1;
        int max = maxConcurrent.load();
        while (current > max && !maxConcurrent.compare_exchange_weak(max, current)) {
            max = maxConcurrent.load();
        }

        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        counter.fetch_add(1);
        currentConcurrent.fetch_sub(1);
    };

    // Schedule multiple timers to execute simultaneously
    for (int i = 0; i < 5; ++i) {
        timer->ScheduleOnce(callback, 10);
    }

    // Wait for all executions
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    EXPECT_EQ(5, counter.load());
    EXPECT_GE(maxConcurrent.load(), 1);                // Should have at least 1 concurrent execution
    EXPECT_TRUE(timer->GetThreadPoolQueueSize() == 0); // All tasks should be completed
}

RUN_ALL_TESTS()