#include "app.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

#include <string-utils.hpp>

#include "commands.hpp"
#include "io.hpp"

namespace
{
  std::string tokenAt(const shaykhraziev::List< std::string >& tokens, std::size_t index)
  {
    std::size_t current = 0;
    for (shaykhraziev::List< std::string >::const_iterator it = tokens.cbegin(); it != tokens.cend(); ++it)
    {
      if (current == index)
      {
        return *it;
      }
      ++current;
    }
    return "";
  }

  bool isSavedCommand(const std::string& command)
  {
    return command == "make-project" ||
        command == "drop-project" ||
        command == "add-task" ||
        command == "drop-task" ||
        command == "add-dependency" ||
        command == "drop-dependency";
  }
}

int shaykhraziev::runF0(const char* filename, std::istream& in, std::ostream& out, std::ostream& err)
{
  try
  {
    ProjectStorage storage = filename ? readProjectsFromFile(filename) : ProjectStorage();
    std::string saveFilename = filename ? filename : "";
    CommandRegistry commands = makeCommandRegistry();
    std::string line;
    out << " > ";
    while (std::getline(in, line))
    {
      const List< std::string > tokens = splitTokens(line);
      const std::string command = tokens.empty() ? "" : tokenAt(tokens, 0);
      const bool ok = executeCommandLine(storage, commands, line, out);
      if (ok && isSavedCommand(command))
      {
        if (saveFilename.empty() && command == "make-project")
        {
          saveFilename = tokenAt(tokens, 1);
        }
        if (!saveFilename.empty())
        {
          writeProjectsToFile(storage, saveFilename);
        }
      }
      out << " > ";
    }
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
