#ifndef F0_COMMANDS_HPP
#define F0_COMMANDS_HPP

#include "project.hpp"

#include "../common/cuckoo-hash-table.hpp"
#include "../common/hash-functions.hpp"
#include "../common/list.hpp"

#include <cstddef>
#include <iosfwd>
#include <string>

namespace shaykhraziev
{
  using CommandFunction = bool (*)(ProjectStorage&, const List< std::string >&, const std::string&, std::ostream&);

  struct CommandHandler
  {
    std::size_t minArguments;
    std::size_t maxArguments;
    CommandFunction function;
    const char* usage;
    const char* description;
    const char* example;
  };

  using CommandRegistry = CuckooHashTable< std::string, CommandHandler, HmacHash, StringEqual >;

  CommandRegistry makeCommandRegistry();
  bool executeCommandLine(ProjectStorage& storage, const CommandRegistry& commands,
      const std::string& line, std::ostream& out);
  void processCommands(ProjectStorage& storage, const CommandRegistry& commands,
      std::istream& in, std::ostream& out);
}

#endif
