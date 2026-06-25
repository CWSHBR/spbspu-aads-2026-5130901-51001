#include <boost/test/unit_test.hpp>

#include "io.hpp"

#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE(io_reads_single_graph)
{
  std::istringstream input("g1 1\na b 3\n");

  shaykhraziev::GraphTable graphs = shaykhraziev::readGraphs(input);

  BOOST_CHECK(graphs.has("g1"));
  BOOST_TEST(graphs.get("g1").vertexCount() == 2);
}

BOOST_AUTO_TEST_CASE(io_reads_multiple_graphs_and_ignores_empty_lines)
{
  std::istringstream input("\n\ng1 0\n\n g2 1\nb b 2\n");

  shaykhraziev::GraphTable graphs = shaykhraziev::readGraphs(input);

  BOOST_CHECK(graphs.has("g1"));
  BOOST_CHECK(graphs.has("g2"));
  BOOST_TEST(graphs.get("g2").edgeKeyCount() == 1);
}

BOOST_AUTO_TEST_CASE(io_preserves_parallel_edges)
{
  std::istringstream input("g1 2\na b 1\na b 1\n");

  shaykhraziev::GraphTable graphs = shaykhraziev::readGraphs(input);

  auto outbound = graphs.get("g1").getOutbound("a");
  BOOST_REQUIRE(outbound.begin() != outbound.end());
  BOOST_TEST(outbound.begin()->weights.size() == 2);
}

BOOST_AUTO_TEST_CASE(io_rejects_invalid_graph_file)
{
  std::istringstream badName("1g 0\n");
  std::istringstream missingEdge("g 1\n");
  std::istringstream badWeight("g 1\na b -1\n");

  BOOST_CHECK_THROW(shaykhraziev::readGraphs(badName), std::logic_error);
  BOOST_CHECK_THROW(shaykhraziev::readGraphs(missingEdge), std::logic_error);
  BOOST_CHECK_THROW(shaykhraziev::readGraphs(badWeight), std::logic_error);
}
