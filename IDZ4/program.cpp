#include "Administration.cpp"
#include "Client.cpp"
#include "program_params.h"
#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include "manual.cpp"
#include "generate_clients.cpp"
#include "read_file.cpp"

#define BAD_FLAGS                                                              \
  std::cerr << "you print bad flags";                                          \
  exit(1);

int rooms_count = 30;

std::queue<Client *> waiters;

pthread_mutex_t waiters_mutex;


void parse_args(int argc, char **argv) {
    std::vector<Client> clients;
  std::string out;
  std::string source;
  bool is_rundom = false;
  for (size_t i = 0; i < argc; ++i) {
    std::string param = argv[i];
    if (param == "-o") {
      if (i + 1 < argc) {
        ++i;
        out = argv[i];
      } else {
        BAD_FLAGS
      }
    } else if (param == "-s") {
      if (i + 1 < argc) {
        ++i;
        source = argv[i];
      } else {
        BAD_FLAGS
      }
    } else if (param == "-r") {
      is_rundom = true;
    } else if (param == "-h") {
      print_manual();
      exit(0);
    }
  }
  if(source.size()==0 != is_rundom){
    BAD_FLAGS;
  }
  if(is_rundom){
    clients = generateClients(waiters, waiters_mutex);
  }
  else{
    clients = read_clients_from_file(source, waiters, waiters_mutex);
  }
    
}

int main(int argc, char **argv) {

    parse_args(argc, argv);
  Client c1(1, 2, 2, waiters, waiters_mutex);
  Client c2(2, 3, 1, waiters, waiters_mutex);
  Client c3(3, 0, 3, waiters, waiters_mutex);

  Administration adm(10, rooms_count, waiters, waiters_mutex);
  c1.start();
  c2.start();
  c3.start();
  adm.start();
  c1.wait();
  c2.wait();
  c3.wait();
  adm.wait();
}
