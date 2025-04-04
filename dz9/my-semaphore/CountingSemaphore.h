#pragma once

#include <atomic>
#include <semaphore.h>

class CountingSemaphore
{
public:
    CountingSemaphore(int value);
    ~CountingSemaphore();

    void Post();
    void Wait();
    int GetValue();

private:
    sem_t sem_;
};
