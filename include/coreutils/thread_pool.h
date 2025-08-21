/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_THREAD_POOL_H
#define LMSHAO_CORE_UTILS_THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace lmshao::coreutils {
/// @brief Maximum number of threads allowed.
constexpr int THREAD_NUM_MAX = 2;
/// @brief Number of threads to pre-allocate.
constexpr int THREAD_NUM_PRE_ALLOC = 1;

/**
 * @brief A thread pool for executing tasks concurrently.
 */
class ThreadPool {
public:
    /// @brief Type alias for task functions.
    using Task = std::function<void()>;

    /**
     * @brief Construct a new ThreadPool object.
     * @param preAlloc Number of threads to pre-allocate.
     * @param threadsMax Maximum number of threads.
     * @param name Name of the thread pool.
     */
    explicit ThreadPool(int preAlloc = THREAD_NUM_PRE_ALLOC, int threadsMax = THREAD_NUM_MAX, std::string name = "");
    /**
     * @brief Destroy the ThreadPool object.
     */
    ~ThreadPool();

    /**
     * @brief Add a task to the thread pool.
     * @param task The task to be executed.
     * @param serialTag Optional tag for serial execution of tasks with the same tag.
     */
    void AddTask(const Task &task, const std::string &serialTag = "");

    /**
     * @brief Shutdown the thread pool.
     */
    void Shutdown();
    /**
     * @brief Get the current queue size.
     * @return The number of tasks in the queue.
     */
    size_t GetQueueSize() const;
    /**
     * @brief Get the current thread count.
     * @return The number of threads in the pool.
     */
    size_t GetThreadCount() const;

private:
    /**
     * @brief Worker function for threads.
     */
    void Worker();
    /**
     * @brief Create a new worker thread.
     */
    void CreateWorkerThread();

    /**
     * @brief Structure to hold task information.
     */
    struct TaskItem {
        /**
         * @brief Default constructor.
         */
        TaskItem() = default;
        /**
         * @brief Construct a new TaskItem object.
         * @param task The task function.
         * @param serialTag The serial tag.
         */
        TaskItem(Task task, std::string serialTag) : fn(std::move(task)), tag(std::move(serialTag)) {}

        /**
         * @brief Reset the task item for reuse.
         * @param task The new task function.
         * @param serialTag The new serial tag.
         */
        void reset(Task task, std::string serialTag)
        {
            fn = std::move(task);
            tag = std::move(serialTag);
        }

        /**
         * @brief Clear the task item for return to pool.
         */
        void clear()
        {
            fn = nullptr;
            tag.clear();
        }

        /// @brief The task function.
        Task fn;
        /// @brief The serial tag.
        std::string tag;
    };

    /**
     * @brief Check if there are serial tasks available.
     * @return True if serial tasks are available, false otherwise.
     */
    bool HasSerialTask() const;
    /**
     * @brief Get the next serial task.
     * @return Shared pointer to the next serial task.
     */
    std::shared_ptr<TaskItem> GetNextSerialTask();

    /**
     * @brief Acquire a task item from the pool.
     * @return Shared pointer to a task item.
     */
    std::shared_ptr<TaskItem> AcquireTaskItem();
    /**
     * @brief Release a task item back to the pool.
     * @param item The task item to release.
     */
    void ReleaseTaskItem(std::shared_ptr<TaskItem> item);

private:
    /// @brief Flag indicating if the thread pool is running.
    std::atomic<bool> running_{true};
    /// @brief Flag indicating if the thread pool is shutting down.
    std::atomic<bool> shutdown_{false};

    /// @brief Maximum number of threads.
    int threadsMax_;
    /// @brief Number of idle threads.
    std::atomic<int> idle_{0};
    /// @brief Name of the thread pool.
    std::string threadName_ = "threadpool";

    /// @brief Mutex for thread safety.
    mutable std::mutex mutex_;
    /// @brief Condition variable for signaling.
    std::condition_variable signal_;

    /// @brief Queue of tasks to be executed.
    std::queue<std::shared_ptr<TaskItem>> tasks_;
    /// @brief Vector of worker threads.
    std::vector<std::unique_ptr<std::thread>> threads_;

    /// @brief Map of serial tasks grouped by tag.
    std::unordered_map<std::string, std::queue<std::shared_ptr<TaskItem>>> serialTasks_;
    /// @brief Set of currently running serial tags.
    std::unordered_set<std::string> runningSerialTags_;

    /// @brief Queue of available serial tags for O(1) lookup.
    std::queue<std::string> availableSerialTags_;
    /// @brief Pool of task items for reuse.
    std::stack<std::shared_ptr<TaskItem>> taskItemPool_;
};

} // namespace lmshao::coreutils

#endif // LMSHAO_CORE_UTILS_THREAD_POOL_H