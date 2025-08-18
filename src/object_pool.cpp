/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#include "object_pool.h"

#include "data_buffer.h"

namespace lmshao::coreutils {

DataBufferPool::DataBufferPool(size_t defaultSize, size_t maxPoolSize) : defaultSize_(defaultSize)
{
    // Factory function to create DataBuffer with default size
    auto factory = [defaultSize]() { return new DataBuffer(defaultSize); };

    // Reset function to clear DataBuffer state before reuse
    auto resetter = [](DataBuffer *buf) {
        if (buf) {
            buf->Clear();
        }
    };

    // Create the underlying ObjectPool
    pool_ = std::make_unique<ObjectPool<DataBuffer>>(factory, resetter, nullptr, maxPoolSize);
}

std::shared_ptr<DataBuffer> DataBufferPool::Acquire(size_t size)
{
    auto buffer = pool_->Acquire();

    // Ensure the buffer has sufficient capacity
    if (size > 0 && buffer->Capacity() < size) {
        buffer->SetCapacity(size);
    }

    return buffer;
}

} // namespace lmshao::coreutils