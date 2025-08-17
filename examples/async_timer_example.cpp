/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "async_timer.h"

using namespace lmshao::coreutils;

int main()
{
    std::cout << "AsyncTimer Example Program\n";
    std::cout << "=========================\n\n";

    // Create AsyncTimer with 3 worker threads
    auto timer = std::make_unique<AsyncTimer>(3);

    // Start the timer service
    if (timer->Start() != 0) {
        std::cerr << "Failed to start timer service\n";
        return 1;
    }

    std::cout << "Timer service started with " << timer->GetThreadPoolThreadCount() << " worker threads\n\n";

    // Example 1: One-time timer
    std::cout << "Example 1: Scheduling a one-time timer (500ms delay)\n";
    auto onceCallback = []() {
        std::cout << "  ["
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                             .count() %
                         100000
                  << "] One-time timer executed!\n";
    };

    auto onceTimerId = timer->ScheduleOnce(onceCallback, 500);
    std::cout << "Scheduled one-time timer with ID: " << onceTimerId << "\n\n";

    // Example 2: Repeating timer
    std::cout << "Example 2: Scheduling a repeating timer (300ms interval)\n";
    std::atomic<int> repeatCounter{0};
    auto repeatCallback = [&repeatCounter]() {
        int count = repeatCounter.fetch_add(1) + 1;
        std::cout << "  ["
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                             .count() %
                         100000
                  << "] Repeating timer executed (count: " << count << ")\n";
    };

    auto repeatTimerId = timer->ScheduleRepeating(repeatCallback, 300);
    std::cout << "Scheduled repeating timer with ID: " << repeatTimerId << "\n\n";

    // Example 3: Repeating timer with initial delay
    std::cout << "Example 3: Scheduling a repeating timer with initial delay (1000ms initial, 400ms interval)\n";
    std::atomic<int> delayCounter{0};
    auto delayCallback = [&delayCounter]() {
        int count = delayCounter.fetch_add(1) + 1;
        std::cout << "  ["
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                             .count() %
                         100000
                  << "] Delayed repeating timer executed (count: " << count << ")\n";
    };

    auto delayTimerId = timer->ScheduleRepeating(delayCallback, 400, 1000);
    std::cout << "Scheduled delayed repeating timer with ID: " << delayTimerId << "\n\n";

    // Example 4: Multiple concurrent timers
    std::cout << "Example 4: Scheduling multiple concurrent timers\n";
    for (int i = 1; i <= 3; ++i) {
        auto concurrentCallback = [i]() {
            std::cout << "  ["
                      << std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now().time_since_epoch())
                                 .count() %
                             100000
                      << "] Concurrent timer " << i << " executed (thread: " << std::this_thread::get_id() << ")\n";

            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };

        auto timerId = timer->ScheduleOnce(concurrentCallback, 200 + i * 50);
        std::cout << "Scheduled concurrent timer " << i << " with ID: " << timerId << "\n";
    }
    std::cout << "\n";

    // Let timers run for a while
    std::cout << "Letting timers run for 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "\nActive timers: " << timer->GetActiveTimerCount() << "\n";
    std::cout << "Thread pool queue size: " << timer->GetThreadPoolQueueSize() << "\n\n";

    // Example 5: Cancel a specific timer
    std::cout << "Example 5: Canceling the repeating timer\n";
    if (timer->Cancel(repeatTimerId)) {
        std::cout << "Successfully canceled repeating timer with ID: " << repeatTimerId << "\n";
    } else {
        std::cout << "Failed to cancel repeating timer with ID: " << repeatTimerId << "\n";
    }

    std::cout << "Active timers after cancellation: " << timer->GetActiveTimerCount() << "\n\n";

    // Let remaining timers run a bit more
    std::cout << "Letting remaining timers run for 2 more seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Example 6: Cancel all timers
    std::cout << "\nExample 6: Canceling all remaining timers\n";
    timer->CancelAll();
    std::cout << "All timers canceled\n";
    std::cout << "Active timers: " << timer->GetActiveTimerCount() << "\n\n";

    // Stop the timer service
    std::cout << "Stopping timer service...\n";
    timer->Stop();
    std::cout << "Timer service stopped\n";

    std::cout << "\nFinal statistics:\n";
    std::cout << "- Repeat counter: " << repeatCounter.load() << "\n";
    std::cout << "- Delay counter: " << delayCounter.load() << "\n";
    std::cout << "- Thread pool queue size: " << timer->GetThreadPoolQueueSize() << "\n";

    std::cout << "\nAsyncTimer example completed!\n";
    return 0;
}