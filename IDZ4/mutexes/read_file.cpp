#pragma once
#include "Client.cpp"
#include <fstream>
#include <string>
#include <vector>

std::vector<Client> read_clients_from_file(std::string filename,
                                           std::queue<Client *> &waiters,
                                           pthread_mutex_t &mutex) {
  std::vector<Client> clients;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "ERROR :: there is no file " << filename;
    exit(1);
  }
  while (file.good()) {
    int id, checkin, duration;
    file >> id >> checkin >> duration;
    clients.emplace_back(id, checkin, duration, waiters, mutex);
  }
  return clients;
}