#include <boost/test/unit_test.hpp>

#include <cuckoo-hash-table.hpp>
#include <hash-functions.hpp>

#include "commands.hpp"
#include "planner.hpp"
#include "project.hpp"

#include <string>
#include <vector>

namespace
{
  struct ConstantHash
  {
    std::size_t operator()(const std::string&) const
    {
      return 1;
    }
  };

  using Table =
      shaykhraziev::CuckooHashTable< std::string, int, shaykhraziev::HmacHash, shaykhraziev::StringEqual >;
  using CollisionTable =
      shaykhraziev::CuckooHashTable< std::string, int, ConstantHash, shaykhraziev::StringEqual >;
}

BOOST_AUTO_TEST_CASE(hash_table_contract_add_find_set_drop)
{
  Table table(2, 2);

  BOOST_TEST(table.empty());
  BOOST_TEST(table.capacity() == 8);
  BOOST_CHECK(table.add("alpha", 1));
  BOOST_CHECK(table.has("alpha"));
  BOOST_REQUIRE(table.find("alpha"));
  BOOST_TEST(*table.find("alpha") == 1);
  BOOST_CHECK(!table.set("alpha", 2));
  BOOST_TEST(table.get("alpha") == 2);
  BOOST_CHECK(table.set("beta", 3));
  BOOST_TEST(table.size() == 2);
  BOOST_CHECK(table.drop("alpha"));
  BOOST_CHECK(!table.has("alpha"));
  BOOST_CHECK(!table.drop("missing"));
  BOOST_CHECK(table.add("alpha", 4));
  BOOST_TEST(table.get("alpha") == 4);
}

BOOST_AUTO_TEST_CASE(hash_table_contract_rehash_copy_move)
{
  Table table(2, 2);
  table.add("alpha", 1);
  table.add("beta", 2);
  table.rehash(8);

  BOOST_TEST(table.capacity() == 32);
  BOOST_TEST(table.get("alpha") == 1);
  BOOST_TEST(table.get("beta") == 2);

  Table copied(table);
  BOOST_TEST(copied.get("alpha") == 1);
  copied.set("alpha", 10);
  BOOST_TEST(table.get("alpha") == 1);
  BOOST_TEST(copied.get("alpha") == 10);

  Table assigned(1, 2);
  assigned = table;
  BOOST_TEST(assigned.get("beta") == 2);

  Table moved(std::move(assigned));
  BOOST_TEST(moved.get("alpha") == 1);
  BOOST_TEST(moved.get("beta") == 2);

  Table moveAssigned(1, 2);
  moveAssigned = std::move(moved);
  BOOST_TEST(moveAssigned.get("alpha") == 1);
  BOOST_TEST(moveAssigned.get("beta") == 2);
}

BOOST_AUTO_TEST_CASE(hash_table_contract_iterators_and_collisions)
{
  CollisionTable table(2, 4);
  table.add("alpha", 1);
  table.add("beta", 2);
  table.add("gamma", 3);
  table.add("delta", 4);

  std::vector< std::string > keys;
  int total = 0;
  for (CollisionTable::iterator it = table.begin(); it != table.end(); ++it)
  {
    keys.push_back(it->key);
    total += it->value;
  }

  BOOST_TEST(keys.size() == 4);
  BOOST_TEST(total == 10);

  const CollisionTable& constTable = table;
  std::size_t count = 0;
  for (CollisionTable::const_iterator it = constTable.cbegin(); it != constTable.cend(); ++it)
  {
    BOOST_CHECK(constTable.has(it->key));
    ++count;
  }
  BOOST_TEST(count == table.size());
}

BOOST_AUTO_TEST_CASE(hash_table_contract_f0_tables_keep_working)
{
  shaykhraziev::ProjectStorage storage;
  BOOST_CHECK(storage.makeProject("site", 1, 2));
  shaykhraziev::Project* project = storage.findProject("site");
  BOOST_REQUIRE(project);
  BOOST_CHECK(project->addTask("design", 3, "Design"));
  BOOST_CHECK(project->addTask("backend", 4, "Backend"));
  BOOST_CHECK(project->addDependency("backend", "design"));
  BOOST_CHECK(shaykhraziev::buildProjectPlan(*project));
  BOOST_REQUIRE(project->getPlan().findTask("backend"));

  shaykhraziev::CommandRegistry commands = shaykhraziev::makeCommandRegistry();
  BOOST_CHECK(commands.has("build-plan"));
  BOOST_CHECK(commands.has("critical-path"));
}
