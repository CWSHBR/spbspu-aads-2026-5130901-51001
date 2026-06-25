#include "io.hpp"

#include "../common/string-utils.hpp"

#include <istream>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{
  using shaykhraziev::List;

  const std::size_t INITIAL_GRAPH_SLOTS = 16;

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

  bool readNonEmptyLine(std::istream& in, std::string& line)
  {
    while (std::getline(in, line))
    {
      if (!shaykhraziev::splitTokens(line).empty())
      {
        return true;
      }
    }
    return false;
  }

  std::size_t parseCount(const std::string& token)
  {
    unsigned long long value = 0;
    if (!shaykhraziev::parseUnsigned(token, value) ||
        value > std::numeric_limits< std::size_t >::max())
    {
      throw std::logic_error("invalid graph file");
    }
    return static_cast< std::size_t >(value);
  }
}

shaykhraziev::GraphTable shaykhraziev::readGraphs(std::istream& in)
{
  GraphTable graphs(INITIAL_GRAPH_SLOTS);
  std::string line;
  while (readNonEmptyLine(in, line))
  {
    List< std::string > header = splitTokens(line);
    if (header.size() != 2)
    {
      throw std::logic_error("invalid graph file");
    }

    std::string name = tokenAt(header, 0);
    if (!isValidName(name) || graphs.has(name))
    {
      throw std::logic_error("invalid graph file");
    }
    std::size_t edgeCount = parseCount(tokenAt(header, 1));
    Graph graph;

    for (std::size_t i = 0; i < edgeCount; ++i)
    {
      if (!readNonEmptyLine(in, line))
      {
        throw std::logic_error("invalid graph file");
      }
      List< std::string > edge = splitTokens(line);
      if (edge.size() != 3)
      {
        throw std::logic_error("invalid graph file");
      }

      std::string from = tokenAt(edge, 0);
      std::string to = tokenAt(edge, 1);
      unsigned long long weight = 0;
      if (!isValidName(from) || !isValidName(to) || !parseUnsigned(tokenAt(edge, 2), weight))
      {
        throw std::logic_error("invalid graph file");
      }
      graph.bind(from, to, weight);
    }

    if (graphs.size() == graphs.capacity())
    {
      graphs.rehash(graphs.slots() * 2);
    }
    graphs.add(name, graph);
  }
  return graphs;
}
