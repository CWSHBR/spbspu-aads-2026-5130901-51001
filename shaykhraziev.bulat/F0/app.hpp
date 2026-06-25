#ifndef F0_APP_HPP
#define F0_APP_HPP

#include <iosfwd>

namespace shaykhraziev
{
  int runF0(const char* filename, std::istream& in, std::ostream& out, std::ostream& err);
}

#endif
