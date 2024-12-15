#pragma once
#include "Thread.cpp"
#include <queue>
#include <unistd.h> // dont work at windows
#include <iostream>

class Client: public Thread{
    public:
    int id;
            int during;
    Client(int _id,int _checkin, int _during, std::queue<Client> &_waiters, pthread_mutex_t &_mutex) : id{_id}, checkin_time{_checkin}, during{_during}, waiters{_waiters}, queue_mutex{_mutex}{}
    virtual void run() {
        sleep(checkin_time);
        pthread_mutex_lock(&queue_mutex);
        waiters.push(*this);
        pthread_mutex_unlock(&queue_mutex);
    }

    private:
    int checkin_time;
    std::queue<Client> &waiters;
         pthread_mutex_t &queue_mutex;
};

struct live_args{
        int &room_number;
        pthread_mutex_t &mutex;
        size_t cur_time;
        int during;
        int id;
    };

class Client_live: public Thread{
    public:
    Client_live(live_args _args) :args{_args} {}
    virtual void run(){
        std::cout<<"\tClient "<<args.id<<" check-in into his room for "<<args.during<<" days.\n";
        pthread_mutex_lock(&args.mutex);
        --args.room_number;
        pthread_mutex_unlock(&args.mutex);
        sleep(args.during);
        pthread_mutex_lock(&args.mutex);
        ++args.room_number;
        pthread_mutex_unlock(&args.mutex);
        std::cout<<"\tClient "<<args.id<<" leave his room.\n";
    }
    private:
    live_args args;
};