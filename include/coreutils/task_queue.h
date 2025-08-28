/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_TASK_QUEUE_H
#define LMSHAO_CORE_UTILS_TASK_QUEUE_H

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace lmshao::coreutils {
class TaskQueue;
template <typename T>
class TaskHandler;

/**
 * @brief Represents the result of a task.
 * @tparam T The type of the result.
 */
template <typename T>
struct TaskResult {
    /**
     * @brief Check if the result is available.
     * @return True if the result is available, false otherwise.
     */
    bool HasResult() { return val.has_value(); }
    /**
     * @brief Get the value of the result.
     * @return The result value.
     */
    T Value() { return val.value(); }

private:
    friend class TaskHandler<T>;
    /// @brief The optional result value.
    std::optional<T> val;
};

/**
 * @brief Represents the result of a task with no return value.
 */
template <>
struct TaskResult<void> {
    /**
     * @brief Check if the task has been executed.
     * @return True if the task has been executed, false otherwise.
     */
    bool HasResult() { return executed; }

private:
    friend class TaskHandler<void>;
    /// @brief Flag indicating if the task has been executed.
    bool executed = false;
};

/**
 * @brief Interface for task handlers.
 */
class ITaskHandler {
public:
    /**
     * @brief Structure for task attributes.
     */
    struct Attribute {
        /**
         * @brief Construct a new Attribute object.
         * @param interval The periodic execution time in microseconds. UINT64_MAX means no periodic execution.
         */
        explicit Attribute(uint64_t interval = UINT64_MAX) : periodicTimeUs_(interval) {}

        /// @brief Periodic execute time in microseconds.
        uint64_t periodicTimeUs_;
    };
    /**
     * @brief Destroy the ITaskHandler object.
     */
    virtual ~ITaskHandler() = default;
    /**
     * @brief Execute the task.
     */
    virtual void Execute() = 0;
    /**
     * @brief Cancel the task.
     */
    virtual void Cancel() = 0;
    /**
     * @brief Check if the task is canceled.
     * @return True if the task is canceled, false otherwise.
     */
    virtual bool IsCanceled() = 0;
    /**
     * @brief Get the task attributes.
     * @return The task attributes.
     */
    virtual Attribute GetAttribute() const = 0;

private:
    /**
     * @brief Clear the internal state of the task handler.
     */
    virtual void Clear() = 0;
    friend class TaskQueue;
};

/**
 * @brief A handler for a task that can be executed in a TaskQueue.
 * @tparam T The return type of the task.
 */
template <typename T>
class TaskHandler : public ITaskHandler {
public:
    /**
     * @brief Construct a new TaskHandler object.
     * @param task The task to be executed.
     * @param interval The periodic execution time in microseconds.
     */
    explicit TaskHandler(std::function<T(void)> task, uint64_t interval = UINT64_MAX)
        : task_(task), attribute_(interval)
    {
    }

    /**
     * @brief Destroy the TaskHandler object.
     */
    ~TaskHandler() override = default;

    /**
     * @brief Execute the task.
     */
    void Execute() override
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (state_ != TaskState::IDLE) {
                return;
            }
            state_ = TaskState::RUNNING;
        }

        if constexpr (std::is_void_v<T>) {
            task_();
            std::unique_lock<std::mutex> lock(mutex_);
            state_ = TaskState::FINISHED;
            result_.executed = true;
        } else {
            T result = task_();
            std::unique_lock<std::mutex> lock(mutex_);
            state_ = TaskState::FINISHED;
            result_.val = result;
        }
        cond_.notify_all();
    }

    /**
     * @brief Get the result of the task. This will block until the task is finished or canceled.
     * @return The result of the task.
     */
    TaskResult<T> GetResult()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while ((state_ != TaskState::FINISHED) && (state_ != TaskState::CANCELED)) {
            cond_.wait(lock);
        }

        return ClearResult();
    }

    /**
     * @brief Cancel the task.
     */
    void Cancel() override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (state_ != RUNNING) {
            state_ = TaskState::CANCELED;
            cond_.notify_all();
        }
    }

    /**
     * @brief Check if the task is canceled.
     * @return True if the task is canceled, false otherwise.
     */
    bool IsCanceled() override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return state_ == TaskState::CANCELED;
    }

    /**
     * @brief Get the task attributes.
     * @return The task attributes.
     */
    ITaskHandler::Attribute GetAttribute() const override { return attribute_; }

private:
    /**
     * @brief Clear the result of the task.
     * @return The cleared result.
     */
    TaskResult<T> ClearResult()
    {
        if (state_ == TaskState::FINISHED) {
            state_ = TaskState::IDLE;
            TaskResult<T> tmp;
            if constexpr (std::is_void_v<T>) {
                std::swap(tmp.executed, result_.executed);
            } else {
                result_.val.swap(tmp.val);
            }
            return tmp;
        }
        return result_;
    }

    /**
     * @brief Clear the internal state of the task handler.
     */
    void Clear() override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        (void)ClearResult();
    }

    /// @brief The state of the task.
    enum TaskState {
        IDLE = 0,
        RUNNING = 1,
        CANCELED = 2,
        FINISHED = 3,
    };

    /// @brief The current state of the task.
    TaskState state_ = TaskState::IDLE;
    /// @brief Mutex for thread safety.
    std::mutex mutex_;
    /// @brief Condition variable for waiting on task completion.
    std::condition_variable cond_;
    /// @brief The task function.
    std::function<T(void)> task_;
    /// @brief The result of the task.
    TaskResult<T> result_;
    /// @brief The task execution attributes.
    ITaskHandler::Attribute attribute_;
};

/**
 * @brief A queue that executes tasks in a separate thread.
 */
class TaskQueue {
public:
    /**
     * @brief Construct a new TaskQueue object.
     * @param name The name of the task queue.
     */
    explicit TaskQueue(const std::string &name) : name_(name) {}
    /**
     * @brief Destroy the TaskQueue object.
     */
    ~TaskQueue();

    /**
     * @brief Start the task queue.
     * @return 0 on success, -1 on failure.
     */
    int32_t Start();
    /**
     * @brief Stop the task queue.
     * @return 0 on success, -1 on failure.
     */
    int32_t Stop() noexcept;

    /**
     * @brief Check if a task is currently executing.
     * @return True if a task is executing, false otherwise.
     */
    bool IsTaskExecuting();

    /**
     * @brief Enqueue a task to be executed.
     * @param task The task to be executed.
     * @param cancelNotExecuted If true, cancel tasks that have not been executed when the queue is stopped.
     * @param delayUs The delay in microseconds before executing the task.
     * @return 0 on success, -1 on failure.
     */
    int32_t EnqueueTask(const std::shared_ptr<ITaskHandler> &task, bool cancelNotExecuted = false,
                        uint64_t delayUs = 0ULL);

private:
    /**
     * @brief A struct to hold a task and its execution time.
     */
    struct TaskHandlerItem {
        /// @brief The task handler.
        std::shared_ptr<ITaskHandler> task_{nullptr};
        /// @brief The time at which the task should be executed, in nanoseconds.
        uint64_t executeTimeNs_{0ULL};
    };
    /**
     * @brief The main loop for the task processor thread.
     */
    void TaskProcessor();
    /**
     * @brief Cancel all tasks that have not been executed.
     */
    void CancelNotExecutedTaskLocked();

    /// @brief Flag to indicate if the task queue is exiting.
    bool isExit_ = true;
    /// @brief The task processor thread.
    std::unique_ptr<std::thread> thread_;
    /// @brief The list of tasks to be executed.
    std::list<TaskHandlerItem> taskList_;
    /// @brief Mutex for thread safety.
    std::mutex mutex_;
    /// @brief Condition variable for waiting on tasks.
    std::condition_variable cond_;
    /// @brief The name of the task queue.
    std::string name_;
    /// @brief The thread ID of the task processor thread.
#ifdef _WIN32
    DWORD tid_ = 0;
#else
    pid_t tid_ = -1;
#endif
    /// @brief Flag to indicate if a task is currently executing.
    bool isTaskExecuting_ = false;
};

} // namespace lmshao::coreutils

#endif // LMSHAO_CORE_UTILS_TASK_QUEUE_H
