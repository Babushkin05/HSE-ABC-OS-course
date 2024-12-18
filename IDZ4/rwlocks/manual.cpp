#include <iostream>

void print_manual() {
  // Print the header
  std::cout << "\n\n\tWelcome to Socialistic Hotel Modulation Manual\n";
  std::cout << "--------------------------------------------------------------\n";

  // Print the options
  std::cout << "\t\t-o <file> : output to file\n";
  std::cout << "\t\t-s <file> : read data from file\n";
  std::cout << "\t\t-r : generate data randomly\n";
  std::cout << "\t\t-h : writes this manual\n";
  std::cout
      << "---------------------------------------------------------------\n";

  // Print the data writing instructions
  std::cout << "\t\tHow to Write Data:\n";
  std::cout << "\t\t\t0 1 2\n";
  std::cout << "hotel client with id=0, who comes in the day=1 and live for "
               "2 days\n\n\n";
}