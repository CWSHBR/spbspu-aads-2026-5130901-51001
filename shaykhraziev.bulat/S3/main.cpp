#include "commands.hpp"
#include "io.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "filename required\n";
    return 1;
  }

  std::ifstream file(argv[1]);
  if (!file)
  {
    std::cerr << "cannot open file\n";
    return 1;
  }

  try
  {
    shaykhraziev::GraphTable graphs = shaykhraziev::readGraphs(file);
    shaykhraziev::CommandContext context(graphs);
    shaykhraziev::CommandTable commands = shaykhraziev::createCommandTable();
    shaykhraziev::processCommands(context, commands, std::cin, std::cout);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 2;
  }

  return 0;
}
