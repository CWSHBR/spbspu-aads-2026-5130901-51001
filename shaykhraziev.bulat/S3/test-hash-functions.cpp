#include <boost/test/unit_test.hpp>

#include "../common/hash-functions.hpp"

#include <string>

BOOST_AUTO_TEST_CASE(hmac_hash_is_deterministic)
{
  shaykhraziev::HmacHash hash;
  std::string value = "alpha";

  BOOST_TEST(hash(value) == hash(value));
}

BOOST_AUTO_TEST_CASE(hmac_hash_distinguishes_common_inputs)
{
  shaykhraziev::HmacHash hash;

  BOOST_TEST(hash(std::string("alpha")) != hash(std::string("beta")));
  BOOST_TEST(hash(std::string("alpha")) != hash(std::string("alpha1")));
}

BOOST_AUTO_TEST_CASE(hmac_hash_accepts_empty_input)
{
  shaykhraziev::HmacHash hash;

  BOOST_TEST(hash(std::string()) == hash(std::string()));
}

BOOST_AUTO_TEST_CASE(hmac_hash_accepts_ascii_bytes)
{
  shaykhraziev::HmacHash hash;
  const char data[] = "AZaz09_-";

  BOOST_TEST(hash(data, sizeof(data) - 1) == hash(std::string("AZaz09_-")));
}
