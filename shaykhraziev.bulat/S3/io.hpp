#ifndef S3_IO_HPP
#define S3_IO_HPP

#include "graph.hpp"

#include "../common/hash-functions.hpp"
#include "../common/hash-table.hpp"

#include <iosfwd>
#include <string>

namespace shaykhraziev
{
  using GraphTable = HashTable< std::string, Graph, HmacHash, StringEqual >;

  GraphTable readGraphs(std::istream& in);
}

#endif
