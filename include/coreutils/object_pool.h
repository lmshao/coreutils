/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_OBJECT_POOL_H
#define LMSHAO_CORE_UTILS_OBJECT_POOL_H

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace lmshao::coreutils {

/**
 * @brief Generic object pool template class
 * @tparam T The type of objects to pool
 *
 * This class provides a thread-safe object pool that can be used by any class
 * to manage instances of type T. Each class can have its own independent pool.
 */
template <typename T>
class ObjectPool {
public:
    using ObjectPtr = std::shared_ptr<T>;
    using ObjectFactory = std::function<T *()>;
    using ObjectResetter = std::function<void(T *)>;
    using ObjectDeleter = std::function<void(T *)>;

    /**
     * @brief Constructor
     * @param factory Function to create new objects when pool is empty (optional, defaults to new T())
     * @param resetter Function to reset object state before reuse (optional)
     * @param deleter Function to delete objects (optional, uses delete by default)
     * @param maxPoolSize Maximum number of objects to keep in pool (default: 100)
     */
    explicit ObjectPool(ObjectFactory factory = nullptr, ObjectResetter resetter = nullptr,
                        ObjectDeleter deleter = nullptr, size_t maxPoolSize = 100)
        : factory_(factory ? std::move(factory) : []() { return new T(); }), resetter_(std::move(resetter)),
          deleter_(deleter ? std::move(deleter) : [](T *obj) { delete obj; }), maxPoolSize_(maxPoolSize)
    {
    }

    /**
     * @brief Destructor - cleans up all pooled objects
     */
    ~ObjectPool()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (T *obj : pool_) {
            deleter_(obj);
        }
        pool_.clear();
    }

    /**
     * @brief Acquire an object from the pool
     * @return Shared pointer to the object with custom deleter that returns it to pool
     */
    ObjectPtr Acquire()
    {
        T *obj = nullptr;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!pool_.empty()) {
                obj = pool_.back();
                pool_.pop_back();
            }
        }

        if (!obj) {
            obj = factory_();
        } else if (resetter_) {
            resetter_(obj);
        }

        // Return shared_ptr with custom deleter that returns object to pool
        return ObjectPtr(obj, [this](T *ptr) { this->Release(ptr); });
    }

    /**
     * @brief Get current pool size
     * @return Number of objects currently in the pool
     */
    size_t GetPoolSize() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

    /**
     * @brief Get maximum pool size
     * @return Maximum number of objects that can be stored in pool
     */
    size_t GetMaxPoolSize() const { return maxPoolSize_; }

    /**
     * @brief Set maximum pool size
     * @param maxSize New maximum pool size
     */
    void SetMaxPoolSize(size_t maxSize)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        maxPoolSize_ = maxSize;

        // Remove excess objects if new max size is smaller
        while (pool_.size() > maxPoolSize_) {
            T *obj = pool_.back();
            pool_.pop_back();
            deleter_(obj);
        }
    }

    /**
     * @brief Clear all objects from the pool
     */
    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (T *obj : pool_) {
            deleter_(obj);
        }
        pool_.clear();
    }

private:
    /**
     * @brief Release an object back to the pool
     * @param obj Object to release
     */
    void Release(T *obj)
    {
        if (!obj) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_.size() < maxPoolSize_) {
            pool_.push_back(obj);
        } else {
            deleter_(obj);
        }
    }

    ObjectFactory factory_;   // Function to create new objects
    ObjectResetter resetter_; // Function to reset object state
    ObjectDeleter deleter_;   // Function to delete objects
    size_t maxPoolSize_;      // Maximum pool size

    mutable std::mutex mutex_; // Mutex for thread safety
    std::vector<T *> pool_;    // Pool of available objects
};

/**
 * @brief Specialized DataBuffer object pool
 *
 * This class provides a convenient wrapper around ObjectPool specifically
 * for DataBuffer objects, with appropriate factory and reset functions.
 */
class DataBufferPool {
public:
    /**
     * @brief Constructor
     * @param defaultSize Default size for new DataBuffer objects
     * @param maxPoolSize Maximum number of objects to keep in pool
     */
    explicit DataBufferPool(size_t defaultSize = 4096, size_t maxPoolSize = 100);

    /**
     * @brief Acquire a DataBuffer from the pool
     * @param size Minimum size required (will resize if necessary)
     * @return Shared pointer to DataBuffer
     */
    std::shared_ptr<class DataBuffer> Acquire(size_t size = 0);

    /**
     * @brief Get current pool size
     */
    size_t GetPoolSize() const { return pool_->GetPoolSize(); }

    /**
     * @brief Get maximum pool size
     */
    size_t GetMaxPoolSize() const { return pool_->GetMaxPoolSize(); }

    /**
     * @brief Set maximum pool size
     */
    void SetMaxPoolSize(size_t maxSize) { pool_->SetMaxPoolSize(maxSize); }

    /**
     * @brief Clear all objects from the pool
     */
    void Clear() { pool_->Clear(); }

private:
    size_t defaultSize_;
    std::unique_ptr<ObjectPool<DataBuffer>> pool_;
};

} // namespace lmshao::coreutils

#endif // LMSHAO_CORE_UTILS_OBJECT_POOL_H