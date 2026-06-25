#ifndef S3_GRAPH_HPP
#define S3_GRAPH_HPP

#include "../common/hash-functions.hpp"
#include "../common/hash-table.hpp"
#include "../common/list.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace shaykhraziev
{
  class Graph
  {
  public:
    using Weight = unsigned long long;
    using EdgeKey = std::pair< std::string, std::string >;

    struct WeightedVertex
    {
      std::string vertex;
      List< Weight > weights;
    };

    Graph();

    bool hasVertex(const std::string& vertex) const;
    void addVertex(const std::string& vertex);
    void bind(const std::string& from, const std::string& to, Weight weight);
    bool cut(const std::string& from, const std::string& to, Weight weight);
    List< std::string > getVertexes() const;
    List< WeightedVertex > getOutbound(const std::string& vertex) const;
    List< WeightedVertex > getInbound(const std::string& vertex) const;
    std::size_t vertexCount() const noexcept;
    std::size_t edgeKeyCount() const noexcept;
    Graph merge(const Graph& other) const;
    Graph extract(const List< std::string >& vertices) const;

  private:
    using VertexTable = HashTable< std::string, bool, HmacHash, StringEqual >;
    using EdgeHash = PairHash< HmacHash, HmacHash >;
    using EdgeEqual = PairEqual< StringEqual, StringEqual >;
    using EdgeTable = HashTable< EdgeKey, List< Weight >, EdgeHash, EdgeEqual >;

    VertexTable vertices_;
    EdgeTable edges_;

    void ensureVertexCapacity();
    void ensureEdgeCapacity();
  };
}

#endif
