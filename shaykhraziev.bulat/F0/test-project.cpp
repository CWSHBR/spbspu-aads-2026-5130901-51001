#include <boost/test/unit_test.hpp>

#include "project.hpp"

#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE(project_storage_creates_and_finds_projects)
{
  shaykhraziev::ProjectStorage storage;

  BOOST_CHECK(storage.makeProject("site", 1, 2));
  BOOST_TEST(storage.countProjects() == 1);

  shaykhraziev::Project* project = storage.findProject("site");
  BOOST_REQUIRE(project);
  BOOST_TEST(project->getName() == "site");
  BOOST_TEST(project->getStartDay() == 1);
  BOOST_TEST(project->getWorkersCount() == 2);
}

BOOST_AUTO_TEST_CASE(project_storage_rejects_invalid_projects)
{
  shaykhraziev::ProjectStorage storage;

  BOOST_CHECK(!storage.makeProject("1site", 1, 2));
  BOOST_CHECK(!storage.makeProject("site", 0, 2));
  BOOST_CHECK(!storage.makeProject("site", 1, 0));
  BOOST_CHECK(storage.makeProject("site", 1, 1));
  BOOST_CHECK(!storage.makeProject("site", 2, 2));
}

BOOST_AUTO_TEST_CASE(project_storage_drops_projects)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);

  BOOST_CHECK(storage.dropProject("site"));
  BOOST_CHECK(!storage.findProject("site"));
  BOOST_TEST(storage.countProjects() == 0);
  BOOST_CHECK(!storage.dropProject("site"));
}

BOOST_AUTO_TEST_CASE(project_adds_tasks_in_stable_order)
{
  shaykhraziev::Project project("site", 1, 2);

  BOOST_CHECK(project.addTask("design", 3, "Design"));
  BOOST_CHECK(project.addTask("backend", 4, "Backend"));
  BOOST_TEST(project.countTasks() == 2);
  BOOST_TEST(project.getTotalDuration() == 7);

  const shaykhraziev::Task* task = project.findTask("design");
  BOOST_REQUIRE(task);
  BOOST_TEST(task->id == "design");
  BOOST_TEST(task->duration == 3);
  BOOST_TEST(task->title == "Design");

  std::vector< std::string > order;
  for (auto it = project.getTaskOrder().cbegin(); it != project.getTaskOrder().cend(); ++it)
  {
    order.push_back(*it);
  }
  std::vector< std::string > expected{"design", "backend"};
  BOOST_TEST(order == expected, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(project_rejects_invalid_tasks)
{
  shaykhraziev::Project project("site", 1, 2);

  BOOST_CHECK(!project.addTask("1task", 1, "Bad"));
  BOOST_CHECK(!project.addTask("task", 0, "Bad"));
  BOOST_CHECK(!project.addTask("task", 1, ""));
  BOOST_CHECK(project.addTask("task", 1, "Task"));
  BOOST_CHECK(!project.addTask("task", 2, "Duplicate"));
}

BOOST_AUTO_TEST_CASE(project_drops_tasks_and_keeps_order)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("layout", 2, "Layout");
  project.addTask("backend", 4, "Backend");

  BOOST_CHECK(project.dropTask("layout"));
  BOOST_CHECK(!project.findTask("layout"));
  BOOST_TEST(project.countTasks() == 2);

  std::vector< std::string > order;
  for (auto it = project.getTaskOrder().cbegin(); it != project.getTaskOrder().cend(); ++it)
  {
    order.push_back(*it);
  }
  std::vector< std::string > expected{"design", "backend"};
  BOOST_TEST(order == expected, boost::test_tools::per_element());
  BOOST_CHECK(!project.dropTask("missing"));
}

BOOST_AUTO_TEST_CASE(project_adds_and_drops_dependencies)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("backend", 4, "Backend");

  BOOST_CHECK(project.addDependency("backend", "design"));
  BOOST_CHECK(project.hasDependency("backend", "design"));
  BOOST_CHECK(!project.addDependency("backend", "design"));
  BOOST_CHECK(!project.addDependency("backend", "backend"));
  BOOST_CHECK(!project.addDependency("backend", "missing"));

  const shaykhraziev::Task* backend = project.findTask("backend");
  BOOST_REQUIRE(backend);
  BOOST_TEST(backend->dependencies.size() == 1);

  BOOST_CHECK(project.dropDependency("backend", "design"));
  BOOST_CHECK(!project.hasDependency("backend", "design"));
  BOOST_CHECK(!project.dropDependency("backend", "design"));
}

BOOST_AUTO_TEST_CASE(project_drop_task_removes_dependencies)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("backend", 4, "Backend");
  project.addDependency("backend", "design");

  BOOST_CHECK(project.dropTask("design"));
  BOOST_CHECK(!project.hasDependency("backend", "design"));
}

BOOST_AUTO_TEST_CASE(project_builds_topological_order)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("backend", 4, "Backend");
  project.addTask("test", 2, "Test");
  project.addDependency("backend", "design");
  project.addDependency("test", "backend");

  shaykhraziev::List< std::string > order;

  BOOST_CHECK(project.getTopologicalOrder(order));
  BOOST_TEST(order.size() == 3);
  BOOST_TEST(order.front() == "design");
  BOOST_TEST(order.back() == "test");
}

BOOST_AUTO_TEST_CASE(project_rejects_simple_and_complex_cycles)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("a", 1, "A");
  project.addTask("b", 1, "B");
  project.addTask("c", 1, "C");

  BOOST_CHECK(project.addDependency("b", "a"));
  BOOST_CHECK(project.addDependency("c", "b"));
  BOOST_CHECK(!project.addDependency("a", "c"));
  BOOST_CHECK(!project.hasCycle());
  BOOST_CHECK(!project.hasDependency("a", "c"));
}
