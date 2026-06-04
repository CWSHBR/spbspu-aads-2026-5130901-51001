#include <boost/test/unit_test.hpp>

#include "commands.hpp"

#include <sstream>
#include <string>

namespace
{
  std::string run(shaykhraziev::ProjectStorage& storage, const std::string& line)
  {
    shaykhraziev::CommandRegistry commands = shaykhraziev::makeCommandRegistry();
    std::ostringstream out;
    shaykhraziev::executeCommandLine(storage, commands, line, out);
    return out.str();
  }

  std::string runSession(shaykhraziev::ProjectStorage& storage, const std::string& lines)
  {
    shaykhraziev::CommandRegistry commands = shaykhraziev::makeCommandRegistry();
    std::istringstream in(lines);
    std::ostringstream out;
    shaykhraziev::processCommands(storage, commands, in, out);
    return out.str();
  }
}

BOOST_AUTO_TEST_CASE(commands_registry_contains_supported_commands)
{
  shaykhraziev::CommandRegistry commands = shaykhraziev::makeCommandRegistry();

  BOOST_CHECK(commands.has("make-project"));
  BOOST_CHECK(commands.has("drop-project"));
  BOOST_CHECK(commands.has("show-project"));
  BOOST_CHECK(commands.has("add-task"));
  BOOST_CHECK(commands.has("drop-task"));
  BOOST_CHECK(commands.has("show-task"));
  BOOST_CHECK(commands.has("add-dependency"));
  BOOST_CHECK(commands.has("drop-dependency"));
  BOOST_CHECK(commands.has("check-cycles"));
  BOOST_CHECK(commands.has("build-plan"));
  BOOST_CHECK(commands.has("show-worker"));
  BOOST_CHECK(commands.has("stats"));
  BOOST_CHECK(commands.has("show-gantt"));
  BOOST_CHECK(commands.has("critical-path"));
  BOOST_CHECK(commands.has("try-task"));
  BOOST_CHECK(commands.has("help"));
  BOOST_CHECK(!commands.has("commands"));
  BOOST_CHECK(!commands.has("ui"));
}

BOOST_AUTO_TEST_CASE(commands_help_and_errors_are_readable)
{
  shaykhraziev::ProjectStorage storage;

  BOOST_TEST(run(storage, "help").find("F0 Project Planner CLI") != std::string::npos);
  BOOST_TEST(run(storage, "help add-task").find("add-task <project> <taskId> <duration> <title...>") !=
      std::string::npos);
  BOOST_TEST(run(storage, "bild-plan site").find("Did you mean: build-plan?") != std::string::npos);
  BOOST_TEST(run(storage, "make-project site 1").find("Invalid arguments for command: make-project") !=
      std::string::npos);
  BOOST_TEST(run(storage, "commands").find("Unknown command: commands") != std::string::npos);
  BOOST_TEST(run(storage, "ui verbose").find("Unknown command: ui") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_project_and_task_flow_is_readable)
{
  shaykhraziev::ProjectStorage storage;

  BOOST_TEST(run(storage, "make-project site 1 2") == "Project created: site\n");
  BOOST_TEST(run(storage, "show-project site").find("Project: site\n") != std::string::npos);
  BOOST_TEST(run(storage, "show-project site").find("Plan: not built") != std::string::npos);
  BOOST_TEST(run(storage, "add-task site design 3 Design page") == "Task added: design\n");
  BOOST_TEST(run(storage, "show-task site design").find("Title: Design page") != std::string::npos);
  BOOST_TEST(run(storage, "drop-task site design") == "Task deleted: design\n");
  BOOST_TEST(run(storage, "drop-project site") == "Project deleted: site\n");
}

BOOST_AUTO_TEST_CASE(commands_dependencies_cycles_plan_and_views)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");

  BOOST_TEST(run(storage, "add-dependency site backend design") ==
      "Dependency added: backend depends on design\n");
  BOOST_TEST(run(storage, "show-task site backend").find("  - design") != std::string::npos);
  BOOST_TEST(run(storage, "check-cycles site") == "Cycle check: no cycles\n");
  BOOST_TEST(run(storage, "build-plan site") == "Plan built for project: site\n");
  BOOST_TEST(run(storage, "show-worker site 1").find("Worker 1 tasks:") != std::string::npos);
  BOOST_TEST(run(storage, "stats site").find("Plan status: built") != std::string::npos);
  BOOST_TEST(run(storage, "show-gantt site").find("Gantt chart: site") != std::string::npos);
  BOOST_TEST(run(storage, "critical-path site").find("Duration: 7") != std::string::npos);
  BOOST_TEST(run(storage, "drop-dependency site backend design") ==
      "Dependency removed: backend no longer depends on design\n");
}

BOOST_AUTO_TEST_CASE(commands_try_task_reports_possible_and_impossible)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 1);
  storage.findProject("site")->addTask("design", 3, "Design");

  BOOST_TEST(run(storage, "try-task site test 2 5 Test task").find("Task can be scheduled before deadline.") !=
      std::string::npos);
  BOOST_TEST(run(storage, "try-task site test 2 4 Test task") == "Task cannot be scheduled before deadline.\n");
  BOOST_CHECK(!storage.findProject("site")->findTask("test"));
}

BOOST_AUTO_TEST_CASE(commands_mutations_reset_built_plan)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");

  BOOST_TEST(run(storage, "build-plan site") == "Plan built for project: site\n");
  BOOST_CHECK(storage.findProject("site")->isPlanBuilt());
  BOOST_TEST(run(storage, "add-dependency site backend design") ==
      "Dependency added: backend depends on design\n");
  BOOST_CHECK(!storage.findProject("site")->isPlanBuilt());
  BOOST_TEST(run(storage, "build-plan site") == "Plan built for project: site\n");
  BOOST_TEST(run(storage, "drop-dependency site backend design") ==
      "Dependency removed: backend no longer depends on design\n");
  BOOST_CHECK(!storage.findProject("site")->isPlanBuilt());
}

BOOST_AUTO_TEST_CASE(commands_process_prints_prompt)
{
  shaykhraziev::ProjectStorage storage;
  const std::string output = runSession(storage, "make-project site 1 2\nshow-project site\n");

  BOOST_TEST(output.find(" > Project created: site\n > Project: site") != std::string::npos);
}
