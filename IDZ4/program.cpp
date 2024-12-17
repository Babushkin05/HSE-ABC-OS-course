#include <pthread.h>
#include <iostream>
#include <queue>
#include "Administration.cpp"
#include "Client.cpp"

int rooms_count = 30;

std::queue<Client*> waiters;

pthread_mutex_t waiters_mutex;

class TestingThread : public Thread {
public:
    TestingThread (const char* pcszText) : _pcszText( pcszText ) {}
    virtual void run () {
        for (unsigned int i = 0; i < 30; i++) {
            std::cout << _pcszText << "\n"; // << std::endl;
            sleep(1);
        }
    }
protected:
    const char* _pcszText;
};

int main(){
    Client c1(1,2,2,waiters,waiters_mutex);
    Client c2(2,3,1,waiters,waiters_mutex);
    Client c3(3,0,3,waiters,waiters_mutex);

    Administration adm(10,rooms_count, waiters, waiters_mutex);
    c1.start();
    c2.start();
    c3.start();
    adm.start();
    c1.wait();
    c2.wait();
    c3.wait();
    adm.wait();
}

