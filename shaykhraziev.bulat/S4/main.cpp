#include "app.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "filename required\n";
    return 1;
  }

  return shaykhraziev::runS4(argv[1], std::cin, std::cout, std::cerr);
}
