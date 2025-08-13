/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2024-2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_NONCOPYABLE_H
#define LMSHAO_CORE_UTILS_NONCOPYABLE_H

namespace lmshao {
class NonCopyable {
protected:
    NonCopyable() = default;
    virtual ~NonCopyable() = default;

public:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = delete;
    NonCopyable &operator=(NonCopyable &&) = delete;
};

} // namespace lmshao

#endif // LMSHAO_CORE_UTILS_NONCOPYABLE_H