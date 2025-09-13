/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_LMCORE_NONCOPYABLE_H
#define LMSHAO_LMCORE_NONCOPYABLE_H

namespace lmshao::lmcore {
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

} // namespace lmshao::lmcore

#endif // LMSHAO_LMCORE_NONCOPYABLE_H