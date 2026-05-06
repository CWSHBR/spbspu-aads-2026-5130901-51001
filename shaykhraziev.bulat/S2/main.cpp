#include "expression.hpp"
#include "stack.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  using shaykhraziev::Stack;
  using shaykhraziev::evaluateExpression;

  if (argc > 2)
  {
    std::cerr << "wrong number of arguments\n";
    return 1;
  }

  std::ifstream file;
  std::istream* input = &std::cin;
  if (argc == 2)
  {
    file.open(argv[1]);
    if (!file)
    {
      std::cerr << "cannot open file\n";
      return 1;
    }
    input = &file;
  }

  Stack< long long > results;
  std::string line;

  try
  {
    while (std::getline(*input, line))
    {
      if (line.empty())
      {
        continue;
      }
      results.push(evaluateExpression(line));
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }

  bool first = true;
  while (!results.empty())
  {
    if (!first)
    {
      std::cout << ' ';
    }
    std::cout << results.drop();
    first = false;
  }
  std::cout << '\n';

  return 0;
}
