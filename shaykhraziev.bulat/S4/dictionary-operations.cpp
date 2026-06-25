#include "dictionary-operations.hpp"

shaykhraziev::Dictionary shaykhraziev::complementDictionaries(
    const Dictionary& left,
    const Dictionary& right)
{
  Dictionary result;
  for (Dictionary::const_iterator it = left.cbegin(); it != left.cend(); ++it)
  {
    if (!right.has(it->key))
    {
      result.push(it->key, it->value);
    }
  }
  return result;
}

shaykhraziev::Dictionary shaykhraziev::intersectDictionaries(
    const Dictionary& left,
    const Dictionary& right)
{
  Dictionary result;
  for (Dictionary::const_iterator it = left.cbegin(); it != left.cend(); ++it)
  {
    if (right.has(it->key))
    {
      result.push(it->key, it->value);
    }
  }
  return result;
}

shaykhraziev::Dictionary shaykhraziev::uniteDictionaries(
    const Dictionary& left,
    const Dictionary& right)
{
  Dictionary result;
  for (Dictionary::const_iterator it = right.cbegin(); it != right.cend(); ++it)
  {
    result.push(it->key, it->value);
  }
  for (Dictionary::const_iterator it = left.cbegin(); it != left.cend(); ++it)
  {
    result.push(it->key, it->value);
  }
  return result;
}
