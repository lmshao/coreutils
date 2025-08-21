/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "../test_framework.h"
#include "coreutils/data_buffer.h"
#include "coreutils/object_pool.h"

using namespace lmshao::coreutils;

// Simple test class for ObjectPool testing
class TestObject {
public:
    TestObject(int value = 0) : value_(value), resetCount_(0) {}

    int GetValue() const { return value_; }
    void SetValue(int value) { value_ = value; }

    int GetResetCount() const { return resetCount_; }
    void Reset()
    {
        value_ = 0;
        resetCount_++;
    }

private:
    int value_;
    int resetCount_;
};

TEST(ObjectPoolTests, BasicObjectPoolFunctionality)
{
    // Create object pool with factory and resetter
    ObjectPool<TestObject> pool([]() { return new TestObject(42); },   // Factory
                                [](TestObject *obj) { obj->Reset(); }, // Resetter
                                nullptr,                               // Default deleter
                                5                                      // Max pool size
    );

    EXPECT_EQ(pool.GetPoolSize(), 0);
    EXPECT_EQ(pool.GetMaxPoolSize(), 5);

    // Acquire first object
    auto obj1 = pool.Acquire();
    EXPECT_EQ(obj1->GetValue(), 42);
    EXPECT_EQ(obj1->GetResetCount(), 0);

    // Modify object
    obj1->SetValue(100);
    EXPECT_EQ(obj1->GetValue(), 100);

    // Release object (goes back to pool)
    obj1.reset();
    EXPECT_EQ(pool.GetPoolSize(), 1);

    // Acquire again (should get the same object, but reset)
    auto obj2 = pool.Acquire();
    EXPECT_EQ(obj2->GetValue(), 0);      // Reset to 0
    EXPECT_EQ(obj2->GetResetCount(), 1); // Reset was called
    EXPECT_EQ(pool.GetPoolSize(), 0);
}

TEST(ObjectPoolTests, ObjectPoolMaxSize)
{
    ObjectPool<TestObject> pool([]() { return new TestObject(); },
                                nullptr, // No resetter
                                nullptr, // Default deleter
                                2        // Max pool size = 2
    );

    // Create 3 objects
    auto obj1 = pool.Acquire();
    auto obj2 = pool.Acquire();
    auto obj3 = pool.Acquire();

    EXPECT_EQ(pool.GetPoolSize(), 0);

    // Release all objects
    obj1.reset();
    obj2.reset();
    obj3.reset();

    // Only 2 should be kept in pool (max size limit)
    EXPECT_EQ(pool.GetPoolSize(), 2);
}

TEST(ObjectPoolTests, ObjectPoolClear)
{
    ObjectPool<TestObject> pool([]() { return new TestObject(); }, nullptr, nullptr, 10);

    // Add some objects to pool
    auto obj1 = pool.Acquire();
    auto obj2 = pool.Acquire();
    obj1.reset();
    obj2.reset();

    EXPECT_EQ(pool.GetPoolSize(), 2);

    // Clear pool
    pool.Clear();
    EXPECT_EQ(pool.GetPoolSize(), 0);
}

TEST(ObjectPoolTests, ObjectPoolSetMaxSize)
{
    ObjectPool<TestObject> pool([]() { return new TestObject(); }, nullptr, nullptr, 5);

    // Fill pool with 3 objects
    std::vector<std::shared_ptr<TestObject>> objects;
    for (int i = 0; i < 3; ++i) {
        objects.push_back(pool.Acquire());
    }
    for (auto &obj : objects) {
        obj.reset();
    }

    EXPECT_EQ(pool.GetPoolSize(), 3);

    // Reduce max size to 2
    pool.SetMaxPoolSize(2);
    EXPECT_EQ(pool.GetPoolSize(), 2);
    EXPECT_EQ(pool.GetMaxPoolSize(), 2);
}

TEST(ObjectPoolTests, DataBufferPoolBasic)
{
    DataBufferPool pool(1024, 10);

    EXPECT_EQ(pool.GetPoolSize(), 0);
    EXPECT_EQ(pool.GetMaxPoolSize(), 10);

    // Acquire buffer
    auto buffer1 = pool.Acquire();
    EXPECT_GE(buffer1->Capacity(), 1024);
    EXPECT_EQ(buffer1->Size(), 0);

    // Use buffer
    buffer1->Assign("Hello, World!");
    EXPECT_EQ(buffer1->Size(), 13);
    EXPECT_EQ(buffer1->ToString(), "Hello, World!");

    // Release buffer
    buffer1.reset();
    EXPECT_EQ(pool.GetPoolSize(), 1);

    // Acquire again (should be cleared)
    auto buffer2 = pool.Acquire();
    EXPECT_EQ(buffer2->Size(), 0);
    EXPECT_TRUE(buffer2->Empty());
}

TEST(ObjectPoolTests, DataBufferPoolSizeRequirement)
{
    DataBufferPool pool(512, 5);

    // Request buffer larger than default size
    auto buffer = pool.Acquire(2048);
    EXPECT_GE(buffer->Capacity(), 2048);

    // Request buffer smaller than default size
    auto buffer2 = pool.Acquire(256);
    EXPECT_GE(buffer2->Capacity(), 512); // Should use default size
}

TEST(ObjectPoolTests, MultipleDataBufferPools)
{
    // Create two independent pools
    DataBufferPool pool1(1024, 5);
    DataBufferPool pool2(2048, 3);

    // Use both pools
    auto buf1 = pool1.Acquire();
    auto buf2 = pool2.Acquire();

    buf1->Assign("Pool 1");
    buf2->Assign("Pool 2");

    EXPECT_EQ(buf1->ToString(), "Pool 1");
    EXPECT_EQ(buf2->ToString(), "Pool 2");

    // Release and check pool sizes
    buf1.reset();
    buf2.reset();

    EXPECT_EQ(pool1.GetPoolSize(), 1);
    EXPECT_EQ(pool2.GetPoolSize(), 1);

    // Pools are independent
    pool1.Clear();
    EXPECT_EQ(pool1.GetPoolSize(), 0);
    EXPECT_EQ(pool2.GetPoolSize(), 1); // pool2 unaffected
}

TEST(ObjectPoolTests, ThreadSafetyTest)
{
    ObjectPool<TestObject> pool([]() { return new TestObject(); }, [](TestObject *obj) { obj->Reset(); }, nullptr, 50);

    std::atomic<int> successCount{0};
    std::atomic<int> errorCount{0};

    // Launch multiple threads
    std::vector<std::thread> threads;
    const int numThreads = 10;
    const int operationsPerThread = 100;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&pool, &successCount, &errorCount, operationsPerThread]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                try {
                    auto obj = pool.Acquire();
                    obj->SetValue(j);

                    // Simulate some work
                    std::this_thread::sleep_for(std::chrono::microseconds(1));

                    if (obj->GetValue() == j) {
                        successCount++;
                    } else {
                        errorCount++;
                    }
                } catch (...) {
                    errorCount++;
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto &thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successCount.load(), numThreads * operationsPerThread);
    EXPECT_EQ(errorCount.load(), 0);
}

RUN_ALL_TESTS()