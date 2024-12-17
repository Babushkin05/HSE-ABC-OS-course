#pragma once
#include <vector>
#include "Client.cpp"
#include "Administration.cpp"
#include <string>

struct ProgramParams{
    std::vector<Client> clients; 
    Administration adm;
    std::string out;
};