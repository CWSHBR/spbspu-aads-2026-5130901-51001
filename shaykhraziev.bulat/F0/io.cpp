#include "io.hpp"

#include "../common/string-utils.hpp"

#include <fstream>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace
{
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

  bool readProjectLine(shaykhraziev::ProjectStorage& storage, const shaykhraziev::List< std::string >& tokens)
  {
    if (tokens.size() != 4)
    {
      return false;
    }
    std::size_t startDay = 0;
    std::size_t workersCount = 0;
    return shaykhraziev::parsePositiveSize(tokenAt(tokens, 2), startDay) &&
        shaykhraziev::parsePositiveSize(tokenAt(tokens, 3), workersCount) &&
        storage.makeProject(tokenAt(tokens, 1), startDay, workersCount);
  }

  bool readTaskLine(
      shaykhraziev::ProjectStorage& storage,
      const shaykhraziev::List< std::string >& tokens,
      const std::string& line)
  {
    if (tokens.size() < 5)
    {
      return false;
    }
    std::size_t duration = 0;
    if (!shaykhraziev::parsePositiveSize(tokenAt(tokens, 3), duration))
    {
      return false;
    }
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    if (!project)
    {
      return false;
    }
    const std::string title = getTailAfterTokens(line, 4);
    return project->addTask(tokenAt(tokens, 2), duration, title);
  }

  bool readDependencyLine(shaykhraziev::ProjectStorage& storage, const shaykhraziev::List< std::string >& tokens)
  {
    if (tokens.size() != 4)
    {
      return false;
    }
    shaykhraziev::Project* project = storage.findProject(tokenAt(tokens, 1));
    return project && project->addDependency(tokenAt(tokens, 2), tokenAt(tokens, 3));
  }
}

bool shaykhraziev::parsePositiveSize(const std::string& token, std::size_t& value)
{
  unsigned long long parsed = 0;
  if (!parseUnsigned(token, parsed) || parsed == 0)
  {
    return false;
  }
  value = static_cast< std::size_t >(parsed);
  return static_cast< unsigned long long >(value) == parsed;
}

shaykhraziev::ProjectStorage shaykhraziev::readProjects(std::istream& in)
{
  ProjectStorage storage;
  std::string line;
  while (std::getline(in, line))
  {
    List< std::string > tokens = splitTokens(line);
    if (tokens.empty())
    {
      continue;
    }

    bool ok = false;
    const std::string type = tokenAt(tokens, 0);
    if (type == "project")
    {
      ok = readProjectLine(storage, tokens);
    }
    else if (type == "task")
    {
      ok = readTaskLine(storage, tokens, line);
    }
    else if (type == "dependency")
    {
      ok = readDependencyLine(storage, tokens);
    }

    if (!ok)
    {
      throw std::logic_error("invalid project file");
    }
  }
  return storage;
}

shaykhraziev::ProjectStorage shaykhraziev::readProjectsFromFile(const char* filename)
{
  std::ifstream file(filename);
  if (!file)
  {
    throw std::runtime_error("cannot open file");
  }
  return readProjects(file);
}

void shaykhraziev::writeProjects(const ProjectStorage& storage, std::ostream& out)
{
  for (List< std::string >::const_iterator pit = storage.getProjectNames().cbegin();
      pit != storage.getProjectNames().cend();
      ++pit)
  {
    const Project* project = storage.findProject(*pit);
    if (!project)
    {
      continue;
    }
    out << "project " << project->getName() << ' ' <<
        project->getStartDay() << ' ' <<
        project->getWorkersCount() << '\n';
    for (List< std::string >::const_iterator tit = project->getTaskOrder().cbegin();
        tit != project->getTaskOrder().cend();
        ++tit)
    {
      const Task* task = project->findTask(*tit);
      if (task)
      {
        out << "task " << project->getName() << ' ' <<
            task->id << ' ' <<
            task->duration << ' ' <<
            task->title << '\n';
      }
    }
    for (List< std::string >::const_iterator tit = project->getTaskOrder().cbegin();
        tit != project->getTaskOrder().cend();
        ++tit)
    {
      const Task* task = project->findTask(*tit);
      if (!task)
      {
        continue;
      }
      for (List< std::string >::const_iterator dit = task->dependencies.cbegin();
          dit != task->dependencies.cend();
          ++dit)
      {
        out << "dependency " << project->getName() << ' ' << task->id << ' ' << *dit << '\n';
      }
    }
  }
}

void shaykhraziev::writeProjectsToFile(const ProjectStorage& storage, const std::string& filename)
{
  std::ofstream file(filename.c_str());
  if (!file)
  {
    throw std::runtime_error("cannot write file");
  }
  writeProjects(storage, file);
}
