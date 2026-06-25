#include <boost/test/unit_test.hpp>

#include "../common/hash-functions.hpp"
#include "../common/hash-table.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace
{
  using Table = shaykhraziev::HashTable< std::string, int, shaykhraziev::HmacHash, shaykhraziev::StringEqual >;

  struct ConstantHash
  {
    std::size_t operator()(const std::string&) const
    {
      return 0;
    }
  };

  using CollisionTable = shaykhraziev::HashTable< std::string, int, ConstantHash, shaykhraziev::StringEqual >;
}

BOOST_AUTO_TEST_CASE(hash_table_default_state_is_empty)
{
  Table table(3);

  BOOST_CHECK(table.empty());
  BOOST_TEST(table.size() == 0);
  BOOST_TEST(table.slots() == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_capacity_is_slots_multiplied_by_bucket_size)
{
  Table table(5, 2);

  BOOST_TEST(table.capacity() == 10);
  BOOST_TEST(table.bucketSize() == 2);
}

BOOST_AUTO_TEST_CASE(hash_table_clear_keeps_capacity)
{
  Table table(2, 3);

  table.clear();

  BOOST_CHECK(table.empty());
  BOOST_TEST(table.capacity() == 6);
}

BOOST_AUTO_TEST_CASE(hash_table_rejects_invalid_capacity)
{
  BOOST_CHECK_THROW(Table(0), std::invalid_argument);
  BOOST_CHECK_THROW(Table(1, 0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(hash_table_add_and_get_store_values)
{
  Table table(4);

  BOOST_CHECK(table.add("one", 1));
  BOOST_CHECK(table.has("one"));
  BOOST_TEST(table.get("one") == 1);
  BOOST_TEST(table.size() == 1);
}

BOOST_AUTO_TEST_CASE(hash_table_add_rejects_duplicate_key)
{
  Table table(2);

  table.add("same", 1);

  BOOST_CHECK_THROW(table.add("same", 2), std::logic_error);
  BOOST_TEST(table.get("same") == 1);
}

BOOST_AUTO_TEST_CASE(hash_table_set_replaces_or_inserts)
{
  Table table(2);

  BOOST_CHECK(table.set("value", 1));
  BOOST_CHECK(!table.set("value", 2));

  BOOST_TEST(table.get("value") == 2);
  BOOST_TEST(table.size() == 1);
}

BOOST_AUTO_TEST_CASE(hash_table_drop_existing_and_missing_keys)
{
  Table table(2);

  table.add("one", 1);

  BOOST_CHECK(table.drop("one"));
  BOOST_CHECK(!table.drop("one"));
  BOOST_CHECK(!table.has("one"));
  BOOST_CHECK_THROW(table.get("one"), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(hash_table_handles_bucket_collisions)
{
  CollisionTable table(2, 2);

  table.add("a", 1);
  table.add("b", 2);
  table.add("c", 3);

  BOOST_TEST(table.get("a") == 1);
  BOOST_TEST(table.get("b") == 2);
  BOOST_TEST(table.get("c") == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_reports_overflow_when_full)
{
  CollisionTable table(1, 2);

  table.add("a", 1);
  table.add("b", 2);

  BOOST_CHECK_THROW(table.add("c", 3), std::overflow_error);
}

BOOST_AUTO_TEST_CASE(hash_table_find_returns_pointer_or_null)
{
  Table table(2);

  table.add("one", 1);

  BOOST_REQUIRE(table.find("one") != nullptr);
  BOOST_TEST(*table.find("one") == 1);
  BOOST_CHECK(table.find("two") == nullptr);
}

BOOST_AUTO_TEST_CASE(hash_table_iterator_visits_all_entries)
{
  Table table(4);
  table.add("one", 1);
  table.add("two", 2);
  table.add("three", 3);

  int sum = 0;
  std::size_t count = 0;
  for (Table::iterator it = table.begin(); it != table.end(); ++it)
  {
    sum += it->value;
    ++count;
  }

  BOOST_TEST(count == 3);
  BOOST_TEST(sum == 6);
}

BOOST_AUTO_TEST_CASE(hash_table_const_iterator_visits_all_entries)
{
  Table table(4);
  table.add("one", 1);
  table.add("two", 2);
  const Table& constTable = table;

  int sum = 0;
  for (Table::const_iterator it = constTable.begin(); it != constTable.end(); ++it)
  {
    sum += it->value;
  }

  BOOST_TEST(sum == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_rehash_preserves_elements)
{
  CollisionTable table(1, 4);
  table.add("a", 1);
  table.add("b", 2);
  table.add("c", 3);

  table.rehash(4);

  BOOST_TEST(table.slots() == 4);
  BOOST_TEST(table.get("a") == 1);
  BOOST_TEST(table.get("b") == 2);
  BOOST_TEST(table.get("c") == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_copy_is_independent)
{
  Table table(4);
  table.add("one", 1);
  table.add("two", 2);

  Table copy(table);
  copy.set("one", 10);
  copy.add("three", 3);

  BOOST_TEST(table.get("one") == 1);
  BOOST_CHECK(!table.has("three"));
  BOOST_TEST(copy.get("one") == 10);
  BOOST_TEST(copy.get("three") == 3);
}

BOOST_AUTO_TEST_CASE(hash_table_move_transfers_entries)
{
  Table table(4);
  table.add("one", 1);

  Table moved(std::move(table));

  BOOST_TEST(moved.get("one") == 1);
  BOOST_TEST(moved.size() == 1);
  BOOST_CHECK(table.empty());
}
