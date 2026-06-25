#ifndef S4_COMMANDS_HPP
#define S4_COMMANDS_HPP

#include "io.hpp"

#include "../common/list.hpp"

#include <cstddef>
#include <iosfwd>
#include <string>

namespace shaykhraziev
{
  using CommandFunction = bool (*)(DatasetTable&, const List< std::string >&, std::ostream&);

  struct CommandHandler
  {
    std::size_t argumentCount;
    CommandFunction function;
  };

  using CommandRegistry = BSTree< std::string, CommandHandler, StringLess >;

  CommandRegistry makeCommandRegistry();
  bool executeCommandLine(DatasetTable& datasets, const std::string& line, std::ostream& out);
  void processCommands(DatasetTable& datasets, std::istream& in, std::ostream& out);
}

#endif
