#pragma once
#include "Thread.cpp"
#include <iostream>
#include <queue>
#include <unistd.h> // dont work at windows

class Client : public Thread {
public:
  int id;
  int during;
  Client(int _id, int _checkin, int _during, std::queue<Client*> &_waiters,
         pthread_rwlock_t &_mutex)
      : id{_id}, checkin_time{_checkin}, during{_during}, waiters{_waiters},
        queue_mutex{_mutex} {}
  virtual void run() {
    sleep(checkin_time);
    pthread_rwlock_wrlock(&queue_mutex);
    waiters.push(new Client{*this});
    pthread_rwlock_unlock(&queue_mutex);
  }

private:
  int checkin_time;
  std::queue<Client*> &waiters;
  pthread_rwlock_t &queue_mutex;
};

struct live_args {
  int &room_number;
  pthread_rwlock_t &mutex;
  int cur_time;
  int during;
  int id;
  pthread_rwlock_t &cout_mutex;
  FILE *fptr;
};

class Client_live : public Thread {
public:
  Client_live(live_args _args) : args{_args} {}
  virtual void run() {
    pthread_rwlock_wrlock(&args.cout_mutex);
    int id = args.id;
    printf("\tClient %d check-in into his room for %d days.\n",id,args.during);
    fprintf(args.fptr,"\tClient %d check-in into his room for %d days.\n",id,args.during);
    pthread_rwlock_unlock(&args.cout_mutex);

    sleep(args.during);
    pthread_rwlock_wrlock(&args.mutex);
    ++args.room_number;
    pthread_rwlock_unlock(&args.mutex);
    pthread_rwlock_wrlock(&args.cout_mutex);
    printf("\tClient %d leave his room.\n",id);
    fprintf(args.fptr,"\tClient %d leave his room.\n",id);
    pthread_rwlock_unlock(&args.cout_mutex);
  }

private:
  live_args args;
};