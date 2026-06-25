#include "app.hpp"

#include "commands.hpp"
#include "io.hpp"

#include <iostream>
#include <stdexcept>

int shaykhraziev::runS4(const char* filename, std::istream& in, std::ostream& out, std::ostream& err)
{
  try
  {
    DatasetTable datasets = readDatasetsFromFile(filename);
    processCommands(datasets, in, out);
  }
  catch (const std::runtime_error& e)
  {
    err << e.what() << '\n';
    return 1;
  }
  catch (const std::logic_error& e)
  {
    err << e.what() << '\n';
    return 1;
  }
  catch (const std::exception& e)
  {
    err << e.what() << '\n';
    return 2;
  }
  return 0;
}
