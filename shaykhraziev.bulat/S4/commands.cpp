#include "commands.hpp"

#include "dictionary-operations.hpp"

#include "../common/string-utils.hpp"

#include <istream>
#include <ostream>
#include <string>

namespace
{
  std::string tokenAt(const shaykhraziev::List< std::string >& tokens, std::size_t index)
  {
    std::size_t current = 0;
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
      if (current == index)
      {
        return *it;
      }
      ++current;
    }
    return "";
  }

  void printInvalid(std::ostream& out)
  {
    out << "<INVALID COMMAND>\n";
  }

  void printDictionary(
      const std::string& name,
      const shaykhraziev::Dictionary& dictionary,
      std::ostream& out)
  {
    if (dictionary.empty())
    {
      out << "<EMPTY>\n";
      return;
    }

    out << name;
    for (shaykhraziev::Dictionary::const_iterator it = dictionary.cbegin();
        it != dictionary.cend();
        ++it)
    {
      out << ' ' << it->key << ' ' << it->value;
    }
    out << '\n';
  }

  bool executePrint(
      shaykhraziev::DatasetTable& datasets,
      const shaykhraziev::List< std::string >& tokens,
      std::ostream& out)
  {
    if (tokens.size() != 2)
    {
      return false;
    }
    const std::string name = tokenAt(tokens, 1);
    if (!datasets.has(name))
    {
      return false;
    }
    printDictionary(name, datasets.get(name), out);
    return true;
  }

  bool addResult(
      shaykhraziev::DatasetTable& datasets,
      const std::string& newName,
      const shaykhraziev::Dictionary& result)
  {
    datasets.push(newName, result);
    return true;
  }

  bool executeSetOperation(
      shaykhraziev::DatasetTable& datasets,
      const shaykhraziev::List< std::string >& tokens,
      shaykhraziev::Dictionary (*operation)(
          const shaykhraziev::Dictionary&,
          const shaykhraziev::Dictionary&) = nullptr)
  {
    if (!operation)
    {
      return false;
    }
    const std::string newName = tokenAt(tokens, 1);
    const std::string leftName = tokenAt(tokens, 2);
    const std::string rightName = tokenAt(tokens, 3);
    if (!datasets.has(leftName) || !datasets.has(rightName))
    {
      return false;
    }

    const shaykhraziev::Dictionary& left = datasets.get(leftName);
    const shaykhraziev::Dictionary& right = datasets.get(rightName);
    return addResult(datasets, newName, operation(left, right));
  }

  bool executeComplement(
      shaykhraziev::DatasetTable& datasets,
      const shaykhraziev::List< std::string >& tokens,
      std::ostream&)
  {
    return executeSetOperation(datasets, tokens, shaykhraziev::complementDictionaries);
  }

  bool executeIntersect(
      shaykhraziev::DatasetTable& datasets,
      const shaykhraziev::List< std::string >& tokens,
      std::ostream&)
  {
    return executeSetOperation(datasets, tokens, shaykhraziev::intersectDictionaries);
  }

  bool executeUnion(
      shaykhraziev::DatasetTable& datasets,
      const shaykhraziev::List< std::string >& tokens,
      std::ostream&)
  {
    return executeSetOperation(datasets, tokens, shaykhraziev::uniteDictionaries);
  }
}

shaykhraziev::CommandRegistry shaykhraziev::makeCommandRegistry()
{
  CommandRegistry registry;
  registry.push("print", CommandHandler{1, executePrint});
  registry.push("complement", CommandHandler{3, executeComplement});
  registry.push("intersect", CommandHandler{3, executeIntersect});
  registry.push("union", CommandHandler{3, executeUnion});
  return registry;
}

bool shaykhraziev::executeCommandLine(
    DatasetTable& datasets,
    const std::string& line,
    std::ostream& out)
{
  List< std::string > tokens = splitTokens(line);
  if (tokens.empty())
  {
    return true;
  }

  const std::string command = tokenAt(tokens, 0);
  CommandRegistry registry = makeCommandRegistry();
  bool ok = registry.has(command);
  if (ok)
  {
    const CommandHandler& handler = registry.get(command);
    ok = tokens.size() == handler.argumentCount + 1 && handler.function(datasets, tokens, out);
  }

  if (!ok)
  {
    printInvalid(out);
  }
  return ok;
}

void shaykhraziev::processCommands(DatasetTable& datasets, std::istream& in, std::ostream& out)
{
  std::string line;
  while (std::getline(in, line))
  {
    executeCommandLine(datasets, line, out);
  }
}
