/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_SINGLETON_H
#define LMSHAO_CORE_UTILS_SINGLETON_H

#include <memory>
#include <mutex>

#include "noncopyable.h"

namespace lmshao::coreutils {
template <typename T>
class Singleton : public NonCopyable {
public:
    /**
     * @brief Get the singleton instance.
     * @return A shared pointer to the singleton instance.
     */
    static std::shared_ptr<T> GetInstance();
    /**
     * @brief Destroy the singleton instance.
     */
    static void DestroyInstance();

protected:
private:
    /// @brief The singleton instance.
    static std::shared_ptr<T> instance_;
    /// @brief Mutex for thread-safe initialization.
    static std::mutex mutex_;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;

template <typename T>
std::mutex Singleton<T>::mutex_;

template <typename T>
std::shared_ptr<T> Singleton<T>::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::shared_ptr<T>(new T());
        }
    }

    return instance_;
}

template <typename T>
void Singleton<T>::DestroyInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance_ != nullptr) {
        instance_.reset();
    }
}

/**
 * @brief Meyers Singleton pattern implementation.
 *
 * This implementation uses the "Initialization On First Use" idiom,
 * also known as the Meyers Singleton. It's thread-safe in C++11 and later
 * due to guaranteed thread-safe initialization of local static variables.
 *
 * Usage:
 * class MyClass : public MeyersSingleton<MyClass> {
 *     friend class MeyersSingleton<MyClass>;
 * private:
 *     MyClass() = default;
 * };
 *
 * auto& instance = MyClass::GetInstance();
 */
template <typename T>
class MeyersSingleton : public NonCopyable {
public:
    /**
     * @brief Get the singleton instance.
     * @return A reference to the singleton instance.
     */
    static T &GetInstance()
    {
        static T instance;
        return instance;
    }

protected:
    MeyersSingleton() = default;
    ~MeyersSingleton() = default;
};

} // namespace lmshao::coreutils

#endif // LMSHAO_CORE_UTILS_SINGLETON_H