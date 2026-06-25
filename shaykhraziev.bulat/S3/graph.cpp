#include "graph.hpp"

#include "../common/list-utils.hpp"

namespace
{
  struct StringLess
  {
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
      return lhs < rhs;
    }
  };

  struct WeightLess
  {
    bool operator()(shaykhraziev::Graph::Weight lhs, shaykhraziev::Graph::Weight rhs) const
    {
      return lhs < rhs;
    }
  };

  struct WeightedVertexLess
  {
    bool operator()(const shaykhraziev::Graph::WeightedVertex& lhs,
        const shaykhraziev::Graph::WeightedVertex& rhs) const
    {
      return lhs.vertex < rhs.vertex;
    }
  };

  shaykhraziev::Graph::WeightedVertex* findWeighted(
      shaykhraziev::List< shaykhraziev::Graph::WeightedVertex >& list,
      const std::string& vertex)
  {
    for (auto it = list.begin(); it != list.end(); ++it)
    {
      if (it->vertex == vertex)
      {
        return &(*it);
      }
    }
    return nullptr;
  }

  void addWeighted(shaykhraziev::List< shaykhraziev::Graph::WeightedVertex >& list,
      const std::string& vertex,
      shaykhraziev::Graph::Weight weight)
  {
    shaykhraziev::Graph::WeightedVertex* item = findWeighted(list, vertex);
    if (item)
    {
      shaykhraziev::insertSorted(item->weights, weight, WeightLess());
      return;
    }

    shaykhraziev::Graph::WeightedVertex created;
    created.vertex = vertex;
    created.weights.pushBack(weight);
    shaykhraziev::insertSorted(list, created, WeightedVertexLess());
  }

  bool removeWeight(shaykhraziev::List< shaykhraziev::Graph::Weight >& weights,
      shaykhraziev::Graph::Weight weight)
  {
    shaykhraziev::List< shaykhraziev::Graph::Weight > result;
    bool removed = false;
    for (auto it = weights.begin(); it != weights.end(); ++it)
    {
      if (!removed && *it == weight)
      {
        removed = true;
      }
      else
      {
        result.pushBack(*it);
      }
    }
    if (removed)
    {
      weights = result;
    }
    return removed;
  }
}

shaykhraziev::Graph::Graph():
  vertices_(16),
  edges_(16)
{}

bool shaykhraziev::Graph::hasVertex(const std::string& vertex) const
{
  return vertices_.has(vertex);
}

void shaykhraziev::Graph::addVertex(const std::string& vertex)
{
  if (vertices_.has(vertex))
  {
    return;
  }
  ensureVertexCapacity();
  vertices_.add(vertex, true);
}

void shaykhraziev::Graph::bind(const std::string& from, const std::string& to, Weight weight)
{
  addVertex(from);
  addVertex(to);

  EdgeKey key(from, to);
  List< Weight >* weights = edges_.find(key);
  if (!weights)
  {
    ensureEdgeCapacity();
    List< Weight > created;
    created.pushBack(weight);
    edges_.add(key, created);
    return;
  }
  weights->pushBack(weight);
}

bool shaykhraziev::Graph::cut(const std::string& from, const std::string& to, Weight weight)
{
  EdgeKey key(from, to);
  List< Weight >* weights = edges_.find(key);
  if (!weights)
  {
    return false;
  }
  if (!removeWeight(*weights, weight))
  {
    return false;
  }
  if (weights->empty())
  {
    edges_.drop(key);
  }
  return true;
}

shaykhraziev::List< std::string > shaykhraziev::Graph::getVertexes() const
{
  List< std::string > result;
  for (auto it = vertices_.begin(); it != vertices_.end(); ++it)
  {
    insertSorted(result, it->key, StringLess());
  }
  return result;
}

shaykhraziev::List< shaykhraziev::Graph::WeightedVertex >
    shaykhraziev::Graph::getOutbound(const std::string& vertex) const
{
  List< WeightedVertex > result;
  for (auto it = edges_.begin(); it != edges_.end(); ++it)
  {
    if (it->key.first == vertex)
    {
      for (auto w = it->value.begin(); w != it->value.end(); ++w)
      {
        addWeighted(result, it->key.second, *w);
      }
    }
  }
  return result;
}

shaykhraziev::List< shaykhraziev::Graph::WeightedVertex >
    shaykhraziev::Graph::getInbound(const std::string& vertex) const
{
  List< WeightedVertex > result;
  for (auto it = edges_.begin(); it != edges_.end(); ++it)
  {
    if (it->key.second == vertex)
    {
      for (auto w = it->value.begin(); w != it->value.end(); ++w)
      {
        addWeighted(result, it->key.first, *w);
      }
    }
  }
  return result;
}

std::size_t shaykhraziev::Graph::vertexCount() const noexcept
{
  return vertices_.size();
}

std::size_t shaykhraziev::Graph::edgeKeyCount() const noexcept
{
  return edges_.size();
}

shaykhraziev::Graph shaykhraziev::Graph::merge(const Graph& other) const
{
  Graph result;
  for (auto it = vertices_.begin(); it != vertices_.end(); ++it)
  {
    result.addVertex(it->key);
  }
  for (auto it = other.vertices_.begin(); it != other.vertices_.end(); ++it)
  {
    result.addVertex(it->key);
  }
  for (auto it = edges_.begin(); it != edges_.end(); ++it)
  {
    for (auto w = it->value.begin(); w != it->value.end(); ++w)
    {
      result.bind(it->key.first, it->key.second, *w);
    }
  }
  for (auto it = other.edges_.begin(); it != other.edges_.end(); ++it)
  {
    for (auto w = it->value.begin(); w != it->value.end(); ++w)
    {
      result.bind(it->key.first, it->key.second, *w);
    }
  }
  return result;
}

shaykhraziev::Graph shaykhraziev::Graph::extract(const List< std::string >& vertices) const
{
  Graph result;
  for (auto it = vertices.begin(); it != vertices.end(); ++it)
  {
    if (hasVertex(*it))
    {
      result.addVertex(*it);
    }
  }
  for (auto it = edges_.begin(); it != edges_.end(); ++it)
  {
    if (result.hasVertex(it->key.first) && result.hasVertex(it->key.second))
    {
      for (auto w = it->value.begin(); w != it->value.end(); ++w)
      {
        result.bind(it->key.first, it->key.second, *w);
      }
    }
  }
  return result;
}

void shaykhraziev::Graph::ensureVertexCapacity()
{
  if (vertices_.size() == vertices_.capacity())
  {
    vertices_.rehash(vertices_.slots() * 2);
  }
}

void shaykhraziev::Graph::ensureEdgeCapacity()
{
  if (edges_.size() == edges_.capacity())
  {
    edges_.rehash(edges_.slots() * 2);
  }
}
