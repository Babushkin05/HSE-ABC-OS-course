#pragma once
#include <vector>
#include "Client.cpp"
#include "Administration.cpp"
#include <string>

class ProgramParams{
    public:
    std::vector<Client> clients;
    Administration adm;
    std::string out;
};