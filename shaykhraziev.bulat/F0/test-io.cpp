#include <boost/test/unit_test.hpp>

#include "io.hpp"

#include <sstream>

BOOST_AUTO_TEST_CASE(io_reads_projects_and_tasks)
{
  std::istringstream input(
      "project site 1 2\n"
      "task site design 3 Design page\n"
      "task site backend 4 Backend\n");

  shaykhraziev::ProjectStorage storage = shaykhraziev::readProjects(input);

  const shaykhraziev::Project* project = storage.findProject("site");
  BOOST_REQUIRE(project);
  BOOST_TEST(project->getStartDay() == 1);
  BOOST_TEST(project->getWorkersCount() == 2);
  BOOST_TEST(project->countTasks() == 2);
  BOOST_REQUIRE(project->findTask("design"));
  BOOST_TEST(project->findTask("design")->title == "Design page");
}

BOOST_AUTO_TEST_CASE(io_reads_dependency_lines)
{
  std::istringstream input(
      "project site 1 2\n"
      "task site design 3 Design\n"
      "task site backend 4 Backend\n"
      "dependency site backend design\n");

  shaykhraziev::ProjectStorage storage = shaykhraziev::readProjects(input);

  const shaykhraziev::Project* project = storage.findProject("site");
  BOOST_REQUIRE(project);
  BOOST_TEST(project->countTasks() == 2);
  BOOST_CHECK(project->hasDependency("backend", "design"));
}

BOOST_AUTO_TEST_CASE(io_rejects_invalid_project_file)
{
  std::istringstream input("task site design 3 Design\n");

  BOOST_CHECK_THROW(shaykhraziev::readProjects(input), std::logic_error);
}

BOOST_AUTO_TEST_CASE(io_rejects_missing_file)
{
  BOOST_CHECK_THROW(shaykhraziev::readProjectsFromFile("__missing_f0_file__"), std::runtime_error);
}
