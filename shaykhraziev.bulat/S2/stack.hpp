#ifndef STACK_HPP
#define STACK_HPP

#include "../common/list.hpp"
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace shaykhraziev
{
  template< class T >
  class Stack
  {
  public:
    bool empty() const noexcept
    {
      return data_.empty();
    }

    std::size_t size() const noexcept
    {
      return data_.size();
    }

    T& top()
    {
      if (empty())
      {
        throw std::logic_error("empty stack");
      }
      return data_.front();
    }

    const T& top() const
    {
      if (empty())
      {
        throw std::logic_error("empty stack");
      }
      return data_.front();
    }

    void push(const T& value)
    {
      data_.pushFront(value);
    }

    void push(T&& value)
    {
      data_.pushFront(std::move(value));
    }

    void pop()
    {
      if (empty())
      {
        throw std::logic_error("empty stack");
      }
      data_.popFront();
    }

    T drop()
    {
      if (empty())
      {
        throw std::logic_error("empty stack");
      }
      T value = std::move(data_.front());
      data_.popFront();
      return value;
    }

    void swap(Stack& other) noexcept
    {
      data_.swap(other.data_);
    }

  private:
    List< T > data_;
  };
}

#endif
