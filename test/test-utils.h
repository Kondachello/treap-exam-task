#pragma once

#include "element.h"
#include "fault-injection.h"
#include "treap.h"

#include <catch2/catch_all.hpp>

#include <initializer_list>
#include <ostream>
#include <sstream>

namespace ct {

template class Treap<ct_test::Element>;

} // namespace ct

namespace ct_test {

using Container = ct::Treap<Element>;

template <typename F>
decltype(auto) operator<<(std::ostream& out, const F& f)
  requires (std::is_invocable_v<F, std::ostream&>)
{
  return f(out);
}

template <typename C, typename T>
void mass_insert(C& c, std::initializer_list<T> elems) {
  for (const T& e : elems) {
    c.insert(e);
  }
}

constexpr size_t bit_floor(size_t x) {
  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  x |= (x >> 32);
  return x - (x >> 1);
}

template <typename C>
void mass_insert_balanced(C& c, size_t count, int factor = 1) {
  for (size_t i = bit_floor(count); i > 0; i /= 2) {
    for (size_t j = i; j <= count; j += i * 2) {
      c.insert(static_cast<int>(j) * factor);
    }
  }
}

template <class Actual, class Expected>
void expect_eq(const Actual& actual, const Expected& expected) {
  FaultInjectionDisable dg;

  REQUIRE(actual.size() == expected.size());

  static const auto pprint = [](const auto& expected, const auto& actual) {
    std::stringstream out;
    out << '{';

    bool add_comma = false;
    for (const auto& e : expected) {
      if (add_comma) {
        out << ", ";
      }
      out << e;
      add_comma = true;
    }

    out << "} != {";

    add_comma = false;
    for (const auto& e : actual) {
      if (add_comma) {
        out << ", ";
      }
      out << e;
      add_comma = true;
    }

    out << "}\n";
    return out.str();
  };
  INFO(pprint(expected, actual));
  REQUIRE(std::equal(expected.begin(), expected.end(), actual.begin(), actual.end()));
}

template <class Actual, class T>
void expect_eq(const Actual& actual, const std::initializer_list<T>& expected) {
  return expect_eq<Actual, std::initializer_list<T>>(actual, expected);
}

template <typename C>
void expect_empty(const C& c) {
  REQUIRE(c.empty());
  REQUIRE(c.size() == 0);
  REQUIRE(c.begin() == c.end());
}

template <class It>
class ReverseView {
public:
  template <class R>
  ReverseView(const R& r) noexcept
      : ReverseView(r.begin(), r.end(), r.size()) {}

  ReverseView(It begin, It end, size_t size) noexcept
      : base_begin(begin)
      , base_end(end)
      , base_size(size) {}

  auto begin() const noexcept {
    return std::make_reverse_iterator(base_end);
  }

  auto end() const noexcept {
    return std::make_reverse_iterator(base_begin);
  }

  size_t size() const noexcept {
    return base_size;
  }

private:
  It base_begin;
  It base_end;
  size_t base_size;
};

template <class R>
ReverseView(const R& r) -> ReverseView<decltype(r.begin())>;

template <typename C>
class StrongExceptionSafetyGuard {
public:
  explicit StrongExceptionSafetyGuard(const C& c) noexcept
      : ref(c)
      , expected((FaultInjectionDisable{}, c)) {}

  StrongExceptionSafetyGuard(const StrongExceptionSafetyGuard&) = delete;

  ~StrongExceptionSafetyGuard() {
    if (std::uncaught_exceptions() > 0) {
      expect_eq(expected, ref);
    }
  }

private:
  const C& ref;
  C expected;
};

class BaseTest {
protected:
  Element::NoNewInstancesGuard instances_guard;
};

} // namespace ct_test
