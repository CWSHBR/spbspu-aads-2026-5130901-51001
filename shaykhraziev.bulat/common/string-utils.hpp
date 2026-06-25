#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include "list.hpp"

#include <limits>
#include <string>

namespace shaykhraziev
{
  inline bool isAsciiLetter(char c)
  {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
  }

  inline bool isAsciiDigit(char c)
  {
    return c >= '0' && c <= '9';
  }

  inline bool isSpace(char c)
  {
    return c == ' ' || c == '\t' || c == '\r';
  }

  inline bool isValidName(const std::string& value)
  {
    if (value.empty() || !isAsciiLetter(value[0]))
    {
      return false;
    }
    for (std::size_t i = 1; i < value.size(); ++i)
    {
      if (!isAsciiLetter(value[i]) && !isAsciiDigit(value[i]))
      {
        return false;
      }
    }
    return true;
  }

  inline List< std::string > splitTokens(const std::string& line)
  {
    List< std::string > tokens;
    std::size_t index = 0;
    while (index < line.size())
    {
      while (index < line.size() && isSpace(line[index]))
      {
        ++index;
      }
      std::size_t start = index;
      while (index < line.size() && !isSpace(line[index]))
      {
        ++index;
      }
      if (start != index)
      {
        tokens.pushBack(line.substr(start, index - start));
      }
    }
    return tokens;
  }

  inline bool parseUnsigned(const std::string& value, unsigned long long& result)
  {
    if (value.empty())
    {
      return false;
    }
    unsigned long long parsed = 0;
    const unsigned long long limit = std::numeric_limits< unsigned long long >::max();
    for (std::size_t i = 0; i < value.size(); ++i)
    {
      if (!isAsciiDigit(value[i]))
      {
        return false;
      }
      unsigned long long digit = static_cast< unsigned long long >(value[i] - '0');
      if (parsed > (limit - digit) / 10)
      {
        return false;
      }
      parsed = parsed * 10 + digit;
    }
    result = parsed;
    return true;
  }
}

#endif
