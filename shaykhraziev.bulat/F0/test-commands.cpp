#include <boost/test/unit_test.hpp>

#include "commands.hpp"

#include <sstream>

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

BOOST_AUTO_TEST_CASE(commands_registry_contains_initial_commands)
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
  BOOST_CHECK(commands.has("commands"));
  BOOST_CHECK(commands.has("ui"));
  BOOST_CHECK(!commands.has("missing"));
}

BOOST_AUTO_TEST_CASE(commands_make_drop_and_show_project)
{
  shaykhraziev::ProjectStorage storage;

  BOOST_TEST(run(storage, "make-project site 1 2") == "");
  BOOST_TEST(run(storage, "show-project site") == "<PROJECT: site, START: 1, WORKERS: 2, TASKS: 0>\n");
  BOOST_TEST(run(storage, "make-project site 1 2") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "drop-project site") == "");
  BOOST_TEST(run(storage, "show-project site") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_add_drop_and_show_task)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);

  BOOST_TEST(run(storage, "add-task site design 3 Design page") == "");
  BOOST_TEST(run(storage, "show-task site design") ==
      "<TASK: design, TITLE: Design page, DURATION: 3, DEPENDENCIES: 0>\n");
  BOOST_TEST(run(storage, "add-task site design 2 Duplicate") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "drop-task site design") == "");
  BOOST_TEST(run(storage, "show-task site design") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_reject_unknown_and_wrong_argument_count)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);

  BOOST_TEST(run(storage, "unknown") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "make-project site 1") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "add-task site task 1") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_help_prints_general_usage)
{
  shaykhraziev::ProjectStorage storage;
  const std::string output = run(storage, "help");

  BOOST_TEST(output.find("F0 Project Planner CLI") != std::string::npos);
  BOOST_TEST(output.find("Usage:") != std::string::npos);
  BOOST_TEST(output.find("lab <project-file>") != std::string::npos);
  BOOST_TEST(output.find("Use \"help <command>\" for command details.") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_list_prints_known_commands)
{
  shaykhraziev::ProjectStorage storage;
  const std::string output = run(storage, "commands");

  BOOST_TEST(output.find("Command") != std::string::npos);
  BOOST_TEST(output.find("make-project") != std::string::npos);
  BOOST_TEST(output.find("build-plan") != std::string::npos);
  BOOST_TEST(output.find("critical-path") != std::string::npos);
  BOOST_TEST(output.find("help") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_help_prints_command_details)
{
  shaykhraziev::ProjectStorage storage;
  const std::string output = run(storage, "help add-task");

  BOOST_TEST(output.find("Command:") != std::string::npos);
  BOOST_TEST(output.find("add-task") != std::string::npos);
  BOOST_TEST(output.find("add-task <project> <taskId> <duration> <title...>") != std::string::npos);
  BOOST_TEST(output.find("Example:") != std::string::npos);
  BOOST_TEST(run(storage, "help missing") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_verbose_mode_reports_unknown_command_with_suggestion)
{
  shaykhraziev::ProjectStorage storage;
  const std::string output = runSession(storage, "ui verbose\nbild-plan site\n");

  BOOST_TEST(output.find("<UI: verbose>") != std::string::npos);
  BOOST_TEST(output.find("Unknown command: bild-plan") != std::string::npos);
  BOOST_TEST(output.find("Did you mean: build-plan?") != std::string::npos);
  BOOST_TEST(output.find("Use \"commands\" to see available commands.") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_verbose_mode_reports_invalid_arguments)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  const std::string output = runSession(storage, "ui verbose\nadd-task site task 1\n");

  BOOST_TEST(output.find("Invalid arguments for command: add-task") != std::string::npos);
  BOOST_TEST(output.find("Usage:") != std::string::npos);
  BOOST_TEST(output.find("add-task <project> <taskId> <duration> <title...>") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_compact_mode_keeps_legacy_errors)
{
  shaykhraziev::ProjectStorage storage;
  const std::string output = runSession(storage, "ui verbose\nui compact\nbild-plan site\n");

  BOOST_TEST(output.find("<UI: verbose>") != std::string::npos);
  BOOST_TEST(output.find("<UI: compact>") != std::string::npos);
  BOOST_TEST(output.find("Unknown command:") == std::string::npos);
  BOOST_TEST(output.find("<INVALID COMMAND>") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_verbose_mode_reports_failed_command)
{
  shaykhraziev::ProjectStorage storage;
  const std::string output = runSession(storage, "ui verbose\nshow-project missing\n");

  BOOST_TEST(output.find("Command failed: show-project") != std::string::npos);
  BOOST_TEST(output.find("show-project <name>") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_verbose_show_project_is_readable)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);

  BOOST_TEST(run(storage, "show-project site") == "<PROJECT: site, START: 1, WORKERS: 2, TASKS: 0>\n");
  const std::string output = runSession(storage, "ui verbose\nshow-project site\n");

  BOOST_TEST(output.find("Project: site") != std::string::npos);
  BOOST_TEST(output.find("Start day: 1") != std::string::npos);
  BOOST_TEST(output.find("Workers: 2") != std::string::npos);
  BOOST_TEST(output.find("Plan: not built") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_verbose_show_task_and_stats_are_readable)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");
  storage.findProject("site")->addDependency("backend", "design");
  const std::string output = runSession(storage, "ui verbose\nshow-task site backend\nstats site\n");

  BOOST_TEST(output.find("Task: backend") != std::string::npos);
  BOOST_TEST(output.find("Dependencies:") != std::string::npos);
  BOOST_TEST(output.find("  - design") != std::string::npos);
  BOOST_TEST(output.find("Project statistics: site") != std::string::npos);
  BOOST_TEST(output.find("Dependencies: 1") != std::string::npos);
  BOOST_TEST(output.find("Plan status: not built") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_verbose_plan_views_are_readable)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");
  storage.findProject("site")->addDependency("backend", "design");
  const std::string output = runSession(storage,
      "ui verbose\n"
      "build-plan site\n"
      "show-worker site 1\n"
      "critical-path site\n"
      "show-gantt site\n");

  BOOST_TEST(output.find("Worker 1 tasks:") != std::string::npos);
  BOOST_TEST(output.find("Task       Start   End") != std::string::npos);
  BOOST_TEST(output.find("Critical path: site") != std::string::npos);
  BOOST_TEST(output.find("Duration: 7") != std::string::npos);
  BOOST_TEST(output.find("Path:") != std::string::npos);
  BOOST_TEST(output.find("Gantt chart: site") != std::string::npos);
  BOOST_TEST(output.find("<GANTT site>") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(commands_add_and_drop_dependencies)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");

  BOOST_TEST(run(storage, "add-dependency site backend design") == "");
  BOOST_TEST(run(storage, "show-task site backend") ==
      "<TASK: backend, TITLE: Backend, DURATION: 4, DEPENDENCIES: 1>\n"
      "design\n");
  BOOST_TEST(run(storage, "add-dependency site backend design") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "add-dependency site backend missing") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "add-dependency site backend backend") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "drop-dependency site backend design") == "");
  BOOST_TEST(run(storage, "drop-dependency site backend design") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_check_cycles)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("a", 1, "A");
  storage.findProject("site")->addTask("b", 1, "B");

  BOOST_TEST(run(storage, "check-cycles site") == "<NO CYCLES>\n");
  BOOST_TEST(run(storage, "add-dependency site b a") == "");
  BOOST_TEST(run(storage, "add-dependency site a b") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "check-cycles site") == "<NO CYCLES>\n");
  BOOST_TEST(run(storage, "check-cycles missing") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_build_plan)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");

  BOOST_TEST(run(storage, "build-plan site") == "<PLAN BUILT>\n");
  BOOST_REQUIRE(storage.findProject("site"));
  BOOST_CHECK(storage.findProject("site")->isPlanBuilt());
  BOOST_TEST(run(storage, "build-plan missing") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_show_worker_and_stats)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");

  BOOST_TEST(run(storage, "stats site") ==
      "<PROJECT: site, START: 1, END: NOT-BUILT, TASKS: 2, WORKERS: 2, TOTAL-DURATION: 7>\n");
  BOOST_TEST(run(storage, "show-worker site 1") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "build-plan site") == "<PLAN BUILT>\n");
  BOOST_TEST(run(storage, "show-worker site 1") ==
      "<WORKER 1>\n"
      "design: START 1, END 3\n");
  BOOST_TEST(run(storage, "show-worker site 3") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "stats site") ==
      "<PROJECT: site, START: 1, END: 4, TASKS: 2, WORKERS: 2, TOTAL-DURATION: 7>\n");
}

BOOST_AUTO_TEST_CASE(commands_show_gantt_requires_built_plan)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 1);
  storage.findProject("site")->addTask("task", 2, "Task");

  BOOST_TEST(run(storage, "show-gantt site") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "build-plan site") == "<PLAN BUILT>\n");
  BOOST_TEST(run(storage, "show-gantt site") ==
      "<GANTT site>\n"
      "DAYS:     1 2\n"
      "task  W1 # #\n"
      "<PROJECT-END: 2>\n");
}

BOOST_AUTO_TEST_CASE(commands_print_critical_path)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");
  storage.findProject("site")->addDependency("backend", "design");

  BOOST_TEST(run(storage, "critical-path site") ==
      "<CRITICAL-PATH site>\n"
      "design -> backend\n"
      "<DURATION: 7>\n");
  BOOST_TEST(run(storage, "critical-path missing") == "<INVALID COMMAND>\n");
}

BOOST_AUTO_TEST_CASE(commands_try_task_reports_possible_and_impossible)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 1);
  storage.findProject("site")->addTask("design", 3, "Design");

  BOOST_TEST(run(storage, "try-task site test 2 5 Test task") ==
      "<POSSIBLE>\n"
      "test: WORKER 1, START 4, END 5\n");
  BOOST_TEST(run(storage, "try-task site test 2 4 Test task") == "<IMPOSSIBLE>\n");
  BOOST_CHECK(!storage.findProject("site")->findTask("test"));
}

BOOST_AUTO_TEST_CASE(commands_try_task_rejects_invalid_input_and_keeps_plan)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 1);
  storage.findProject("site")->addTask("design", 3, "Design");
  shaykhraziev::buildProjectPlan(*storage.findProject("site"));
  const std::size_t oldEnd = storage.findProject("site")->getPlan().getProjectEndDay();

  BOOST_TEST(run(storage, "try-task site design 2 5 Duplicate") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "try-task site test 0 5 Bad") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "try-task site test 2 0 Bad") == "<INVALID COMMAND>\n");
  BOOST_TEST(run(storage, "try-task missing test 2 5 Bad") == "<INVALID COMMAND>\n");
  BOOST_CHECK(storage.findProject("site")->isPlanBuilt());
  BOOST_TEST(storage.findProject("site")->getPlan().getProjectEndDay() == oldEnd);
}

BOOST_AUTO_TEST_CASE(commands_mutations_reset_built_plan)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);
  storage.findProject("site")->addTask("design", 3, "Design");
  storage.findProject("site")->addTask("backend", 4, "Backend");

  BOOST_TEST(run(storage, "build-plan site") == "<PLAN BUILT>\n");
  BOOST_CHECK(storage.findProject("site")->isPlanBuilt());
  BOOST_TEST(run(storage, "add-dependency site backend design") == "");
  BOOST_CHECK(!storage.findProject("site")->isPlanBuilt());
  BOOST_TEST(run(storage, "show-gantt site") == "<INVALID COMMAND>\n");

  BOOST_TEST(run(storage, "build-plan site") == "<PLAN BUILT>\n");
  BOOST_TEST(run(storage, "drop-dependency site backend design") == "");
  BOOST_CHECK(!storage.findProject("site")->isPlanBuilt());

  BOOST_TEST(run(storage, "build-plan site") == "<PLAN BUILT>\n");
  BOOST_TEST(run(storage, "add-task site test 2 Test") == "");
  BOOST_CHECK(!storage.findProject("site")->isPlanBuilt());

  BOOST_TEST(run(storage, "build-plan site") == "<PLAN BUILT>\n");
  BOOST_TEST(run(storage, "drop-task site test") == "");
  BOOST_CHECK(!storage.findProject("site")->isPlanBuilt());
}

BOOST_AUTO_TEST_CASE(commands_empty_project_reports_stats_and_critical_path)
{
  shaykhraziev::ProjectStorage storage;
  storage.makeProject("site", 1, 2);

  BOOST_TEST(run(storage, "stats site") ==
      "<PROJECT: site, START: 1, END: NOT-BUILT, TASKS: 0, WORKERS: 2, TOTAL-DURATION: 0>\n");
  BOOST_TEST(run(storage, "critical-path site") ==
      "<CRITICAL-PATH site>\n"
      "<DURATION: 0>\n");
}
