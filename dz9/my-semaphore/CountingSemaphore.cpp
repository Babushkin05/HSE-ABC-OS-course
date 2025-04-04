#include "CountingSemaphore.h"

CountingSemaphore::CountingSemaphore(int value) {
    sem_init(&sem_, 0, value);
}

CountingSemaphore::~CountingSemaphore() {
    sem_destroy(&sem_);
}

void CountingSemaphore::Post() {
    sem_post(&sem_);
}

void CountingSemaphore::Wait() {
    sem_wait(&sem_);
}

int CountingSemaphore::GetValue()
{
    int res;
    sem_getvalue(&sem_, &res);
    return res;
}
