#ifndef LIST_UTILS_HPP
#define LIST_UTILS_HPP

#include "list.hpp"

namespace shaykhraziev
{
  template< class T, class Compare >
  void insertSorted(List< T >& list, const T& value, Compare compare)
  {
    List< T > result;
    bool inserted = false;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
      if (!inserted && compare(value, *it))
      {
        result.pushBack(value);
        inserted = true;
      }
      result.pushBack(*it);
    }
    if (!inserted)
    {
      result.pushBack(value);
    }
    list = result;
  }

  template< class T, class Compare >
  void insertSortedUnique(List< T >& list, const T& value, Compare compare)
  {
    List< T > result;
    bool inserted = false;
    bool found = false;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
      if (!inserted && compare(value, *it))
      {
        result.pushBack(value);
        inserted = true;
      }
      if (!compare(*it, value) && !compare(value, *it))
      {
        found = true;
      }
      result.pushBack(*it);
    }
    if (!inserted && !found)
    {
      result.pushBack(value);
    }
    if (!found)
    {
      list = result;
    }
  }
}

#endif
