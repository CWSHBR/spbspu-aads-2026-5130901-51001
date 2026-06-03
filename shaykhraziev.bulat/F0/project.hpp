#ifndef F0_PROJECT_HPP
#define F0_PROJECT_HPP

#include <cuckoo-hash-table.hpp>
#include <hash-functions.hpp>
#include <list.hpp>

#include <cstddef>
#include <string>

#include "planner.hpp"

namespace shaykhraziev
{
  struct Task
  {
    std::string id;
    std::string title;
    std::size_t duration;
    List< std::string > dependencies;
    List< std::string > dependents;

    Task();
    Task(const std::string& newId, std::size_t newDuration, const std::string& newTitle);
  };

  class Project
  {
  public:
    Project();
    Project(const std::string& name, std::size_t startDay, std::size_t workersCount);

    const std::string& getName() const noexcept;
    std::size_t getStartDay() const noexcept;
    std::size_t getWorkersCount() const noexcept;
    std::size_t countTasks() const noexcept;
    std::size_t getTotalDuration() const noexcept;
    bool isPlanBuilt() const noexcept;
    void resetPlan() noexcept;
    void setPlan(const Plan& plan);
    const Plan& getPlan() const noexcept;

    bool addTask(const std::string& taskId, std::size_t duration, const std::string& title);
    bool dropTask(const std::string& taskId);
    bool addDependency(const std::string& taskId, const std::string& dependencyId);
    bool dropDependency(const std::string& taskId, const std::string& dependencyId);
    bool hasDependency(const std::string& taskId, const std::string& dependencyId) const;
    bool hasCycle() const;
    bool getTopologicalOrder(List< std::string >& order) const;
    Task* findTask(const std::string& taskId);
    const Task* findTask(const std::string& taskId) const;
    const List< std::string >& getTaskOrder() const noexcept;

  private:
    using TaskTable = CuckooHashTable< std::string, Task, HmacHash, StringEqual >;

    std::string name_;
    std::size_t startDay_;
    std::size_t workersCount_;
    TaskTable tasks_;
    List< std::string > taskOrder_;
    bool planBuilt_;
    Plan lastPlan_;

    void ensureTaskSpace();
    void removeTaskFromOrder(const std::string& taskId);
    void removeTaskFromDependencies(const std::string& taskId);
  };

  class ProjectStorage
  {
  public:
    ProjectStorage();

    bool makeProject(const std::string& name, std::size_t startDay, std::size_t workersCount);
    bool dropProject(const std::string& name);
    Project* findProject(const std::string& name);
    const Project* findProject(const std::string& name) const;
    std::size_t countProjects() const noexcept;

  private:
    using ProjectTable = CuckooHashTable< std::string, Project, HmacHash, StringEqual >;

    ProjectTable projects_;

    void ensureProjectSpace();
  };
}

#endif
