#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include "queue.hpp"
#include "stack.hpp"
#include <limits>
#include <stdexcept>
#include <string>

namespace shaykhraziev
{
  namespace detail
  {
    inline bool isOperator(const std::string& token)
    {
      return token == "+" || token == "-" || token == "*" || token == "/" || token == "%" || token == "lcm";
    }

    inline int getPriority(const std::string& token)
    {
      if (token == "lcm")
      {
        return 3;
      }
      if (token == "*" || token == "/" || token == "%")
      {
        return 2;
      }
      return 1;
    }

    inline long long parseNumber(const std::string& token)
    {
      return std::stoll(token);
    }

    inline size_t getMagnitude(long long value)
    {
      if (value >= 0)
      {
        return value;
      }
      return static_cast< size_t >(-(value + 1)) + 1ULL;
    }

    inline size_t gcdUnsigned(size_t lhs, size_t rhs)
    {
      while (rhs != 0)
      {
        size_t tmp = lhs % rhs;
        lhs = rhs;
        rhs = tmp;
      }
      return lhs;
    }

    inline long long safeAdd(long long lhs, long long rhs)
    {
      const long long maxValue = std::numeric_limits< long long >::max();
      const long long minValue = std::numeric_limits< long long >::min();
      if (rhs > 0 && lhs > maxValue - rhs)
      {
        throw std::overflow_error("overflow");
      }
      if (rhs < 0 && lhs < minValue - rhs)
      {
        throw std::overflow_error("overflow");
      }
      return lhs + rhs;
    }

    inline long long safeSub(long long lhs, long long rhs)
    {
      const long long maxValue = std::numeric_limits< long long >::max();
      const long long minValue = std::numeric_limits< long long >::min();
      if (rhs > 0 && lhs < minValue + rhs)
      {
        throw std::overflow_error("overflow");
      }
      if (rhs < 0 && lhs > maxValue + rhs)
      {
        throw std::overflow_error("overflow");
      }
      return lhs - rhs;
    }

    inline long long safeMul(long long lhs, long long rhs)
    {
      const long long maxValue = std::numeric_limits< long long >::max();
      const long long minValue = std::numeric_limits< long long >::min();

      if (lhs == 0 || rhs == 0)
      {
        return 0;
      }
      if ((lhs == -1 && rhs == minValue) || (rhs == -1 && lhs == minValue))
      {
        throw std::overflow_error("overflow");
      }

      if (lhs > 0)
      {
        if (rhs > 0)
        {
          if (lhs > maxValue / rhs)
          {
            throw std::overflow_error("overflow");
          }
        }
        else if (rhs < minValue / lhs)
        {
          throw std::overflow_error("overflow");
        }
      }
      else
      {
        if (rhs > 0)
        {
          if (lhs < minValue / rhs)
          {
            throw std::overflow_error("overflow");
          }
        }
        else if (lhs < maxValue / rhs)
        {
          throw std::overflow_error("overflow");
        }
      }

      return lhs * rhs;
    }

    inline long long safeDiv(long long lhs, long long rhs)
    {
      if (rhs == 0)
      {
        throw std::logic_error("division by zero");
      }
      if (lhs == std::numeric_limits< long long >::min() && rhs == -1)
      {
        throw std::overflow_error("overflow");
      }
      return lhs / rhs;
    }

    inline long long safeMod(long long lhs, long long rhs)
    {
      if (rhs == 0)
      {
        throw std::logic_error("division by zero");
      }
      if (lhs == std::numeric_limits< long long >::min() && rhs == -1)
      {
        return 0;
      }

      long long remainder = lhs % rhs;
      if (remainder >= 0)
      {
        return remainder;
      }

      size_t divisorMagnitude = getMagnitude(rhs);
      size_t remainderMagnitude = getMagnitude(remainder);
      return static_cast< long long >(divisorMagnitude - remainderMagnitude);
    }

    inline long long safeLcm(long long lhs, long long rhs)
    {
      if (lhs == 0 || rhs == 0)
      {
        return 0;
      }

      size_t left = getMagnitude(lhs);
      size_t right = getMagnitude(rhs);
      size_t gcd = gcdUnsigned(left, right);
      size_t reduced = left / gcd;
      size_t limit = std::numeric_limits< long long >::max();

      if (reduced > limit / right)
      {
        throw std::overflow_error("overflow");
      }

      return static_cast< long long >(reduced * right);
    }

    inline long long applyOperator(const std::string& op, long long lhs, long long rhs)
    {
      if (op == "+")
      {
        return safeAdd(lhs, rhs);
      }
      if (op == "-")
      {
        return safeSub(lhs, rhs);
      }
      if (op == "*")
      {
        return safeMul(lhs, rhs);
      }
      if (op == "/")
      {
        return safeDiv(lhs, rhs);
      }
      if (op == "%")
      {
        return safeMod(lhs, rhs);
      }
      return safeLcm(lhs, rhs);
    }

    inline Queue< std::string > tokenize(const std::string& line)
    {
      Queue< std::string > tokens;
      std::size_t start = 0;

      while (start < line.size())
      {
        std::size_t finish = line.find(' ', start);
        if (finish == std::string::npos)
        {
          tokens.push(line.substr(start));
          break;
        }
        tokens.push(line.substr(start, finish - start));
        start = finish + 1;
      }

      return tokens;
    }
  }

  inline Queue< std::string > toPostfix(const std::string& line)
  {
    Queue< std::string > infix = detail::tokenize(line);
    Queue< std::string > postfix;
    Stack< std::string > operators;

    while (!infix.empty())
    {
      std::string token = infix.drop();

      if (token == "(")
      {
        operators.push(std::move(token));
      }
      else if (token == ")")
      {
        while (operators.top() != "(")
        {
          postfix.push(operators.drop());
        }
        operators.pop();
      }
      else if (detail::isOperator(token))
      {
        while (!operators.empty() && operators.top() != "(" &&
            detail::getPriority(operators.top()) >= detail::getPriority(token))
        {
          postfix.push(operators.drop());
        }
        operators.push(std::move(token));
      }
      else
      {
        postfix.push(std::move(token));
      }
    }

    while (!operators.empty())
    {
      postfix.push(operators.drop());
    }

    return postfix;
  }

  inline long long evaluatePostfix(Queue< std::string > postfix)
  {
    Stack< long long > values;

    while (!postfix.empty())
    {
      std::string token = postfix.drop();

      if (detail::isOperator(token))
      {
        long long rhs = values.drop();
        long long lhs = values.drop();
        values.push(detail::applyOperator(token, lhs, rhs));
      }
      else
      {
        values.push(detail::parseNumber(token));
      }
    }

    return values.drop();
  }

  inline long long evaluateExpression(const std::string& line)
  {
    return evaluatePostfix(toPostfix(line));
  }
}

#endif
