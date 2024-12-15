#pragma once
#include <cstdlib>
#include <memory>
#include <pthread.h>
#include <unistd.h>

class Thread {
public:
    virtual ~Thread () {}
    virtual void run () = 0;
    int start () {
        return pthread_create( &_ThreadId, nullptr, Thread::thread_func, this ); 
    }
    int wait () { 
        return pthread_join( _ThreadId, NULL ); 
    }
protected:
    pthread_t _ThreadId;
    //Thread(const Thread&);
    static void* thread_func(void* d) {
        (static_cast <Thread*>(d))->run(); 
        return nullptr; 
    }
};