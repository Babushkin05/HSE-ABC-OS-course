#include "CountingSemaphore.h"
#include "gtest/gtest.h"
#include <thread>

constexpr size_t BITS_PER_THREAD = 1024 * 1024 * 4;
constexpr size_t NTHREADS = 6;
constexpr size_t TARGET_BIT_INDEX(size_t i, size_t threadId)
{
    return i * NTHREADS + threadId;
}

TEST(CountingSemaphore, stress)
{
    CountingSemaphore sem(1);

    std::vector<bool> bitField;
    bitField.resize(BITS_PER_THREAD * NTHREADS, false);

    std::atomic_bool start = false;

    std::vector<std::thread> workers;
    for (size_t threadId = 0; threadId < NTHREADS; ++threadId) {
        workers.emplace_back(std::thread{[threadId, &start, &bitField, &sem]() {
            while (!start)
                ;

            for (size_t i = 0; i < BITS_PER_THREAD; ++i) {
                sem.Wait();

                bitField[TARGET_BIT_INDEX(i, threadId)] = true;

                sem.Post();

                std::this_thread::yield();
            }
        }});
    }

    start = true;

    for (auto &w : workers) {
        w.join();
    }

    ASSERT_EQ(bitField.size(), BITS_PER_THREAD * NTHREADS);

    size_t falseBits = 0;
    for (bool bit : bitField) {
        if (!bit)
            falseBits++;
    }

    ASSERT_EQ(falseBits, 0) << "false rate "
                            << static_cast<float>(falseBits) * 100 / bitField.size() << "%";
}
