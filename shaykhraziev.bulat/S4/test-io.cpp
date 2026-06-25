#include <boost/test/unit_test.hpp>

#include "io.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE(io_reads_valid_datasets)
{
  std::istringstream input("first 1 name 2 surname\nsecond 4 mouse 1 keyboard\n");

  shaykhraziev::DatasetTable datasets = shaykhraziev::readDatasets(input);

  BOOST_CHECK(datasets.has("first"));
  BOOST_CHECK(datasets.has("second"));
  BOOST_TEST(datasets.get("first").get(1) == "name");
  BOOST_TEST(datasets.get("first").get(2) == "surname");
  BOOST_TEST(datasets.get("second").get(4) == "mouse");
}

BOOST_AUTO_TEST_CASE(io_dataset_storage_keeps_named_dictionaries_in_order)
{
  shaykhraziev::DatasetStorage datasets;
  shaykhraziev::Dictionary beta;
  beta.push(2, "two");
  beta.push(1, "one");
  shaykhraziev::Dictionary alpha;
  alpha.push(3, "three");

  BOOST_CHECK(datasets.push("beta", beta));
  BOOST_CHECK(datasets.push("alpha", alpha));

  BOOST_CHECK(datasets.has("beta"));
  BOOST_TEST(datasets.get("beta").get(1) == "one");
  BOOST_TEST(datasets.get("beta").get(2) == "two");

  std::vector< std::string > names;
  for (shaykhraziev::DatasetStorage::const_iterator it = datasets.cbegin(); it != datasets.cend(); ++it)
  {
    names.push_back(it->key);
  }
  std::vector< std::string > expectedNames{"alpha", "beta"};
  BOOST_TEST(names == expectedNames, boost::test_tools::per_element());

  std::vector< int > keys;
  const shaykhraziev::Dictionary& dictionary = datasets.get("beta");
  for (shaykhraziev::Dictionary::const_iterator it = dictionary.cbegin(); it != dictionary.cend(); ++it)
  {
    keys.push_back(it->key);
  }
  std::vector< int > expectedKeys{1, 2};
  BOOST_TEST(keys == expectedKeys, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(io_ignores_empty_lines)
{
  std::istringstream input("\nfirst 1 one\n\nsecond\n");

  shaykhraziev::DatasetTable datasets = shaykhraziev::readDatasets(input);

  BOOST_CHECK(datasets.has("first"));
  BOOST_CHECK(datasets.has("second"));
  BOOST_CHECK(datasets.get("second").empty());
}

BOOST_AUTO_TEST_CASE(io_reads_empty_dictionary)
{
  std::istringstream input("empty\n");

  shaykhraziev::DatasetTable datasets = shaykhraziev::readDatasets(input);

  BOOST_CHECK(datasets.has("empty"));
  BOOST_CHECK(datasets.get("empty").empty());
}

BOOST_AUTO_TEST_CASE(io_rejects_odd_key_value_tail)
{
  std::istringstream input("bad 1 one 2\n");

  BOOST_CHECK_THROW(shaykhraziev::readDatasets(input), std::logic_error);
}

BOOST_AUTO_TEST_CASE(io_rejects_invalid_key)
{
  std::istringstream input("bad x value\n");

  BOOST_CHECK_THROW(shaykhraziev::readDatasets(input), std::logic_error);
}

BOOST_AUTO_TEST_CASE(io_rejects_duplicate_dataset_name)
{
  std::istringstream input("same 1 one\nsame 2 two\n");

  BOOST_CHECK_THROW(shaykhraziev::readDatasets(input), std::logic_error);
}

BOOST_AUTO_TEST_CASE(io_rejects_missing_file)
{
  BOOST_CHECK_THROW(shaykhraziev::readDatasetsFromFile("__missing_s4_file__"), std::runtime_error);
}
