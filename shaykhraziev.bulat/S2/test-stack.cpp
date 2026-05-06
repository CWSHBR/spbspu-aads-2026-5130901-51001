#include <boost/test/unit_test.hpp>

#include "stack.hpp"

#include <stdexcept>
#include <string>
#include <utility>

BOOST_AUTO_TEST_CASE(default_constructor_creates_empty_stack)
{
  shaykhraziev::Stack< int > stack;

  BOOST_CHECK(stack.empty());
  BOOST_TEST(stack.size() == 0);
}

BOOST_AUTO_TEST_CASE(top_and_pop_throw_on_empty_stack)
{
  shaykhraziev::Stack< int > stack;
  const shaykhraziev::Stack< int >& constStack = stack;

  BOOST_CHECK_THROW(stack.top(), std::logic_error);
  BOOST_CHECK_THROW(constStack.top(), std::logic_error);
  BOOST_CHECK_THROW(stack.pop(), std::logic_error);
  BOOST_CHECK_THROW(stack.drop(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(push_and_pop_follow_lifo_order)
{
  shaykhraziev::Stack< int > stack;
  stack.push(10);
  stack.push(20);
  stack.push(30);

  BOOST_CHECK(!stack.empty());
  BOOST_TEST(stack.size() == 3);
  BOOST_TEST(stack.top() == 30);

  stack.pop();
  BOOST_TEST(stack.size() == 2);
  BOOST_TEST(stack.top() == 20);

  stack.pop();
  BOOST_TEST(stack.size() == 1);
  BOOST_TEST(stack.top() == 10);

  stack.pop();
  BOOST_CHECK(stack.empty());
  BOOST_TEST(stack.size() == 0);
}

BOOST_AUTO_TEST_CASE(non_const_top_returns_mutable_reference)
{
  shaykhraziev::Stack< std::string > stack;
  stack.push("value");

  stack.top() = "updated";

  BOOST_TEST(stack.top() == "updated");
}

BOOST_AUTO_TEST_CASE(const_top_returns_top_element)
{
  shaykhraziev::Stack< std::string > stack;
  stack.push("first");
  stack.push("second");

  const shaykhraziev::Stack< std::string >& constStack = stack;

  BOOST_TEST(constStack.top() == "second");
  BOOST_TEST(constStack.size() == 2);
}

BOOST_AUTO_TEST_CASE(drop_returns_removed_element_and_updates_stack)
{
  shaykhraziev::Stack< std::string > stack;
  stack.push("bottom");
  stack.push("middle");
  stack.push("top");

  std::string value = stack.drop();

  BOOST_TEST(value == "top");
  BOOST_TEST(stack.size() == 2);
  BOOST_TEST(stack.top() == "middle");
}

BOOST_AUTO_TEST_CASE(push_rvalue_preserves_value)
{
  shaykhraziev::Stack< std::string > stack;
  std::string value = "payload";

  stack.push(std::move(value));

  BOOST_TEST(stack.top() == "payload");
  BOOST_TEST(stack.size() == 1);
}

BOOST_AUTO_TEST_CASE(swap_exchanges_contents_of_stacks)
{
  shaykhraziev::Stack< int > left;
  left.push(1);
  left.push(2);

  shaykhraziev::Stack< int > right;
  right.push(100);

  left.swap(right);

  BOOST_TEST(left.size() == 1);
  BOOST_TEST(left.top() == 100);
  BOOST_TEST(right.size() == 2);
  BOOST_TEST(right.top() == 2);
}

BOOST_AUTO_TEST_CASE(copy_and_move_operations_preserve_stack_state)
{
  shaykhraziev::Stack< int > source;
  source.push(4);
  source.push(5);
  source.push(6);

  shaykhraziev::Stack< int > copy(source);
  BOOST_TEST(copy.size() == 3);
  BOOST_TEST(copy.top() == 6);

  copy.pop();
  BOOST_TEST(copy.top() == 5);
  BOOST_TEST(source.top() == 6);

  shaykhraziev::Stack< int > moved(std::move(source));
  BOOST_TEST(moved.size() == 3);
  BOOST_TEST(moved.top() == 6);
  BOOST_CHECK(source.empty());
}
