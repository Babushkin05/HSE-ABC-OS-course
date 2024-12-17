#include "Administration.cpp"
#include "Client.cpp"
#include "program_params.h"
#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include "manual.cpp"

#define BAD_FLAGS                                                              \
  std::cerr << "you print bad flags";                                          \
  exit(1);

int rooms_count = 30;

std::queue<Client *> waiters;

pthread_mutex_t waiters_mutex;

std::vector<Client> generateClients(){
    
}

void parse_args(int argc, char **argv) {
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
  if(out.size()==0 != is_rundom){
    BAD_FLAGS;
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
