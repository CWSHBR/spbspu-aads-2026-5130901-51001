#ifndef S4_DICTIONARY_OPERATIONS_HPP
#define S4_DICTIONARY_OPERATIONS_HPP

#include "io.hpp"

namespace shaykhraziev
{
  Dictionary complementDictionaries(const Dictionary& left, const Dictionary& right);
  Dictionary intersectDictionaries(const Dictionary& left, const Dictionary& right);
  Dictionary uniteDictionaries(const Dictionary& left, const Dictionary& right);
}

#endif
