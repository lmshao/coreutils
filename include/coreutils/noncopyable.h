/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_NONCOPYABLE_H
#define LMSHAO_CORE_UTILS_NONCOPYABLE_H

namespace lmshao::coreutils {
class NonCopyable {
protected:
    /**
     * @brief Default constructor.
     */
    NonCopyable() = default;
    /**
     * @brief Default virtual destructor.
     */
    virtual ~NonCopyable() = default;

public:
    /**
     * @brief Deleted copy constructor.
     */
    NonCopyable(const NonCopyable &) = delete;
    /**
     * @brief Deleted copy assignment operator.
     */
    NonCopyable &operator=(const NonCopyable &) = delete;
    /**
     * @brief Deleted move constructor.
     */
    NonCopyable(NonCopyable &&) = delete;
    /**
     * @brief Deleted move assignment operator.
     */
    NonCopyable &operator=(NonCopyable &&) = delete;
};

} // namespace lmshao::coreutils

#endif // LMSHAO_CORE_UTILS_NONCOPYABLE_H