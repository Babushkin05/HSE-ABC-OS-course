#pragma once
#include "Client.cpp"
#include "Thread.cpp"
#include <iostream>
#include <queue>
#include <unistd.h> // dont work at windows

class Administration : public Thread {
public:
  Administration(int visitors, int &rooms, std::queue<Client *> &_waiters,
                 pthread_mutex_t &mutex)
      : visitors_count{visitors}, rooms_number{rooms}, waiters{_waiters},
        waiters_mutex(mutex) {}
  virtual void run() {
    pthread_mutex_t room_mutex;
    pthread_mutex_t cout_mutex;
    int served = 0;
    for (int cur_time = 0; served < visitors_count; ++cur_time) {

      pthread_mutex_lock(&cout_mutex);
      printf("Day %d:\n",cur_time);
      pthread_mutex_unlock(&cout_mutex);
      pthread_mutex_lock(&waiters_mutex);
      while (!waiters.empty() && rooms_number > 0) {
        if (waiters.empty()) {
          pthread_mutex_unlock(&waiters_mutex);
          break;
        }
        Client client = *waiters.front();
        waiters.pop();
        pthread_mutex_unlock(&waiters_mutex);
        live_args args{rooms_number,  room_mutex, cur_time,
                       client.during, client.id,  cout_mutex};
        Client_live live{args};
        pthread_mutex_lock(&room_mutex);
        --args.room_number;
        pthread_mutex_unlock(&room_mutex);
        live.start();
        pthread_mutex_lock(&waiters_mutex);
      }
      sleep(1);
    }
  }

private:
  int visitors_count;
  int &rooms_number;
  std::queue<Client *> &waiters;
  pthread_mutex_t &waiters_mutex;
};