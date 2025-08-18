/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <string>

#include "data_buffer.h"
#include "object_pool.h"

using namespace lmshao::coreutils;

int main()
{
    std::cout << "DataBufferPool Simple Example" << std::endl;
    std::cout << "============================" << std::endl;

    // Create a DataBuffer pool with 1KB buffers, max 5 in pool
    DataBufferPool pool(1024, 5);

    std::cout << "\nInitial pool state:" << std::endl;
    std::cout << "Pool size: " << pool.GetPoolSize() << std::endl;
    std::cout << "Max pool size: " << pool.GetMaxPoolSize() << std::endl;

    std::cout << "\n=== Basic Usage ===" << std::endl;

    // Acquire a buffer from pool
    auto buffer1 = pool.Acquire();
    std::cout << "Acquired buffer1, capacity: " << buffer1->Capacity() << " bytes" << std::endl;
    std::cout << "Pool size after acquire: " << pool.GetPoolSize() << std::endl;

    // Use the buffer
    buffer1->Assign("Hello, DataBufferPool!");
    std::cout << "Buffer1 content: \"" << buffer1->ToString() << "\"" << std::endl;
    std::cout << "Buffer1 size: " << buffer1->Size() << " bytes" << std::endl;

    // Acquire another buffer
    auto buffer2 = pool.Acquire();
    buffer2->Assign("This is buffer2");
    std::cout << "Buffer2 content: \"" << buffer2->ToString() << "\"" << std::endl;

    std::cout << "\n=== Pool Reuse ===" << std::endl;

    // Release buffers (they return to pool automatically when going out of scope)
    buffer1.reset();
    buffer2.reset();

    std::cout << "Pool size after releasing buffers: " << pool.GetPoolSize() << std::endl;

    // Acquire buffer again (should reuse from pool)
    auto buffer3 = pool.Acquire();
    std::cout << "Acquired buffer3, size: " << buffer3->Size() << " (should be 0 - cleared)" << std::endl;
    std::cout << "Pool size after reacquire: " << pool.GetPoolSize() << std::endl;

    // Use the reused buffer
    buffer3->Assign("Reused buffer!");
    std::cout << "Buffer3 content: \"" << buffer3->ToString() << "\"" << std::endl;

    std::cout << "\n=== Different Size Requirements ===" << std::endl;

    // Request buffer with specific size
    auto largeBuffer = pool.Acquire(2048);
    std::cout << "Requested 2048 bytes, got capacity: " << largeBuffer->Capacity() << " bytes" << std::endl;

    largeBuffer->Assign("This is a larger buffer for more data!");
    std::cout << "Large buffer content: \"" << largeBuffer->ToString() << "\"" << std::endl;

    std::cout << "\n=== Final Pool State ===" << std::endl;

    // All buffers will be automatically returned to pool when they go out of scope
    buffer3.reset();
    largeBuffer.reset();

    std::cout << "Final pool size: " << pool.GetPoolSize() << std::endl;

    std::cout << "\n=== Example completed! ===" << std::endl;

    return 0;
}