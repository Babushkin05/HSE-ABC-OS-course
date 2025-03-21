#pragma once
#include "Client.cpp"
#include "Thread.cpp"
#include <iostream>
#include <queue>
#include <unistd.h> // dont work at windows
#include <stdio.h>

class Administration : public Thread {
public:
  Administration(int _visitors, int &rooms, std::queue<Client *> &_waiters,
                 pthread_mutex_t &mutex, FILE *_fptr)
      : visitors{_visitors}, rooms_number{rooms}, waiters{_waiters},
        waiters_mutex(mutex),  fptr{_fptr} {}
  virtual void run() {
    pthread_mutex_t room_mutex;
    pthread_mutex_t cout_mutex;

    for (int cur_time = 0; cur_time < 16; ++cur_time) {
      pthread_mutex_lock(&cout_mutex);
      printf("Day %d:\n", cur_time);
      fprintf(fptr, "Day %d:\n", cur_time);
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
                       client.during, client.id,  cout_mutex, fptr};
        Client_live *live = new Client_live{args};
        pthread_mutex_lock(&room_mutex);
        --rooms_number;
        pthread_mutex_unlock(&room_mutex);
        live->start();
        pthread_mutex_lock(&waiters_mutex);
      }
      sleep(1);
    }
  }

private:
  int visitors;
  int &rooms_number;
  std::queue<Client *> &waiters;
  pthread_mutex_t &waiters_mutex;
  FILE *fptr;
};