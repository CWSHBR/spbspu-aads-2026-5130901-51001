#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "../common/list.hpp"
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace shaykhraziev
{
  template< class T >
  class Queue
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

    T& front()
    {
      if (empty())
      {
        throw std::logic_error("empty queue");
      }
      return data_.front();
    }

    const T& front() const
    {
      if (empty())
      {
        throw std::logic_error("empty queue");
      }
      return data_.front();
    }

    T& back()
    {
      if (empty())
      {
        throw std::logic_error("empty queue");
      }
      return data_.back();
    }

    const T& back() const
    {
      if (empty())
      {
        throw std::logic_error("empty queue");
      }
      return data_.back();
    }

    void push(const T& value)
    {
      data_.pushBack(value);
    }

    void push(T&& value)
    {
      data_.pushBack(std::move(value));
    }

    void pop()
    {
      if (empty())
      {
        throw std::logic_error("empty queue");
      }
      data_.popFront();
    }

    T drop()
    {
      if (empty())
      {
        throw std::logic_error("empty queue");
      }
      T value = std::move(data_.front());
      data_.popFront();
      return value;
    }

    void swap(Queue& other) noexcept
    {
      data_.swap(other.data_);
    }

  private:
    List< T > data_;
  };
}

#endif
