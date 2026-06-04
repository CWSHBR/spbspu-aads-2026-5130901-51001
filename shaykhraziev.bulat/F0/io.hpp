#ifndef F0_IO_HPP
#define F0_IO_HPP

#include "project.hpp"

#include <iosfwd>

namespace shaykhraziev
{
  bool parsePositiveSize(const std::string& token, std::size_t& value);
  ProjectStorage readProjects(std::istream& in);
  ProjectStorage readProjectsFromFile(const char* filename);
  void writeProjects(const ProjectStorage& storage, std::ostream& out);
  void writeProjectsToFile(const ProjectStorage& storage, const std::string& filename);
}

#endif
