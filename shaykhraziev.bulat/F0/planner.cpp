#include "planner.hpp"

#include <vector>

#include "project.hpp"

namespace
{
  const std::size_t BUCKET_SIZE = 4;

  struct PathInfo
  {
    std::size_t duration;
    std::string previous;

    PathInfo():
      duration(0),
      previous()
    {}

    PathInfo(std::size_t newDuration, const std::string& newPrevious):
      duration(newDuration),
      previous(newPrevious)
    {}
  };

  using PathInfoTable = shaykhraziev::CuckooHashTable<
      std::string,
      PathInfo,
      shaykhraziev::HmacHash,
      shaykhraziev::StringEqual >;

  bool isPlanned(const shaykhraziev::Plan& plan, const std::string& taskId)
  {
    return plan.findTask(taskId) != nullptr;
  }

  bool areDependenciesPlanned(const shaykhraziev::Task& task, const shaykhraziev::Plan& plan)
  {
    for (shaykhraziev::List< std::string >::const_iterator it = task.dependencies.cbegin();
        it != task.dependencies.cend();
        ++it)
    {
      if (!isPlanned(plan, *it))
      {
        return false;
      }
    }
    return true;
  }

  std::size_t getEarliestDependencyStart(const shaykhraziev::Task& task, const shaykhraziev::Plan& plan)
  {
    std::size_t earliestStart = 0;
    for (shaykhraziev::List< std::string >::const_iterator it = task.dependencies.cbegin();
        it != task.dependencies.cend();
        ++it)
    {
      const shaykhraziev::PlannedTask* dependency = plan.findTask(*it);
      if (dependency && dependency->endDay + 1 > earliestStart)
      {
        earliestStart = dependency->endDay + 1;
      }
    }
    return earliestStart;
  }

  shaykhraziev::PlannedTask scheduleTask(
      const shaykhraziev::Project& project,
      const shaykhraziev::Task& task,
      const shaykhraziev::Plan& plan,
      std::vector< std::size_t >& workerAvailable)
  {
    std::size_t earliestStart = getEarliestDependencyStart(task, plan);
    if (earliestStart < project.getStartDay())
    {
      earliestStart = project.getStartDay();
    }

    std::size_t bestWorker = 1;
    std::size_t bestStart = earliestStart > workerAvailable[1] ? earliestStart : workerAvailable[1];
    for (std::size_t worker = 2; worker <= project.getWorkersCount(); ++worker)
    {
      const std::size_t workerStart = earliestStart > workerAvailable[worker] ? earliestStart : workerAvailable[worker];
      if (workerStart < bestStart)
      {
        bestWorker = worker;
        bestStart = workerStart;
      }
    }

    const std::size_t endDay = bestStart + task.duration - 1;
    workerAvailable[bestWorker] = endDay + 1;
    return shaykhraziev::PlannedTask(task.id, bestWorker, bestStart, endDay);
  }

  PathInfo calculatePathInfo(const shaykhraziev::Task& task, const PathInfoTable& infos)
  {
    std::size_t bestDuration = task.duration;
    std::string previous;
    for (shaykhraziev::List< std::string >::const_iterator it = task.dependencies.cbegin();
        it != task.dependencies.cend();
        ++it)
    {
      const PathInfo* dependency = infos.find(*it);
      if (dependency && dependency->duration + task.duration > bestDuration)
      {
        bestDuration = dependency->duration + task.duration;
        previous = *it;
      }
    }
    return PathInfo(bestDuration, previous);
  }

  void restoreCriticalPath(
      const PathInfoTable& infos,
      const std::string& lastTask,
      shaykhraziev::CriticalPath& path)
  {
    std::string current = lastTask;
    while (!current.empty())
    {
      path.taskIds.pushFront(current);
      const PathInfo* info = infos.find(current);
      current = info ? info->previous : "";
    }
  }
}

shaykhraziev::PlannedTask::PlannedTask():
  taskId(),
  workerId(0),
  startDay(0),
  endDay(0)
{}

shaykhraziev::PlannedTask::PlannedTask(
    const std::string& newTaskId,
    std::size_t newWorkerId,
    std::size_t newStartDay,
    std::size_t newEndDay):
  taskId(newTaskId),
  workerId(newWorkerId),
  startDay(newStartDay),
  endDay(newEndDay)
{}

shaykhraziev::Plan::Plan():
  tasks_(1, BUCKET_SIZE),
  taskOrder_(),
  projectEndDay_(0)
{}

shaykhraziev::Plan::Plan(std::size_t taskCount):
  tasks_(taskCount + 1, BUCKET_SIZE),
  taskOrder_(),
  projectEndDay_(0)
{}

void shaykhraziev::Plan::addTask(const PlannedTask& task)
{
  tasks_.add(task.taskId, task);
  taskOrder_.pushBack(task.taskId);
  if (task.endDay > projectEndDay_)
  {
    projectEndDay_ = task.endDay;
  }
}

const shaykhraziev::PlannedTask* shaykhraziev::Plan::findTask(const std::string& taskId) const
{
  return tasks_.find(taskId);
}

const shaykhraziev::List< std::string >& shaykhraziev::Plan::getTaskOrder() const noexcept
{
  return taskOrder_;
}

std::size_t shaykhraziev::Plan::getProjectEndDay() const noexcept
{
  return projectEndDay_;
}

std::size_t shaykhraziev::Plan::countTasks() const noexcept
{
  return tasks_.size();
}

shaykhraziev::CriticalPath::CriticalPath():
  taskIds(),
  duration(0)
{}

bool shaykhraziev::buildProjectPlan(Project& project)
{
  if (project.hasCycle())
  {
    return false;
  }

  Plan plan(project.countTasks());
  std::vector< std::size_t > workerAvailable(project.getWorkersCount() + 1, project.getStartDay());
  while (plan.countTasks() < project.countTasks())
  {
    bool madeProgress = false;
    for (List< std::string >::const_iterator it = project.getTaskOrder().cbegin();
        it != project.getTaskOrder().cend();
        ++it)
    {
      if (isPlanned(plan, *it))
      {
        continue;
      }
      const Task* task = project.findTask(*it);
      if (task && areDependenciesPlanned(*task, plan))
      {
        plan.addTask(scheduleTask(project, *task, plan, workerAvailable));
        madeProgress = true;
      }
    }
    if (!madeProgress)
    {
      return false;
    }
  }
  project.setPlan(plan);
  return true;
}

bool shaykhraziev::calculateCriticalPath(const Project& project, CriticalPath& path)
{
  path = CriticalPath();
  List< std::string > topologicalOrder;
  if (!project.getTopologicalOrder(topologicalOrder))
  {
    return false;
  }
  if (project.countTasks() == 0)
  {
    return true;
  }

  PathInfoTable infos(project.countTasks() + 1, BUCKET_SIZE);
  for (List< std::string >::const_iterator it = topologicalOrder.cbegin(); it != topologicalOrder.cend(); ++it)
  {
    const Task* task = project.findTask(*it);
    if (task)
    {
      infos.add(*it, calculatePathInfo(*task, infos));
    }
  }

  std::string lastTask;
  for (List< std::string >::const_iterator it = project.getTaskOrder().cbegin(); it != project.getTaskOrder().cend(); ++it)
  {
    const PathInfo* info = infos.find(*it);
    if (info && info->duration > path.duration)
    {
      path.duration = info->duration;
      lastTask = *it;
    }
  }
  restoreCriticalPath(infos, lastTask, path);
  return true;
}
