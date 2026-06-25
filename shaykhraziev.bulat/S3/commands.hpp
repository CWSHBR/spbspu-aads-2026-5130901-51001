#ifndef S3_COMMANDS_HPP
#define S3_COMMANDS_HPP

#include "io.hpp"

#include <functional>
#include <iosfwd>
#include <string>

namespace shaykhraziev
{
  class CommandContext
  {
  public:
    explicit CommandContext(GraphTable& graphs);

    GraphTable& graphs();
    const GraphTable& graphs() const;

  private:
    GraphTable& graphs_;
  };

  using CommandHandler = std::function< bool(CommandContext&, const List< std::string >&, std::ostream&) >;
  using CommandTable = HashTable< std::string, CommandHandler, HmacHash, StringEqual >;

  CommandTable createCommandTable();
  bool executeCommandLine(CommandContext& context, const CommandTable& commands,
      const std::string& line, std::ostream& out);
  void processCommands(CommandContext& context, const CommandTable& commands,
      std::istream& in, std::ostream& out);
}

#endif
