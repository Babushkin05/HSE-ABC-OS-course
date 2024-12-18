#pragma once
#include "Client.cpp"
#include <queue>
#include <random>
#include <vector>

const int MAX_CLIENT_NUMBER = 100;
const int MAX_DAY_NUMBER = 7;

// function for generation client
std::vector<Client> generateClients(std::queue<Client *> &waiters,
                                    pthread_rwlock_t &mutex) {
  std::random_device random_dev;
  std::mt19937 generator(random_dev());
  std::uniform_int_distribution<> client_number_distr(10, MAX_CLIENT_NUMBER);
  std::uniform_int_distribution<> day_distr(1, MAX_DAY_NUMBER);

  int n = client_number_distr(generator);
  std::vector<Client> clients;
  for(size_t i = 0; i< n;++i){
    int checkin = day_distr(generator);
    int duration = day_distr(generator);
        std::cout<<i<<' '<<checkin<<' '<<duration<<'\n';
    clients.emplace_back(i,checkin,duration,waiters, mutex);
  }

  return clients;
}