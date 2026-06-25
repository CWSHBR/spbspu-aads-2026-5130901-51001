#include "commands.hpp"

#include <algorithm>
#include <iomanip>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "../common/string-utils.hpp"

#include "gantt.hpp"
#include "io.hpp"
#include "planner.hpp"

namespace
{
  const std::size_t UNLIMITED_ARGUMENTS = 0;

  struct CommandDoc
  {
    const char* name;
    const char* usage;
    const char* description;
    const char* example;
  };

  const CommandDoc COMMAND_DOCS[] =
  {
    {"make-project", "make-project <name> <startDay> <workersCount>",
        "Create a new project.", "make-project site 1 2"},
    {"drop-project", "drop-project <name>",
        "Delete an existing project.", "drop-project site"},
    {"show-project", "show-project <name>",
        "Show project summary.", "show-project site"},
    {"add-task", "add-task <project> <taskId> <duration> <title...>",
        "Add a task to an existing project.", "add-task site backend 4 Backend implementation"},
    {"drop-task", "drop-task <project> <taskId>",
        "Delete a task and remove dependencies that reference it.", "drop-task site backend"},
    {"show-task", "show-task <project> <taskId>",
        "Show task details and dependencies.", "show-task site backend"},
    {"add-dependency", "add-dependency <project> <taskId> <dependencyId>",
        "Make task depend on another task.", "add-dependency site backend design"},
    {"drop-dependency", "drop-dependency <project> <taskId> <dependencyId>",
        "Remove a dependency between two tasks.", "drop-dependency site backend design"},
    {"check-cycles", "check-cycles <project>",
        "Check whether project dependencies contain a cycle.", "check-cycles site"},
    {"build-plan", "build-plan <project>",
        "Build a calendar plan for project tasks.", "build-plan site"},
    {"show-worker", "show-worker <project> <workerId>",
        "Show tasks assigned to a worker after plan building.", "show-worker site 1"},
    {"stats", "stats <project>",
        "Show project statistics.", "stats site"},
    {"show-gantt", "show-gantt <project>",
        "Show ASCII Gantt chart after plan building.", "show-gantt site"},
    {"critical-path", "critical-path <project>",
        "Calculate the longest dependency path.", "critical-path site"},
    {"try-task", "try-task <project> <taskId> <duration> <deadline> <title...>",
        "Check whether a new independent task can finish before a deadline.",
        "try-task site qa 2 9 QA task"},
    {"help", "help [command]",
        "Show general help or details for one command.", "help add-task"}
  };

  const std::size_t COMMAND_DOCS_COUNT = sizeof(COMMAND_DOCS) / sizeof(COMMAND_DOCS[0]);

  std::string tokenAt(const shaykhraziev::List< std::string >& tokens, std::size_t index)
  {
    std::size_t current = 0;
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
      if (current == index)
      {
        return *it;
      }
      ++current;
    }
    return "";
  }

  std::string getTailAfterTokens(const std::string& line, std::size_t tokenCount)
  {
    std::size_t index = 0;
    for (std::size_t token = 0; token < tokenCount; ++token)
    {
      while (index < line.size() && shaykhraziev::isSpace(line[index]))
      {
        ++index;
      }
      while (index < line.size() && !shaykhraziev::isSpace(line[index]))
      {
        ++index;
      }
    }
    while (index < line.size() && shaykhraziev::isSpace(line[index]))
    {
      ++index;
    }
    return line.substr(index);
  }

  bool checkArgumentCount(const shaykhraziev::CommandHandler& handler, std::size_t tokenCount)
  {
    const std::size_t argumentCount = tokenCount - 1;
    return argumentCount >= handler.minArguments &&
        (handler.maxArguments == UNLIMITED_ARGUMENTS || argumentCount <= handler.maxArguments);
  }

  const CommandDoc* findCommandDoc(const std::string& name)
  {
    for (std::size_t i = 0; i < COMMAND_DOCS_COUNT; ++i)
    {
      if (name == COMMAND_DOCS[i].name)
      {
        return &COMMAND_DOCS[i];
      }
    }
    return nullptr;
  }

  void printGeneralHelp(std::ostream& out)
  {
    out << "F0 Project Planner CLI\n\n";
    out << "Usage:\n";
    out << "  lab <project-file>\n\n";
    out << "Input file format:\n";
    out << "  project <projectName> <startDay> <workersCount>\n";
    out << "  task <projectName> <taskId> <duration> <title...>\n";
    out << "  dependency <projectName> <taskId> <dependencyId>\n\n";
    out << "Main commands:\n";
    out << "  make-project <name> <startDay> <workersCount>\n";
    out << "  add-task <project> <taskId> <duration> <title...>\n";
    out << "  add-dependency <project> <taskId> <dependencyId>\n";
    out << "  build-plan <project>\n";
    out << "  show-gantt <project>\n";
    out << "  critical-path <project>\n";
    out << "  stats <project>\n\n";
    out << "Use \"help <command>\" for command details.\n";
  }

  void printCommandDetails(const CommandDoc& doc, std::ostream& out)
  {
    out << "Command:\n";
    out << "  " << doc.name << "\n\n";
    out << "Usage:\n";
    out << "  " << doc.usage << "\n\n";
    out << "Description:\n";
    out << "  " << doc.description << "\n\n";
    out << "Example:\n";
    out << "  " << doc.example << '\n';
  }

  void printCommandUsage(const shaykhraziev::CommandHandler& handler, std::ostream& out)
  {
    out << "Usage:\n";
    out << "  " << handler.usage << "\n\n";
    out << "Example:\n";
    out << "  " << handler.example << '\n';
  }

  std::size_t editDistance(const std::string& lhs, const std::string& rhs)
  {
    std::vector< std::size_t > previous(rhs.size() + 1);
    std::vector< std::size_t > current(rhs.size() + 1);
    for (std::size_t j = 0; j <= rhs.size(); ++j)
    {
      previous[j] = j;
    }
    for (std::size_t i = 1; i <= lhs.size(); ++i)
    {
      current[0] = i;
      for (std::size_t j = 1; j <= rhs.size(); ++j)
      {
        const std::size_t replaceCost = lhs[i - 1] == rhs[j - 1] ? 0 : 1;
        current[j] = std::min(
            std::min(previous[j] + 1, current[j - 1] + 1),
            previous[j - 1] + replaceCost);
      }
      previous.swap(current);
    }
    return previous[rhs.size()];
  }

  const CommandDoc* findClosestCommandDoc(const std::string& name)
  {
    const CommandDoc* best = nullptr;
    std::size_t bestDistance = 3;
    for (std::size_t i = 0; i < COMMAND_DOCS_COUNT; ++i)
    {
      const std::size_t distance = editDistance(name, COMMAND_DOCS[i].name);
      if (distance < bestDistance)
      {
        bestDistance = distance;
        best = &COMMAND_DOCS[i];
      }
    }
    return best;
  }

  void printUnknownCommand(const std::string& commandName, std::ostream& out)
  {
    out << "Unknown command: " << commandName << '\n';
    const CommandDoc* suggestion = findClosestCommandDoc(commandName);
    if (suggestion)
    {
      out << "Did you mean: " << suggestion->name << "?\n";
    }
    out << "Use \"help\" to see available commands.\n";
  }

  void printInvalidArguments(
      const std::string& commandName,
      const shaykhraziev::CommandHandler& handler,
      std::ostream& out)
  {
    out << "Invalid arguments for command: " << commandName << "\n\n";
    printCommandUsage(handler, out);
  }

  void printCommandFailure(
      const std::string& commandName,
      const shaykhraziev::CommandHandler& handler,
      std::ostream& out)
  {
    out << "Command failed: " << commandName << "\n\n";
    printCommandUsage(handler, out);
  }

  bool helpCommand(
      shaykhraziev::ProjectStorage&,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    if (tokens.size() == 1)
    {
      printGeneralHelp(out);
      return true;
    }

    const CommandDoc* doc = findCommandDoc(tokenAt(tokens, 1));
    if (!doc)
    {
      return false;
    }
    printCommandDetails(*doc, out);
    return true;
  }

  void printIndentedList(const shaykhraziev::List< std::string >& values, std::ostream& out)
  {
    if (values.empty())
    {
      out << "  <none>\n";
      return;
    }
    for (shaykhraziev::List< std::string >::const_iterator it = values.cbegin(); it != values.cend(); ++it)
    {
      out << "  - " << *it << '\n';
    }
  }

  std::size_t countDependencies(const shaykhraziev::Project& project)
  {
    std::size_t count = 0;
    for (shaykhraziev::List< std::string >::const_iterator it = project.getTaskOrder().cbegin();
        it != project.getTaskOrder().cend();
        ++it)
    {
      const shaykhraziev::Task* task = project.findTask(*it);
      if (task)
      {
        count += task->dependencies.size();
      }
    }
    return count;
  }

  void printReadableProject(const shaykhraziev::Project& project, std::ostream& out)
  {
    out << "Project: " << project.getName() << '\n';
    out << "Start day: " << project.getStartDay() << '\n';
    out << "Workers: " << project.getWorkersCount() << '\n';
    out << "Tasks: " << project.countTasks() << '\n';
    out << "Plan: " << (project.isPlanBuilt() ? "built" : "not built") << '\n';
  }

  bool makeProjectCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    std::size_t startDay = 0;
    std::size_t workersCount = 0;
    const bool ok = shaykhraziev::parsePositiveSize(tokenAt(tokens, 2), startDay) &&
        shaykhraziev::parsePositiveSize(tokenAt(tokens, 3), workersCount) &&
        storage.makeProject(tokenAt(tokens, 1), startDay, workersCount);
    if (ok)
    {
      out << "Project created: " << tokenAt(tokens, 1) << '\n';
    }
    return ok;
  }

  bool dropProjectCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    const bool ok = storage.dropProject(tokenAt(tokens, 1));
    if (ok)
    {
      out << "Project deleted: " << tokenAt(tokens, 1) << '\n';
    }
    return ok;
  }

  bool showProjectCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    const shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project)
    {
      return false;
    }
    printReadableProject(*project, out);
    return true;
  }

  bool addTaskCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string& line,
      std::ostream& out)
  {
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    std::size_t duration = 0;
    if (!project || !shaykhraziev::parsePositiveSize(tokenAt(tokens, 3), duration))
    {
      return false;
    }
    const std::string title = getTailAfterTokens(line, 4);
    const bool ok = project->addTask(tokenAt(tokens, 2), duration, title);
    if (ok)
    {
      out << "Task added: " << tokenAt(tokens, 2) << '\n';
    }
    return ok;
  }

  bool dropTaskCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    const bool ok = project && project->dropTask(tokenAt(tokens, 2));
    if (ok)
    {
      out << "Task deleted: " << tokenAt(tokens, 2) << '\n';
    }
    return ok;
  }

  bool showTaskCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    const shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project)
    {
      return false;
    }
    const shaykhraziev::Task* task = project->findTask(tokenAt(tokens, 2));
    if (!task)
    {
      return false;
    }
    out << "Task: " << task->id << '\n';
    out << "Title: " << task->title << '\n';
    out << "Duration: " << task->duration << '\n';
    out << "Dependencies:\n";
    printIndentedList(task->dependencies, out);
    out << "Dependents:\n";
    printIndentedList(task->dependents, out);
    return true;
  }

  bool addDependencyCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    const bool ok = project && project->addDependency(tokenAt(tokens, 2), tokenAt(tokens, 3));
    if (ok)
    {
      out << "Dependency added: " << tokenAt(tokens, 2) << " depends on " << tokenAt(tokens, 3) << '\n';
    }
    return ok;
  }

  bool dropDependencyCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    const bool ok = project && project->dropDependency(tokenAt(tokens, 2), tokenAt(tokens, 3));
    if (ok)
    {
      out << "Dependency removed: " << tokenAt(tokens, 2) << " no longer depends on " << tokenAt(tokens, 3) << '\n';
    }
    return ok;
  }

  bool checkCyclesCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    const shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project)
    {
      return false;
    }
    out << "Cycle check: " << (project->hasCycle() ? "cycle found" : "no cycles") << '\n';
    return true;
  }

  bool buildPlanCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project || !shaykhraziev::buildProjectPlan(*project))
    {
      return false;
    }
    out << "Plan built for project: " << project->getName() << '\n';
    return true;
  }

  bool showWorkerCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    std::size_t workerId = 0;
    if (!project || !project->isPlanBuilt() ||
        !shaykhraziev::parsePositiveSize(tokenAt(tokens, 2), workerId) ||
        workerId > project->getWorkersCount())
    {
      return false;
    }

    out << "Worker " << workerId << " tasks:\n\n";
    out << "Task       Start   End\n";
    out << "--------   -----   ---\n";
    const shaykhraziev::Plan& plan = project->getPlan();
    for (shaykhraziev::List< std::string >::const_iterator it = project->getTaskOrder().cbegin();
        it != project->getTaskOrder().cend();
        ++it)
    {
      const shaykhraziev::PlannedTask* planned = plan.findTask(*it);
      if (planned && planned->workerId == workerId)
      {
        out << std::left << std::setw(10) << planned->taskId << ' ' <<
            std::right << std::setw(5) << planned->startDay << "   " <<
            std::setw(3) << planned->endDay << '\n';
      }
    }
    return true;
  }

  bool statsCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    const shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project)
    {
      return false;
    }
    out << "Project statistics: " << project->getName() << "\n\n";
    out << "Tasks: " << project->countTasks() << '\n';
    out << "Dependencies: " << countDependencies(*project) << '\n';
    out << "Total duration: " << project->getTotalDuration() << '\n';
    out << "Workers: " << project->getWorkersCount() << '\n';
    out << "Plan status: " << (project->isPlanBuilt() ? "built" : "not built") << '\n';
    if (project->isPlanBuilt())
    {
      out << "Project end day: " << project->getPlan().getProjectEndDay() << '\n';
    }
    else
    {
      out << "Project end day: not built\n";
    }
    return true;
  }

  bool showGanttCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    const shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project)
    {
      return false;
    }
    if (!project->isPlanBuilt())
    {
      out << "Gantt chart is not available because the plan is not built.\n";
      out << "Run: build-plan " << project->getName() << '\n';
      return true;
    }
    out << "Gantt chart: " << project->getName() << "\n\n";
    shaykhraziev::renderGantt(*project, out);
    return true;
  }

  void printCriticalPath(const shaykhraziev::CriticalPath& path, std::ostream& out)
  {
    bool first = true;
    for (shaykhraziev::List< std::string >::const_iterator it = path.taskIds.cbegin();
        it != path.taskIds.cend();
        ++it)
    {
      if (!first)
      {
        out << " -> ";
      }
      out << *it;
      first = false;
    }
    if (!path.taskIds.empty())
    {
      out << '\n';
    }
  }

  bool criticalPathCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string&,
      std::ostream& out)
  {
    const shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project)
    {
      return false;
    }
    shaykhraziev::CriticalPath path;
    if (!shaykhraziev::calculateCriticalPath(*project, path))
    {
      return false;
    }
    out << "Critical path: " << project->getName() << "\n\n";
    out << "Duration: " << path.duration << '\n';
    out << "Path:\n";
    if (path.taskIds.empty())
    {
      out << "  <empty>\n";
    }
    else
    {
      out << "  ";
      printCriticalPath(path, out);
    }
    return true;
  }

  bool tryTaskCommand(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string& line,
      std::ostream& out)
  {
    const shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    std::size_t duration = 0;
    std::size_t deadline = 0;
    if (!project ||
        project->findTask(tokenAt(tokens, 2)) ||
        !shaykhraziev::parsePositiveSize(tokenAt(tokens, 3), duration) ||
        !shaykhraziev::parsePositiveSize(tokenAt(tokens, 4), deadline) ||
        deadline < project->getStartDay())
    {
      return false;
    }

    shaykhraziev::Project trial = *project;
    const std::string taskId = tokenAt(tokens, 2);
    const std::string title = getTailAfterTokens(line, 5);
    if (!trial.addTask(taskId, duration, title) || !shaykhraziev::buildProjectPlan(trial))
    {
      return false;
    }
    const shaykhraziev::PlannedTask* planned = trial.getPlan().findTask(taskId);
    if (!planned)
    {
      return false;
    }
    if (planned->endDay <= deadline)
    {
      out << "Task can be scheduled before deadline.\n";
      out << "Task: " << taskId << '\n';
      out << "Worker: " << planned->workerId << '\n';
      out << "Start: " << planned->startDay << '\n';
      out << "End: " << planned->endDay << '\n';
    }
    else
    {
      out << "Task cannot be scheduled before deadline.\n";
    }
    return true;
  }
}

shaykhraziev::CommandRegistry shaykhraziev::makeCommandRegistry()
{
  CommandRegistry commands(8, 4);
  commands.add("make-project", CommandHandler{3, 3, makeProjectCommand,
      "make-project <name> <startDay> <workersCount>", "Create a new project.", "make-project site 1 2"});
  commands.add("drop-project", CommandHandler{1, 1, dropProjectCommand,
      "drop-project <name>", "Delete an existing project.", "drop-project site"});
  commands.add("show-project", CommandHandler{1, 1, showProjectCommand,
      "show-project <name>", "Show project summary.", "show-project site"});
  commands.add("add-task", CommandHandler{4, UNLIMITED_ARGUMENTS, addTaskCommand,
      "add-task <project> <taskId> <duration> <title...>", "Add a task to an existing project.",
      "add-task site backend 4 Backend implementation"});
  commands.add("drop-task", CommandHandler{2, 2, dropTaskCommand,
      "drop-task <project> <taskId>", "Delete a task and remove dependencies that reference it.",
      "drop-task site backend"});
  commands.add("show-task", CommandHandler{2, 2, showTaskCommand,
      "show-task <project> <taskId>", "Show task details and dependencies.", "show-task site backend"});
  commands.add("add-dependency", CommandHandler{3, 3, addDependencyCommand,
      "add-dependency <project> <taskId> <dependencyId>", "Make task depend on another task.",
      "add-dependency site backend design"});
  commands.add("drop-dependency", CommandHandler{3, 3, dropDependencyCommand,
      "drop-dependency <project> <taskId> <dependencyId>", "Remove a dependency between two tasks.",
      "drop-dependency site backend design"});
  commands.add("check-cycles", CommandHandler{1, 1, checkCyclesCommand,
      "check-cycles <project>", "Check whether project dependencies contain a cycle.", "check-cycles site"});
  commands.add("build-plan", CommandHandler{1, 1, buildPlanCommand,
      "build-plan <project>", "Build a calendar plan for project tasks.", "build-plan site"});
  commands.add("show-worker", CommandHandler{2, 2, showWorkerCommand,
      "show-worker <project> <workerId>", "Show tasks assigned to a worker after plan building.",
      "show-worker site 1"});
  commands.add("stats", CommandHandler{1, 1, statsCommand,
      "stats <project>", "Show project statistics.", "stats site"});
  commands.add("show-gantt", CommandHandler{1, 1, showGanttCommand,
      "show-gantt <project>", "Show ASCII Gantt chart after plan building.", "show-gantt site"});
  commands.add("critical-path", CommandHandler{1, 1, criticalPathCommand,
      "critical-path <project>", "Calculate the longest dependency path.", "critical-path site"});
  commands.add("try-task", CommandHandler{5, UNLIMITED_ARGUMENTS, tryTaskCommand,
      "try-task <project> <taskId> <duration> <deadline> <title...>",
      "Check whether a new independent task can finish before a deadline.", "try-task site qa 2 9 QA task"});
  commands.add("help", CommandHandler{0, 1, helpCommand,
      "help [command]", "Show general help or details for one command.", "help add-task"});
  return commands;
}

bool shaykhraziev::executeCommandLine(
    ProjectStorage& storage,
    const CommandRegistry& commands,
    const std::string& line,
    std::ostream& out)
{
  List< std::string > tokens = splitTokens(line);
  if (tokens.empty())
  {
    return true;
  }

  const CommandHandler* handler = commands.find(tokenAt(tokens, 0));
  const std::string commandName = tokenAt(tokens, 0);
  if (!handler)
  {
    printUnknownCommand(commandName, out);
    return false;
  }
  if (!checkArgumentCount(*handler, tokens.size()))
  {
    printInvalidArguments(commandName, *handler, out);
    return false;
  }

  const bool ok = handler->function(storage, tokens, line, out);
  if (!ok)
  {
    printCommandFailure(commandName, *handler, out);
  }
  return ok;
}

void shaykhraziev::processCommands(
    ProjectStorage& storage,
    const CommandRegistry& commands,
    std::istream& in,
    std::ostream& out)
{
  std::string line;
  out << " > ";
  while (std::getline(in, line))
  {
    executeCommandLine(storage, commands, line, out);
    out << " > ";
  }
}
