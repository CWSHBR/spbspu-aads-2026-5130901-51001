#include <boost/test/unit_test.hpp>

#include "dictionary-operations.hpp"

namespace
{
  shaykhraziev::Dictionary makeLeft()
  {
    shaykhraziev::Dictionary dict;
    dict.push(1, "one");
    dict.push(2, "two-left");
    dict.push(3, "three");
    return dict;
  }

  shaykhraziev::Dictionary makeRight()
  {
    shaykhraziev::Dictionary dict;
    dict.push(2, "two-right");
    dict.push(4, "four");
    return dict;
  }
}

BOOST_AUTO_TEST_CASE(dictionary_complement_keeps_left_unique_keys)
{
  shaykhraziev::Dictionary result = shaykhraziev::complementDictionaries(makeLeft(), makeRight());

  BOOST_CHECK(result.has(1));
  BOOST_CHECK(!result.has(2));
  BOOST_CHECK(result.has(3));
  BOOST_TEST(result.size() == 2);
}

BOOST_AUTO_TEST_CASE(dictionary_intersect_keeps_common_keys_from_left)
{
  shaykhraziev::Dictionary result = shaykhraziev::intersectDictionaries(makeLeft(), makeRight());

  BOOST_TEST(result.size() == 1);
  BOOST_CHECK(result.has(2));
  BOOST_TEST(result.get(2) == "two-left");
}

BOOST_AUTO_TEST_CASE(dictionary_union_keeps_all_keys_and_left_wins)
{
  shaykhraziev::Dictionary result = shaykhraziev::uniteDictionaries(makeLeft(), makeRight());

  BOOST_TEST(result.size() == 4);
  BOOST_TEST(result.get(1) == "one");
  BOOST_TEST(result.get(2) == "two-left");
  BOOST_TEST(result.get(3) == "three");
  BOOST_TEST(result.get(4) == "four");
}

BOOST_AUTO_TEST_CASE(dictionary_operations_support_empty_inputs)
{
  shaykhraziev::Dictionary empty;
  shaykhraziev::Dictionary right = makeRight();

  BOOST_CHECK(shaykhraziev::complementDictionaries(empty, right).empty());
  BOOST_CHECK(shaykhraziev::intersectDictionaries(empty, right).empty());
  BOOST_TEST(shaykhraziev::uniteDictionaries(empty, right).size() == 2);
}
