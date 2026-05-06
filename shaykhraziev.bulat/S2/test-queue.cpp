#include <boost/test/unit_test.hpp>

#include "queue.hpp"

#include <stdexcept>
#include <string>
#include <utility>

BOOST_AUTO_TEST_CASE(queue_default_constructor_creates_empty_queue)
{
  shaykhraziev::Queue< int > queue;

  BOOST_CHECK(queue.empty());
  BOOST_TEST(queue.size() == 0);
}

BOOST_AUTO_TEST_CASE(queue_front_back_pop_and_drop_throw_on_empty_queue)
{
  shaykhraziev::Queue< int > queue;
  const shaykhraziev::Queue< int >& constQueue = queue;

  BOOST_CHECK_THROW(queue.front(), std::logic_error);
  BOOST_CHECK_THROW(constQueue.front(), std::logic_error);
  BOOST_CHECK_THROW(queue.back(), std::logic_error);
  BOOST_CHECK_THROW(constQueue.back(), std::logic_error);
  BOOST_CHECK_THROW(queue.pop(), std::logic_error);
  BOOST_CHECK_THROW(queue.drop(), std::logic_error);
}

BOOST_AUTO_TEST_CASE(queue_push_and_pop_follow_fifo_order)
{
  shaykhraziev::Queue< int > queue;
  queue.push(10);
  queue.push(20);
  queue.push(30);

  BOOST_CHECK(!queue.empty());
  BOOST_TEST(queue.size() == 3);
  BOOST_TEST(queue.front() == 10);
  BOOST_TEST(queue.back() == 30);

  queue.pop();
  BOOST_TEST(queue.size() == 2);
  BOOST_TEST(queue.front() == 20);
  BOOST_TEST(queue.back() == 30);

  queue.pop();
  BOOST_TEST(queue.size() == 1);
  BOOST_TEST(queue.front() == 30);
  BOOST_TEST(queue.back() == 30);

  queue.pop();
  BOOST_CHECK(queue.empty());
  BOOST_TEST(queue.size() == 0);
}

BOOST_AUTO_TEST_CASE(queue_non_const_front_and_back_return_mutable_references)
{
  shaykhraziev::Queue< std::string > queue;
  queue.push("front");
  queue.push("back");

  queue.front() = "updated-front";
  queue.back() = "updated-back";

  BOOST_TEST(queue.front() == "updated-front");
  BOOST_TEST(queue.back() == "updated-back");
}

BOOST_AUTO_TEST_CASE(queue_const_front_and_back_return_queue_ends)
{
  shaykhraziev::Queue< std::string > queue;
  queue.push("first");
  queue.push("second");

  const shaykhraziev::Queue< std::string >& constQueue = queue;

  BOOST_TEST(constQueue.front() == "first");
  BOOST_TEST(constQueue.back() == "second");
  BOOST_TEST(constQueue.size() == 2);
}

BOOST_AUTO_TEST_CASE(queue_drop_returns_front_element_and_updates_queue)
{
  shaykhraziev::Queue< std::string > queue;
  queue.push("first");
  queue.push("second");
  queue.push("third");

  std::string value = queue.drop();

  BOOST_TEST(value == "first");
  BOOST_TEST(queue.size() == 2);
  BOOST_TEST(queue.front() == "second");
  BOOST_TEST(queue.back() == "third");
}

BOOST_AUTO_TEST_CASE(queue_push_rvalue_preserves_value)
{
  shaykhraziev::Queue< std::string > queue;
  std::string value = "payload";

  queue.push(std::move(value));

  BOOST_TEST(queue.front() == "payload");
  BOOST_TEST(queue.back() == "payload");
  BOOST_TEST(queue.size() == 1);
}

BOOST_AUTO_TEST_CASE(queue_swap_exchanges_contents_of_queues)
{
  shaykhraziev::Queue< int > left;
  left.push(1);
  left.push(2);

  shaykhraziev::Queue< int > right;
  right.push(100);
  right.push(200);
  right.push(300);

  left.swap(right);

  BOOST_TEST(left.size() == 3);
  BOOST_TEST(left.front() == 100);
  BOOST_TEST(left.back() == 300);
  BOOST_TEST(right.size() == 2);
  BOOST_TEST(right.front() == 1);
  BOOST_TEST(right.back() == 2);
}

BOOST_AUTO_TEST_CASE(queue_copy_and_move_operations_preserve_queue_state)
{
  shaykhraziev::Queue< int > source;
  source.push(4);
  source.push(5);
  source.push(6);

  shaykhraziev::Queue< int > copy(source);
  BOOST_TEST(copy.size() == 3);
  BOOST_TEST(copy.front() == 4);
  BOOST_TEST(copy.back() == 6);

  copy.pop();
  BOOST_TEST(copy.front() == 5);
  BOOST_TEST(source.front() == 4);

  shaykhraziev::Queue< int > moved(std::move(source));
  BOOST_TEST(moved.size() == 3);
  BOOST_TEST(moved.front() == 4);
  BOOST_TEST(moved.back() == 6);
  BOOST_CHECK(source.empty());
}
