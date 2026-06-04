#include "project.hpp"

#include <string-utils.hpp>

namespace
{
  const std::size_t INITIAL_SLOTS = 8;
  const std::size_t BUCKET_SIZE = 4;

  bool isValidPositive(std::size_t value)
  {
    return value >= 1;
  }

  bool containsString(const shaykhraziev::List< std::string >& values, const std::string& value)
  {
    for (shaykhraziev::List< std::string >::const_iterator it = values.cbegin(); it != values.cend(); ++it)
    {
      if (*it == value)
      {
        return true;
      }
    }
    return false;
  }

  void removeString(shaykhraziev::List< std::string >& values, const std::string& value)
  {
    shaykhraziev::List< std::string > kept;
    for (shaykhraziev::List< std::string >::const_iterator it = values.cbegin(); it != values.cend(); ++it)
    {
      if (*it != value)
      {
        kept.pushBack(*it);
      }
    }
    values.swap(kept);
  }

  using VisitStateTable = shaykhraziev::CuckooHashTable<
      std::string,
      std::size_t,
      shaykhraziev::HmacHash,
      shaykhraziev::StringEqual >;

  const std::size_t UNVISITED = 0;
  const std::size_t VISITING = 1;
  const std::size_t VISITED = 2;

  std::size_t getVisitState(const VisitStateTable& states, const std::string& taskId)
  {
    const std::size_t* state = states.find(taskId);
    return state ? *state : UNVISITED;
  }

  bool visitTask(
      const shaykhraziev::Project& project,
      const std::string& taskId,
      VisitStateTable& states,
      shaykhraziev::List< std::string >* order)
  {
    const std::size_t state = getVisitState(states, taskId);
    if (state == VISITING)
    {
      return false;
    }
    if (state == VISITED)
    {
      return true;
    }

    states.set(taskId, VISITING);
    const shaykhraziev::Task* task = project.findTask(taskId);
    if (!task)
    {
      return false;
    }
    for (shaykhraziev::List< std::string >::const_iterator it = task->dependencies.cbegin();
        it != task->dependencies.cend();
        ++it)
    {
      if (!visitTask(project, *it, states, order))
      {
        return false;
      }
    }
    states.set(taskId, VISITED);
    if (order)
    {
      order->pushBack(taskId);
    }
    return true;
  }
}

shaykhraziev::Task::Task():
  id(),
  title(),
  duration(0),
  dependencies(),
  dependents()
{}

shaykhraziev::Task::Task(const std::string& newId, std::size_t newDuration, const std::string& newTitle):
  id(newId),
  title(newTitle),
  duration(newDuration),
  dependencies(),
  dependents()
{}

shaykhraziev::Project::Project():
  name_(),
  startDay_(0),
  workersCount_(0),
  tasks_(INITIAL_SLOTS, BUCKET_SIZE),
  taskOrder_(),
  planBuilt_(false),
  lastPlan_()
{}

shaykhraziev::Project::Project(
    const std::string& name,
    std::size_t startDay,
    std::size_t workersCount):
  name_(name),
  startDay_(startDay),
  workersCount_(workersCount),
  tasks_(INITIAL_SLOTS, BUCKET_SIZE),
  taskOrder_(),
  planBuilt_(false),
  lastPlan_()
{}

const std::string& shaykhraziev::Project::getName() const noexcept
{
  return name_;
}

std::size_t shaykhraziev::Project::getStartDay() const noexcept
{
  return startDay_;
}

std::size_t shaykhraziev::Project::getWorkersCount() const noexcept
{
  return workersCount_;
}

std::size_t shaykhraziev::Project::countTasks() const noexcept
{
  return tasks_.size();
}

std::size_t shaykhraziev::Project::getTotalDuration() const noexcept
{
  std::size_t total = 0;
  for (TaskTable::const_iterator it = tasks_.cbegin(); it != tasks_.cend(); ++it)
  {
    total += it->value.duration;
  }
  return total;
}

bool shaykhraziev::Project::isPlanBuilt() const noexcept
{
  return planBuilt_;
}

void shaykhraziev::Project::resetPlan() noexcept
{
  planBuilt_ = false;
}

void shaykhraziev::Project::setPlan(const Plan& plan)
{
  lastPlan_ = plan;
  planBuilt_ = true;
}

const shaykhraziev::Plan& shaykhraziev::Project::getPlan() const noexcept
{
  return lastPlan_;
}

bool shaykhraziev::Project::addTask(
    const std::string& taskId,
    std::size_t duration,
    const std::string& title)
{
  if (!isValidName(taskId) || !isValidPositive(duration) || title.empty() || tasks_.has(taskId))
  {
    return false;
  }
  ensureTaskSpace();
  tasks_.add(taskId, Task(taskId, duration, title));
  taskOrder_.pushBack(taskId);
  resetPlan();
  return true;
}

bool shaykhraziev::Project::dropTask(const std::string& taskId)
{
  if (!tasks_.drop(taskId))
  {
    return false;
  }
  removeTaskFromOrder(taskId);
  removeTaskFromDependencies(taskId);
  resetPlan();
  return true;
}

bool shaykhraziev::Project::addDependency(const std::string& taskId, const std::string& dependencyId)
{
  if (taskId == dependencyId)
  {
    return false;
  }
  Task* task = findTask(taskId);
  Task* dependency = findTask(dependencyId);
  if (!task || !dependency || containsString(task->dependencies, dependencyId))
  {
    return false;
  }
  task->dependencies.pushBack(dependencyId);
  dependency->dependents.pushBack(taskId);
  if (hasCycle())
  {
    removeString(task->dependencies, dependencyId);
    removeString(dependency->dependents, taskId);
    return false;
  }
  resetPlan();
  return true;
}

bool shaykhraziev::Project::dropDependency(const std::string& taskId, const std::string& dependencyId)
{
  Task* task = findTask(taskId);
  Task* dependency = findTask(dependencyId);
  if (!task || !dependency || !containsString(task->dependencies, dependencyId))
  {
    return false;
  }
  removeString(task->dependencies, dependencyId);
  removeString(dependency->dependents, taskId);
  resetPlan();
  return true;
}

bool shaykhraziev::Project::hasDependency(const std::string& taskId, const std::string& dependencyId) const
{
  const Task* task = findTask(taskId);
  return task && containsString(task->dependencies, dependencyId);
}

bool shaykhraziev::Project::hasCycle() const
{
  List< std::string > order;
  return !getTopologicalOrder(order);
}

bool shaykhraziev::Project::getTopologicalOrder(List< std::string >& order) const
{
  order.clear();
  VisitStateTable states(countTasks() + 1, BUCKET_SIZE);
  for (List< std::string >::const_iterator it = taskOrder_.cbegin(); it != taskOrder_.cend(); ++it)
  {
    if (!visitTask(*this, *it, states, &order))
    {
      order.clear();
      return false;
    }
  }
  return true;
}

shaykhraziev::Task* shaykhraziev::Project::findTask(const std::string& taskId)
{
  return tasks_.find(taskId);
}

const shaykhraziev::Task* shaykhraziev::Project::findTask(const std::string& taskId) const
{
  return tasks_.find(taskId);
}

const shaykhraziev::List< std::string >& shaykhraziev::Project::getTaskOrder() const noexcept
{
  return taskOrder_;
}

void shaykhraziev::Project::ensureTaskSpace()
{
  if (tasks_.size() == tasks_.capacity())
  {
    tasks_.rehash(tasks_.slots() * 2);
  }
}

void shaykhraziev::Project::removeTaskFromOrder(const std::string& taskId)
{
  removeString(taskOrder_, taskId);
}

void shaykhraziev::Project::removeTaskFromDependencies(const std::string& taskId)
{
  for (TaskTable::iterator it = tasks_.begin(); it != tasks_.end(); ++it)
  {
    removeString(it->value.dependencies, taskId);
    removeString(it->value.dependents, taskId);
  }
}

shaykhraziev::ProjectStorage::ProjectStorage():
  projects_(INITIAL_SLOTS, BUCKET_SIZE),
  projectNames_()
{}

bool shaykhraziev::ProjectStorage::makeProject(
    const std::string& name,
    std::size_t startDay,
    std::size_t workersCount)
{
  if (!isValidName(name) || !isValidPositive(startDay) || !isValidPositive(workersCount) || projects_.has(name))
  {
    return false;
  }
  ensureProjectSpace();
  projects_.add(name, Project(name, startDay, workersCount));
  projectNames_.pushBack(name);
  return true;
}

bool shaykhraziev::ProjectStorage::dropProject(const std::string& name)
{
  if (!projects_.drop(name))
  {
    return false;
  }
  removeString(projectNames_, name);
  return true;
}

shaykhraziev::Project* shaykhraziev::ProjectStorage::findProject(const std::string& name)
{
  return projects_.find(name);
}

const shaykhraziev::Project* shaykhraziev::ProjectStorage::findProject(const std::string& name) const
{
  return projects_.find(name);
}

std::size_t shaykhraziev::ProjectStorage::countProjects() const noexcept
{
  return projects_.size();
}

const shaykhraziev::List< std::string >& shaykhraziev::ProjectStorage::getProjectNames() const noexcept
{
  return projectNames_;
}

void shaykhraziev::ProjectStorage::ensureProjectSpace()
{
  if (projects_.size() == projects_.capacity())
  {
    projects_.rehash(projects_.slots() * 2);
  }
}
