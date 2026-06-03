#ifndef F0_PLANNER_HPP
#define F0_PLANNER_HPP

#include <cuckoo-hash-table.hpp>
#include <hash-functions.hpp>
#include <list.hpp>

#include <cstddef>
#include <string>

namespace shaykhraziev
{
  class Project;

  struct PlannedTask
  {
    std::string taskId;
    std::size_t workerId;
    std::size_t startDay;
    std::size_t endDay;

    PlannedTask();
    PlannedTask(
        const std::string& newTaskId,
        std::size_t newWorkerId,
        std::size_t newStartDay,
        std::size_t newEndDay);
  };

  class Plan
  {
  public:
    Plan();
    explicit Plan(std::size_t taskCount);

    void addTask(const PlannedTask& task);
    const PlannedTask* findTask(const std::string& taskId) const;
    const List< std::string >& getTaskOrder() const noexcept;
    std::size_t getProjectEndDay() const noexcept;
    std::size_t countTasks() const noexcept;

  private:
    using PlannedTaskTable = CuckooHashTable< std::string, PlannedTask, HmacHash, StringEqual >;

    PlannedTaskTable tasks_;
    List< std::string > taskOrder_;
    std::size_t projectEndDay_;
  };

  struct CriticalPath
  {
    List< std::string > taskIds;
    std::size_t duration;

    CriticalPath();
  };

  bool buildProjectPlan(Project& project);
  bool calculateCriticalPath(const Project& project, CriticalPath& path);
}

#endif
