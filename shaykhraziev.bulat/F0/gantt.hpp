#ifndef F0_GANTT_HPP
#define F0_GANTT_HPP

#include <iosfwd>

namespace shaykhraziev
{
  class Project;

  void renderGantt(const Project& project, std::ostream& out);
}

#endif
