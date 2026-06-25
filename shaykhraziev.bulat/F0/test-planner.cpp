#include <boost/test/unit_test.hpp>

#include "planner.hpp"
#include "project.hpp"

BOOST_AUTO_TEST_CASE(planner_builds_single_task_plan)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");

  BOOST_CHECK(shaykhraziev::buildProjectPlan(project));

  const shaykhraziev::PlannedTask* task = project.getPlan().findTask("design");
  BOOST_REQUIRE(task);
  BOOST_TEST(task->workerId == 1);
  BOOST_TEST(task->startDay == 1);
  BOOST_TEST(task->endDay == 3);
  BOOST_TEST(project.getPlan().getProjectEndDay() == 3);
}

BOOST_AUTO_TEST_CASE(planner_runs_independent_tasks_on_two_workers)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("backend", 4, "Backend");

  BOOST_CHECK(shaykhraziev::buildProjectPlan(project));

  BOOST_TEST(project.getPlan().findTask("design")->workerId == 1);
  BOOST_TEST(project.getPlan().findTask("backend")->workerId == 2);
  BOOST_TEST(project.getPlan().findTask("backend")->startDay == 1);
}

BOOST_AUTO_TEST_CASE(planner_respects_dependency_chain)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("backend", 4, "Backend");
  project.addTask("test", 2, "Test");
  project.addDependency("backend", "design");
  project.addDependency("test", "backend");

  BOOST_CHECK(shaykhraziev::buildProjectPlan(project));

  BOOST_TEST(project.getPlan().findTask("backend")->startDay == 4);
  BOOST_TEST(project.getPlan().findTask("test")->startDay == 8);
  BOOST_TEST(project.getPlan().getProjectEndDay() == 9);
}

BOOST_AUTO_TEST_CASE(planner_runs_parallel_tasks_after_dependency)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("layout", 2, "Layout");
  project.addTask("backend", 4, "Backend");
  project.addDependency("layout", "design");
  project.addDependency("backend", "design");

  BOOST_CHECK(shaykhraziev::buildProjectPlan(project));

  BOOST_TEST(project.getPlan().findTask("layout")->startDay == 4);
  BOOST_TEST(project.getPlan().findTask("backend")->startDay == 4);
  BOOST_TEST(project.getPlan().findTask("layout")->workerId == 1);
  BOOST_TEST(project.getPlan().findTask("backend")->workerId == 2);
}

BOOST_AUTO_TEST_CASE(planner_respects_single_worker_limit)
{
  shaykhraziev::Project project("site", 1, 1);
  project.addTask("design", 3, "Design");
  project.addTask("backend", 4, "Backend");

  BOOST_CHECK(shaykhraziev::buildProjectPlan(project));

  BOOST_TEST(project.getPlan().findTask("design")->startDay == 1);
  BOOST_TEST(project.getPlan().findTask("backend")->startDay == 4);
  BOOST_TEST(project.getPlan().getProjectEndDay() == 7);
}

BOOST_AUTO_TEST_CASE(planner_calculates_critical_path_for_chain)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("design", 3, "Design");
  project.addTask("backend", 4, "Backend");
  project.addTask("test", 2, "Test");
  project.addDependency("backend", "design");
  project.addDependency("test", "backend");
  shaykhraziev::CriticalPath path;

  BOOST_CHECK(shaykhraziev::calculateCriticalPath(project, path));

  BOOST_TEST(path.duration == 9);
  BOOST_TEST(path.taskIds.front() == "design");
  BOOST_TEST(path.taskIds.back() == "test");
}

BOOST_AUTO_TEST_CASE(planner_calculates_critical_path_for_different_chains)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("a", 2, "A");
  project.addTask("b", 2, "B");
  project.addTask("c", 7, "C");
  project.addTask("d", 1, "D");
  project.addDependency("b", "a");
  project.addDependency("d", "c");
  shaykhraziev::CriticalPath path;

  BOOST_CHECK(shaykhraziev::calculateCriticalPath(project, path));

  BOOST_TEST(path.duration == 8);
  BOOST_TEST(path.taskIds.front() == "c");
  BOOST_TEST(path.taskIds.back() == "d");
}

BOOST_AUTO_TEST_CASE(planner_critical_path_keeps_stable_tie)
{
  shaykhraziev::Project project("site", 1, 2);
  project.addTask("a", 3, "A");
  project.addTask("b", 3, "B");
  shaykhraziev::CriticalPath path;

  BOOST_CHECK(shaykhraziev::calculateCriticalPath(project, path));

  BOOST_TEST(path.duration == 3);
  BOOST_TEST(path.taskIds.front() == "a");
}

BOOST_AUTO_TEST_CASE(planner_critical_path_supports_empty_project)
{
  shaykhraziev::Project project("site", 1, 2);
  shaykhraziev::CriticalPath path;

  BOOST_CHECK(shaykhraziev::calculateCriticalPath(project, path));

  BOOST_CHECK(path.taskIds.empty());
  BOOST_TEST(path.duration == 0);
}
