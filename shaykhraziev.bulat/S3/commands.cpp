#include "commands.hpp"

#include "../common/list-utils.hpp"
#include "../common/string-utils.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>

namespace
{
  using shaykhraziev::CommandContext;
  using shaykhraziev::Graph;
  using shaykhraziev::List;

  struct StringLess
  {
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
      return lhs < rhs;
    }
  };

  std::string tokenAt(const List< std::string >& tokens, std::size_t index)
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
    throw std::logic_error("missing token");
  }

  bool parseWeight(const std::string& token, Graph::Weight& weight)
  {
    unsigned long long parsed = 0;
    if (!shaykhraziev::parseUnsigned(token, parsed))
    {
      return false;
    }
    weight = parsed;
    return true;
  }

  void ensureGraphSpace(shaykhraziev::GraphTable& graphs)
  {
    if (graphs.size() == graphs.capacity())
    {
      graphs.rehash(graphs.slots() * 2);
    }
  }

  List< std::string > getGraphNames(const shaykhraziev::GraphTable& graphs)
  {
    List< std::string > names;
    for (auto it = graphs.begin(); it != graphs.end(); ++it)
    {
      shaykhraziev::insertSorted(names, it->key, StringLess());
    }
    return names;
  }

  void printLines(std::ostream& out, const List< std::string >& values)
  {
    if (values.empty())
    {
      out << '\n';
      return;
    }
    for (auto it = values.begin(); it != values.end(); ++it)
    {
      out << *it << '\n';
    }
  }

  void printAdjacent(std::ostream& out, const List< Graph::WeightedVertex >& values)
  {
    if (values.empty())
    {
      out << '\n';
      return;
    }
    for (auto it = values.begin(); it != values.end(); ++it)
    {
      out << it->vertex;
      for (auto weight = it->weights.begin(); weight != it->weights.end(); ++weight)
      {
        out << ' ' << *weight;
      }
      out << '\n';
    }
  }

  bool checkName(const std::string& name)
  {
    return shaykhraziev::isValidName(name);
  }

  bool graphsCommand(CommandContext& context, const List< std::string >& tokens, std::ostream& out)
  {
    if (tokens.size() != 1)
    {
      return false;
    }
    printLines(out, getGraphNames(context.graphs()));
    return true;
  }

  bool vertexesCommand(CommandContext& context, const List< std::string >& tokens, std::ostream& out)
  {
    if (tokens.size() != 2)
    {
      return false;
    }
    std::string graphName = tokenAt(tokens, 1);
    if (!checkName(graphName) || !context.graphs().has(graphName))
    {
      return false;
    }
    printLines(out, context.graphs().get(graphName).getVertexes());
    return true;
  }

  bool outboundCommand(CommandContext& context, const List< std::string >& tokens, std::ostream& out)
  {
    if (tokens.size() != 3)
    {
      return false;
    }
    std::string graphName = tokenAt(tokens, 1);
    std::string vertex = tokenAt(tokens, 2);
    if (!checkName(graphName) || !checkName(vertex) || !context.graphs().has(graphName))
    {
      return false;
    }
    const Graph& graph = context.graphs().get(graphName);
    if (!graph.hasVertex(vertex))
    {
      return false;
    }
    printAdjacent(out, graph.getOutbound(vertex));
    return true;
  }

  bool inboundCommand(CommandContext& context, const List< std::string >& tokens, std::ostream& out)
  {
    if (tokens.size() != 3)
    {
      return false;
    }
    std::string graphName = tokenAt(tokens, 1);
    std::string vertex = tokenAt(tokens, 2);
    if (!checkName(graphName) || !checkName(vertex) || !context.graphs().has(graphName))
    {
      return false;
    }
    const Graph& graph = context.graphs().get(graphName);
    if (!graph.hasVertex(vertex))
    {
      return false;
    }
    printAdjacent(out, graph.getInbound(vertex));
    return true;
  }

  bool bindCommand(CommandContext& context, const List< std::string >& tokens, std::ostream&)
  {
    if (tokens.size() != 5)
    {
      return false;
    }
    std::string graphName = tokenAt(tokens, 1);
    std::string from = tokenAt(tokens, 2);
    std::string to = tokenAt(tokens, 3);
    Graph::Weight weight = 0;
    if (!checkName(graphName) || !checkName(from) || !checkName(to) ||
        !parseWeight(tokenAt(tokens, 4), weight) || !context.graphs().has(graphName))
    {
      return false;
    }
    context.graphs().get(graphName).bind(from, to, weight);
    return true;
  }

  bool cutCommand(CommandContext& context, const List< std::string >& tokens, std::ostream&)
  {
    if (tokens.size() != 5)
    {
      return false;
    }
    std::string graphName = tokenAt(tokens, 1);
    std::string from = tokenAt(tokens, 2);
    std::string to = tokenAt(tokens, 3);
    Graph::Weight weight = 0;
    if (!checkName(graphName) || !checkName(from) || !checkName(to) ||
        !parseWeight(tokenAt(tokens, 4), weight) || !context.graphs().has(graphName))
    {
      return false;
    }
    return context.graphs().get(graphName).cut(from, to, weight);
  }

  bool createCommand(CommandContext& context, const List< std::string >& tokens, std::ostream&)
  {
    if (tokens.size() < 2)
    {
      return false;
    }
    std::string graphName = tokenAt(tokens, 1);
    if (!checkName(graphName) || context.graphs().has(graphName))
    {
      return false;
    }

    Graph graph;
    if (tokens.size() > 2)
    {
      unsigned long long count = 0;
      if (!shaykhraziev::parseUnsigned(tokenAt(tokens, 2), count) || tokens.size() != count + 3)
      {
        return false;
      }
      for (std::size_t i = 0; i < count; ++i)
      {
        std::string vertex = tokenAt(tokens, i + 3);
        if (!checkName(vertex))
        {
          return false;
        }
        graph.addVertex(vertex);
      }
    }

    ensureGraphSpace(context.graphs());
    context.graphs().add(graphName, graph);
    return true;
  }

  bool mergeCommand(CommandContext& context, const List< std::string >& tokens, std::ostream&)
  {
    if (tokens.size() != 4)
    {
      return false;
    }
    std::string newName = tokenAt(tokens, 1);
    std::string leftName = tokenAt(tokens, 2);
    std::string rightName = tokenAt(tokens, 3);
    if (!checkName(newName) || !checkName(leftName) || !checkName(rightName) ||
        context.graphs().has(newName) || !context.graphs().has(leftName) || !context.graphs().has(rightName))
    {
      return false;
    }
    Graph graph = context.graphs().get(leftName).merge(context.graphs().get(rightName));
    ensureGraphSpace(context.graphs());
    context.graphs().add(newName, graph);
    return true;
  }

  bool extractCommand(CommandContext& context, const List< std::string >& tokens, std::ostream&)
  {
    if (tokens.size() < 4)
    {
      return false;
    }
    std::string newName = tokenAt(tokens, 1);
    std::string oldName = tokenAt(tokens, 2);
    unsigned long long count = 0;
    if (!checkName(newName) || !checkName(oldName) || context.graphs().has(newName) ||
        !context.graphs().has(oldName) || !shaykhraziev::parseUnsigned(tokenAt(tokens, 3), count) ||
        tokens.size() != count + 4)
    {
      return false;
    }

    const Graph& source = context.graphs().get(oldName);
    List< std::string > vertices;
    for (std::size_t i = 0; i < count; ++i)
    {
      std::string vertex = tokenAt(tokens, i + 4);
      if (!checkName(vertex) || !source.hasVertex(vertex))
      {
        return false;
      }
      vertices.pushBack(vertex);
    }

    Graph graph = source.extract(vertices);
    ensureGraphSpace(context.graphs());
    context.graphs().add(newName, graph);
    return true;
  }
}

shaykhraziev::CommandContext::CommandContext(GraphTable& graphs):
  graphs_(graphs)
{}

shaykhraziev::GraphTable& shaykhraziev::CommandContext::graphs()
{
  return graphs_;
}

const shaykhraziev::GraphTable& shaykhraziev::CommandContext::graphs() const
{
  return graphs_;
}

shaykhraziev::CommandTable shaykhraziev::createCommandTable()
{
  CommandTable commands(4, 4);
  commands.add("graphs", graphsCommand);
  commands.add("vertexes", vertexesCommand);
  commands.add("outbound", outboundCommand);
  commands.add("inbound", inboundCommand);
  commands.add("bind", bindCommand);
  commands.add("cut", cutCommand);
  commands.add("create", createCommand);
  commands.add("merge", mergeCommand);
  commands.add("extract", extractCommand);
  return commands;
}

bool shaykhraziev::executeCommandLine(CommandContext& context, const CommandTable& commands,
    const std::string& line, std::ostream& out)
{
  List< std::string > tokens = splitTokens(line);
  if (tokens.empty())
  {
    return true;
  }

  const CommandHandler* handler = commands.find(tokenAt(tokens, 0));
  if (!handler)
  {
    out << "<INVALID COMMAND>\n";
    return false;
  }

  bool ok = false;
  try
  {
    ok = (*handler)(context, tokens, out);
  }
  catch (const std::exception&)
  {
    ok = false;
  }
  if (!ok)
  {
    out << "<INVALID COMMAND>\n";
  }
  return ok;
}

void shaykhraziev::processCommands(CommandContext& context, const CommandTable& commands,
    std::istream& in, std::ostream& out)
{
  std::string line;
  while (std::getline(in, line))
  {
    executeCommandLine(context, commands, line, out);
  }
}
