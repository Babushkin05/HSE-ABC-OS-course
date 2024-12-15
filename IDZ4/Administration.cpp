#pragma once
#include "Thread.cpp"
#include "Client.cpp"
#include <queue>
#include <unistd.h> // dont work at windows
#include <iostream>

class Administration : public Thread{
    public:
        Administration(int _duration, int &rooms, std::queue<Client> &_waiters, pthread_mutex_t &mutex): work_duration{_duration}, rooms_number{rooms}, waiters{_waiters}, waiters_mutex(mutex) {}
        virtual void run(){
            pthread_mutex_t room_mutex;
            std::cout<<"Modeling:: \n";
            for(size_t cur_time = 0; cur_time < work_duration; ++cur_time){
                std::cout<<"Day "<<cur_time<<":\n";
                while(!waiters.empty() && rooms_number >0){
                    pthread_mutex_lock(&waiters_mutex);
                    if(waiters.empty())
                        break;
                    Client client= waiters.front();
                    waiters.pop();
                    pthread_mutex_unlock(&waiters_mutex);
                    live_args args{rooms_number,room_mutex,cur_time, client.during, client.id};
                    Client_live live{args};
                    live.start();
                }
                sleep(1);
            }
        }
    private:
    int work_duration;
    int &rooms_number;
    std::queue<Client> &waiters;
    pthread_mutex_t &waiters_mutex;
};