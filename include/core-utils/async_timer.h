/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_ASYNC_TIMER_H
#define LMSHAO_CORE_UTILS_ASYNC_TIMER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "thread_pool.h"

namespace lmshao::coreutils {

class AsyncTimer {
public:
    using TimerCallback = std::function<void()>;
    using TimerId = uint64_t;
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::milliseconds;

    /**
     * @brief Constructor
     * @param threadPoolSize Maximum number of threads in the thread pool (default: 4)
     */
    explicit AsyncTimer(int threadPoolSize = 4);
    ~AsyncTimer();

    /**
     * @brief Start the timer service
     * @return 0 on success, -1 on failure
     */
    int32_t Start();

    /**
     * @brief Stop the timer service
     * @return 0 on success, -1 on failure
     */
    int32_t Stop();

    /**
     * @brief Schedule a one-time timer
     * @param callback The callback function to execute
     * @param delayMs Delay in milliseconds before execution
     * @return Timer ID (0 if failed)
     */
    TimerId ScheduleOnce(const TimerCallback &callback, uint64_t delayMs);

    /**
     * @brief Schedule a repeating timer
     * @param callback The callback function to execute
     * @param intervalMs Interval in milliseconds between executions
     * @param initialDelayMs Initial delay before first execution (default: 0)
     * @return Timer ID (0 if failed)
     */
    TimerId ScheduleRepeating(const TimerCallback &callback, uint64_t intervalMs, uint64_t initialDelayMs = 0);

    /**
     * @brief Cancel a specific timer
     * @param timerId The timer ID to cancel
     * @return true if cancelled successfully, false otherwise
     */
    bool Cancel(TimerId timerId);

    /**
     * @brief Cancel all timers
     */
    void CancelAll();

    /**
     * @brief Check if the timer service is running
     * @return true if running, false otherwise
     */
    bool IsRunning() const;

    /**
     * @brief Get the number of active timers
     * @return Number of active timers
     */
    size_t GetActiveTimerCount() const;

    /**
     * @brief Get the thread pool queue size
     * @return Number of pending tasks in thread pool
     */
    size_t GetThreadPoolQueueSize() const;

    /**
     * @brief Get the thread pool thread count
     * @return Number of threads in thread pool
     */
    size_t GetThreadPoolThreadCount() const;

private:
    struct TimerTask {
        TimerId id;
        TimerCallback callback;
        TimePoint nextExecutionTime;
        Duration interval;
        bool isRepeating;
        bool isCancelled;

        TimerTask(TimerId id, const TimerCallback &cb, TimePoint execTime, Duration interval, bool repeating)
            : id(id), callback(cb), nextExecutionTime(execTime), interval(interval), isRepeating(repeating),
              isCancelled(false)
        {
        }
    };

    void TimerWorker();
    TimerId GenerateTimerId();
    void ExecuteExpiredTimers();
    TimePoint GetNextExecutionTime() const;

    std::atomic<bool> running_{false};
    std::atomic<bool> shouldStop_{false};
    std::atomic<TimerId> nextTimerId_{1};

    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::unique_ptr<std::thread> workerThread_;

    // Timer storage
    std::multimap<TimePoint, std::shared_ptr<TimerTask>> timerTasks_;
    std::map<TimerId, std::shared_ptr<TimerTask>> timerMap_; // For quick lookup by ID

    // Thread pool for async callback execution
    std::unique_ptr<ThreadPool> threadPool_;
};

} // namespace lmshao::coreutils

#endif // LMSHAO_CORE_UTILS_ASYNC_TIMER_H