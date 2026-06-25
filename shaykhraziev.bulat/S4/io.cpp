#include "io.hpp"

#include "../common/string-utils.hpp"

#include <fstream>
#include <istream>
#include <limits>
#include <stdexcept>
#include <string>

bool shaykhraziev::IntLess::operator()(int lhs, int rhs) const
{
  return lhs < rhs;
}

bool shaykhraziev::StringLess::operator()(const std::string& lhs, const std::string& rhs) const
{
  return lhs < rhs;
}

bool shaykhraziev::parseInt(const std::string& value, int& result)
{
  if (value.empty())
  {
    return false;
  }

  bool negative = false;
  std::size_t index = 0;
  if (value[index] == '-')
  {
    negative = true;
    ++index;
  }
  if (index == value.size())
  {
    return false;
  }

  const unsigned long long positiveLimit = std::numeric_limits< int >::max();
  const unsigned long long negativeLimit = positiveLimit + 1ULL;
  const unsigned long long limit = negative ? negativeLimit : positiveLimit;
  unsigned long long parsed = 0;
  while (index < value.size())
  {
    if (!isAsciiDigit(value[index]))
    {
      return false;
    }
    const unsigned long long digit = static_cast< unsigned long long >(value[index] - '0');
    if (parsed > (limit - digit) / 10)
    {
      return false;
    }
    parsed = parsed * 10 + digit;
    ++index;
  }

  if (negative && parsed == negativeLimit)
  {
    result = std::numeric_limits< int >::min();
  }
  else if (negative)
  {
    result = -static_cast< int >(parsed);
  }
  else
  {
    result = static_cast< int >(parsed);
  }
  return true;
}

shaykhraziev::DatasetTable shaykhraziev::readDatasets(std::istream& in)
{
  DatasetTable datasets;
  std::string line;
  while (std::getline(in, line))
  {
    List< std::string > tokens = splitTokens(line);
    if (tokens.empty())
    {
      continue;
    }
    if (tokens.size() % 2 == 0)
    {
      throw std::logic_error("invalid dataset file");
    }

    auto token = tokens.begin();
    const std::string datasetName = *token;
    if (!isValidName(datasetName) || datasets.has(datasetName))
    {
      throw std::logic_error("invalid dataset file");
    }
    ++token;

    Dictionary dictionary;
    while (token != tokens.end())
    {
      int key = 0;
      if (!parseInt(*token, key))
      {
        throw std::logic_error("invalid dataset file");
      }
      ++token;
      const std::string value = *token;
      ++token;
      dictionary.push(key, value);
    }

    datasets.push(datasetName, dictionary);
  }
  return datasets;
}

shaykhraziev::DatasetTable shaykhraziev::readDatasetsFromFile(const char* filename)
{
  std::ifstream file(filename);
  if (!file)
  {
    throw std::runtime_error("cannot open file");
  }
  return readDatasets(file);
}
