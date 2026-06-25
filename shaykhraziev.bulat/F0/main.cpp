#include "app.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
  if (argc > 2)
  {
    std::cerr << "usage: lab [project-file]\n";
    return 1;
  }

  return shaykhraziev::runF0(argc == 2 ? argv[1] : nullptr, std::cin, std::cout, std::cerr);
}
