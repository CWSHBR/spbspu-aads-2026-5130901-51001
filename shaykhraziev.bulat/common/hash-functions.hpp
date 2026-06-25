#ifndef HASH_FUNCTIONS_HPP
#define HASH_FUNCTIONS_HPP

#include <cstddef>
#include <string>
#include <utility>

namespace shaykhraziev
{
  namespace detail
  {
    const std::size_t HMAC_BLOCK_SIZE = 64;
    const std::size_t HMAC_SEED = 1469598103934665603ULL;
    const std::size_t HMAC_PRIME = 1099511628211ULL;

    inline std::size_t mixByte(std::size_t state, unsigned char byte)
    {
      state ^= byte;
      state *= HMAC_PRIME;
      state ^= state >> 23;
      return state;
    }

    inline std::size_t finishHash(std::size_t state)
    {
      state ^= state >> 33;
      state *= 0xff51afd7ed558ccdULL;
      state ^= state >> 33;
      state *= 0xc4ceb9fe1a85ec53ULL;
      state ^= state >> 33;
      return state;
    }
  }

  class HmacHash
  {
  public:
    std::size_t operator()(const unsigned char* data, std::size_t size) const
    {
      const unsigned char key[] = "shaykhraziev-s3-hmac-key";
      unsigned char inner[detail::HMAC_BLOCK_SIZE];
      unsigned char outer[detail::HMAC_BLOCK_SIZE];

      for (std::size_t i = 0; i < detail::HMAC_BLOCK_SIZE; ++i)
      {
        unsigned char keyByte = 0;
        if (i + 1 < sizeof(key))
        {
          keyByte = key[i];
        }
        inner[i] = static_cast< unsigned char >(keyByte ^ 0x36);
        outer[i] = static_cast< unsigned char >(keyByte ^ 0x5c);
      }

      std::size_t innerState = detail::HMAC_SEED;
      for (std::size_t i = 0; i < detail::HMAC_BLOCK_SIZE; ++i)
      {
        innerState = detail::mixByte(innerState, inner[i]);
      }
      for (std::size_t i = 0; i < size; ++i)
      {
        innerState = detail::mixByte(innerState, data[i]);
      }
      innerState = detail::finishHash(innerState);

      std::size_t outerState = detail::HMAC_SEED;
      for (std::size_t i = 0; i < detail::HMAC_BLOCK_SIZE; ++i)
      {
        outerState = detail::mixByte(outerState, outer[i]);
      }
      const unsigned char* digest = reinterpret_cast< const unsigned char* >(&innerState);
      for (std::size_t i = 0; i < sizeof(innerState); ++i)
      {
        outerState = detail::mixByte(outerState, digest[i]);
      }
      return detail::finishHash(outerState);
    }

    std::size_t operator()(const char* data, std::size_t size) const
    {
      return operator()(reinterpret_cast< const unsigned char* >(data), size);
    }

    std::size_t operator()(const std::string& value) const
    {
      return operator()(value.data(), value.size());
    }
  };

  struct StringEqual
  {
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
      return lhs == rhs;
    }
  };

  template< class FirstHash, class SecondHash >
  class PairHash
  {
  public:
    template< class First, class Second >
    std::size_t operator()(const std::pair< First, Second >& value) const
    {
      FirstHash firstHash;
      SecondHash secondHash;
      std::size_t result = firstHash(value.first);
      std::size_t next = secondHash(value.second);
      const unsigned char* bytes = reinterpret_cast< const unsigned char* >(&next);
      for (std::size_t i = 0; i < sizeof(next); ++i)
      {
        result = detail::mixByte(result, bytes[i]);
      }
      return detail::finishHash(result);
    }
  };

  template< class FirstEqual, class SecondEqual >
  class PairEqual
  {
  public:
    template< class First, class Second >
    bool operator()(const std::pair< First, Second >& lhs, const std::pair< First, Second >& rhs) const
    {
      FirstEqual firstEqual;
      SecondEqual secondEqual;
      return firstEqual(lhs.first, rhs.first) && secondEqual(lhs.second, rhs.second);
    }
  };
}

#endif
