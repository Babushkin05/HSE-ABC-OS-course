#include "Administration.cpp"
#include "Client.cpp"
#include "generate_clients.cpp"
#include "manual.cpp"
#include "program_params.h"
#include "read_file.cpp"
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdio.h>
#include <string>

#define BAD_FLAGS                                                              \
  std::cerr << "ERROR :: you print bad flags";                                 \
  exit(1);

int rooms_count = 30;

std::queue<Client *> waiters;

pthread_rwlock_t waiters_mutex;

FILE *fptr;

// function that parse flags and rturns parameters for program
ProgramParams parse_args(int argc, char **argv) {
  std::vector<Client> clients;
  std::string out = "standart_out.txt";
  std::string source;
  bool is_rundom = false;
  // hadle every flag
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
  // contradiction
  if (source.size() == 0 != is_rundom) {
    BAD_FLAGS;
  }
  if (is_rundom) {
    clients = generateClients(waiters, waiters_mutex);
  } else {
    clients = read_clients_from_file(source, waiters, waiters_mutex);
  }
  // opens file
  fptr = fopen(out.c_str(), "w");
  if (fptr == NULL) {
    std::cerr << "ERROR :: could not open file";
    exit(0);
  }
  return ProgramParams{clients,
                       Administration{static_cast<int>(clients.size()),
                                      rooms_count, waiters, waiters_mutex,
                                      fptr},
                       out};
}

int main(int argc, char **argv) {
  // parse flags and get parameters for program
  ProgramParams params = parse_args(argc, argv);

  // creates threads
  for (size_t i = 0; i < params.clients.size(); ++i) {
    params.clients[i].start();
  }
  params.adm.start();

  // join threads
  for (size_t i = 0; i < params.clients.size(); ++i) {
    params.clients[i].wait();
  }
  params.adm.wait();

  //close file
  fclose(fptr);
}
