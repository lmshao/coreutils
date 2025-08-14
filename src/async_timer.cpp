/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#include "async_timer.h"

#include <algorithm>
#include <chrono>

#include "core_utils_log.h"

namespace lmshao {

AsyncTimer::AsyncTimer(int threadPoolSize)
{
    threadPool_ = std::make_unique<ThreadPool>(1, threadPoolSize, "AsyncTimer");
}

AsyncTimer::~AsyncTimer()
{
    Stop();
}

int32_t AsyncTimer::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_.load()) {
        return 0; // Already running
    }

    shouldStop_.store(false);
    running_.store(true);

    workerThread_ = std::make_unique<std::thread>(&AsyncTimer::TimerWorker, this);
    printf("AsyncTimer started successfully\n");
    return 0;
}

int32_t AsyncTimer::Stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_.load()) {
            return 0;
        }

        shouldStop_.store(true);
        running_.store(false);
    }

    condition_.notify_all();

    if (workerThread_ && workerThread_->joinable()) {
        workerThread_->join();
        workerThread_.reset();
    }

    // Shutdown thread pool
    if (threadPool_) {
        threadPool_->Shutdown();
    }

    printf("AsyncTimer stopped successfully\n");
    return 0;
}

AsyncTimer::TimerId AsyncTimer::ScheduleOnce(const TimerCallback &callback, uint64_t delayMs)
{
    if (!callback) {
        COREUTILS_LOGE("Invalid callback for timer");
        return 0;
    }

    if (!running_.load()) {
        COREUTILS_LOGE("Timer is not running");
        return 0;
    }

    auto timerId = GenerateTimerId();
    auto now = std::chrono::steady_clock::now();
    auto execTime = now + std::chrono::milliseconds(delayMs);

    auto task = std::make_shared<TimerTask>(timerId, callback, execTime, Duration(0), false);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        timerTasks_.emplace(execTime, task);
        timerMap_[timerId] = task;
    }

    condition_.notify_one();
    return timerId;
}

AsyncTimer::TimerId AsyncTimer::ScheduleRepeating(const TimerCallback &callback, uint64_t intervalMs,
                                                  uint64_t initialDelayMs)
{
    if (!callback) {
        COREUTILS_LOGE("Invalid callback for timer");
        return 0;
    }

    if (intervalMs == 0) {
        COREUTILS_LOGE("Invalid interval for repeating timer");
        return 0;
    }

    if (!running_.load()) {
        COREUTILS_LOGE("Timer is not running");
        return 0;
    }

    auto timerId = GenerateTimerId();
    auto now = std::chrono::steady_clock::now();
    auto execTime = now + std::chrono::milliseconds(initialDelayMs > 0 ? initialDelayMs : intervalMs);

    auto task = std::make_shared<TimerTask>(timerId, callback, execTime, Duration(intervalMs), true);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        timerTasks_.emplace(execTime, task);
        timerMap_[timerId] = task;
    }

    condition_.notify_one();
    return timerId;
}

bool AsyncTimer::Cancel(TimerId timerId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = timerMap_.find(timerId);
    if (it != timerMap_.end()) {
        it->second->isCancelled = true;
        return true;
    }
    return false;
}

void AsyncTimer::CancelAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &pair : timerMap_) {
        pair.second->isCancelled = true;
    }
    printf("Cancelled all timers\n");
}

bool AsyncTimer::IsRunning() const
{
    return running_.load();
}

size_t AsyncTimer::GetActiveTimerCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return timerMap_.size();
}

size_t AsyncTimer::GetThreadPoolQueueSize() const
{
    return threadPool_ ? threadPool_->GetQueueSize() : 0;
}

void AsyncTimer::TimerWorker()
{
    printf("AsyncTimer worker thread started\n");

    while (!shouldStop_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (timerTasks_.empty()) {
            // Wait for new timers or stop signal
            condition_.wait(lock, [this] { return !timerTasks_.empty() || shouldStop_.load(); });
            continue;
        }

        auto nextTime = GetNextExecutionTime();
        auto now = std::chrono::steady_clock::now();
        if (nextTime <= now) {
            // Execute expired timers
            ExecuteExpiredTimers();
        } else {
            // Wait until next timer expires or new timer is added
            auto waitResult = condition_.wait_until(lock, nextTime, [this] { return shouldStop_.load(); });
            if (waitResult) {
                // shouldStop_ became true
                break;
            }
        }
    }

    printf("AsyncTimer worker thread stopped\n");
}

AsyncTimer::TimerId AsyncTimer::GenerateTimerId()
{
    return nextTimerId_.fetch_add(1);
}

void AsyncTimer::ExecuteExpiredTimers()
{
    auto now = std::chrono::steady_clock::now();
    std::vector<std::shared_ptr<TimerTask>> expiredTasks;

    // Find all expired timers
    auto it = timerTasks_.begin();
    while (it != timerTasks_.end() && it->first <= now) {
        auto task = it->second;
        if (!task->isCancelled) {
            expiredTasks.push_back(task);
        }
        it = timerTasks_.erase(it);
    }

    // Submit expired timers to thread pool for async execution
    for (auto &task : expiredTasks) {
        if (!task->isCancelled && threadPool_) {
            // Create a wrapper task that handles the callback execution
            auto wrappedCallback = [task]() {
                if (!task->isCancelled) {
                    task->callback();
                    COREUTILS_LOGD("Executed timer %llu asynchronously", task->id);
                }
            };

            // Submit to thread pool for async execution
            threadPool_->AddTask(wrappedCallback);
        }
    }

    // Reschedule repeating timers
    for (auto &task : expiredTasks) {
        if (!task->isCancelled && task->isRepeating) {
            task->nextExecutionTime = now + task->interval;
            timerTasks_.emplace(task->nextExecutionTime, task);
            COREUTILS_LOGD("Rescheduled repeating timer %llu", task->id);
        } else if (!task->isRepeating) {
            // Remove one-time timer from map
            timerMap_.erase(task->id);
        }
    }
}

AsyncTimer::TimePoint AsyncTimer::GetNextExecutionTime() const
{
    if (timerTasks_.empty()) {
        return std::chrono::steady_clock::now() + std::chrono::hours(24); // Far future
    }
    return timerTasks_.begin()->first;
}

size_t AsyncTimer::GetThreadPoolThreadCount() const
{
    return threadPool_ ? threadPool_->GetThreadCount() : 0;
}

} // namespace lmshao