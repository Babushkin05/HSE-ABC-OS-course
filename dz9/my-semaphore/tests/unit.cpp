#include "CountingSemaphore.h"
#include "gtest/gtest.h"
#include <thread>

using namespace std::chrono_literals;

TEST(CountingSemaphore, unit)
{
    CountingSemaphore sem(2);

    std::mutex milestoneMutex;
    int milestone = 0;
    std::condition_variable syncVar;

    auto passMilestone = [&]() {
        {
            std::unique_lock lock(milestoneMutex);
            milestone++;
        }
        syncVar.notify_one();
    };

    auto expectMilestone = [&](int val) -> bool {
        std::unique_lock lock(milestoneMutex);
        return syncVar.wait_for(lock, 300ms, [val, &milestone] { return milestone >= val; });
    };

    // The test itself

    std::thread testThread([&] {
        sem.Post();
        EXPECT_EQ(sem.GetValue(), 3);

        sem.Wait();
        sem.Wait();
        sem.Wait();
        EXPECT_EQ(sem.GetValue(), 0);

        passMilestone(); // milestone 1

        // Now should block
        sem.Wait();

        passMilestone(); // milestone 2

        // Should block
        sem.Wait();
        sem.Wait();

        passMilestone(); // milestone 3
    });

    ASSERT_TRUE(expectMilestone(1));
    EXPECT_EQ(sem.GetValue(), 0);

    // should actually block before milestone 2
    ASSERT_FALSE(expectMilestone(2));
    sem.Post();
    ASSERT_TRUE(expectMilestone(2));

    // should block again before milestone 3
    ASSERT_FALSE(expectMilestone(3));
    EXPECT_EQ(sem.GetValue(), 0);
    sem.Post();
    sem.Post();
    ASSERT_TRUE(expectMilestone(3));

    testThread.join();
}
