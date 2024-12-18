#pragma once
#include "Client.cpp"
#include "Thread.cpp"
#include <iostream>
#include <queue>
#include <unistd.h> // dont work at windows
#include <stdio.h>

// Hotel administration thread class
class Administration : public Thread {
public:
  Administration(int _visitors, int &rooms, std::queue<Client *> &_waiters,
                 pthread_rwlock_t &mutex, FILE *_fptr)
      : visitors{_visitors}, rooms_number{rooms}, waiters{_waiters},
        waiters_mutex(mutex),  fptr{_fptr} {}
  virtual void run() {
    pthread_rwlock_t room_mutex;
    pthread_rwlock_t cout_mutex;
    // model days
    for (int cur_time = 0; cur_time < 16; ++cur_time) {
      pthread_rwlock_wrlock(&cout_mutex);
      printf("Day %d:\n", cur_time);
      fprintf(fptr, "Day %d:\n", cur_time);
      pthread_rwlock_unlock(&cout_mutex);
      pthread_rwlock_wrlock(&waiters_mutex);
      // find room for client
      while (!waiters.empty() && rooms_number > 0) {
        if (waiters.empty()) {
          pthread_rwlock_unlock(&waiters_mutex);
          break;
        }
        Client client = *waiters.front();
        waiters.pop();
        pthread_rwlock_wrlock(&waiters_mutex);
        live_args args{rooms_number,  room_mutex, cur_time,
                       client.during, client.id,  cout_mutex, fptr};
        Client_live *live = new Client_live{args};
        pthread_rwlock_wrlock(&room_mutex);
        --rooms_number;
        pthread_rwlock_unlock(&room_mutex);
        // start living
        live->start();
        pthread_rwlock_wrlock(&waiters_mutex);
      }
      sleep(1);
    }
  }

private:
  int visitors;
  int &rooms_number;
  std::queue<Client *> &waiters;
  pthread_rwlock_t &waiters_mutex;
  FILE *fptr;
};