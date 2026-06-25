#ifndef S4_APP_HPP
#define S4_APP_HPP

#include <iosfwd>

namespace shaykhraziev
{
  int runS4(const char* filename, std::istream& in, std::ostream& out, std::ostream& err);
}

#endif
