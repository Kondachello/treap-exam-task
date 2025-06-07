#include "element.h"
#include "fault-injection.h"
#include "test-utils.h"
#include "treap.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>

#include <iterator>
#include <random>
#include <type_traits>

namespace ct_test {

static_assert(
    !std::is_constructible_v<Container::Iterator, std::nullptr_t>,
    "Iterator should not be constructible from nullptr"
);
static_assert(
    !std::is_constructible_v<Container::ConstIterator, std::nullptr_t>,
    "ConstIterator should not be constructible from nullptr"
);
static_assert(
    !std::is_constructible_v<Container::ReverseIterator, std::nullptr_t>,
    "ReverseIterator should not be constructible from nullptr"
);
static_assert(
    !std::is_constructible_v<Container::ConstReverseIterator, std::nullptr_t>,
    "ConstReverseIterator should not be constructible from nullptr"
);

namespace {

class CorrectnessTest : public BaseTest {};

class ExceptionSafetyTest : public BaseTest {};

class PerformanceTest : public BaseTest {};

class RandomTest : public BaseTest {};

[[maybe_unused]] void magic([[maybe_unused]] Element& c) {
  c = 42;
}

void magic(const Element&) {}

} // namespace

TEST_CASE_METHOD(CorrectnessTest, "Default constructor") {
  Container c;
  expect_empty(c);
  instances_guard.expect_no_instances();
}

TEST_CASE_METHOD(CorrectnessTest, "Insert single element as lvalue") {
  Container c;
  Element val = 42;
  c.insert(val);
  expect_eq(c, {42});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert single element as rvalue") {
  Container c;
  c.insert(42);
  expect_eq(c, {42});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert ascending") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});
  expect_eq(c, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert descending") {
  Container c;
  mass_insert(c, {4, 3, 2, 1});
  expect_eq(c, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert shuffled #1") {
  Container c;
  mass_insert(c, {2, 1, 3, 4});
  expect_eq(c, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert shuffled #2") {
  Container c;
  mass_insert(c, {4, 2, 1, 5, 3});
  expect_eq(c, {1, 2, 3, 4, 5});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert shuffled #3") {
  Container c;
  mass_insert(c, {2, 1, 5, 3, 4});
  expect_eq(c, {1, 2, 3, 4, 5});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert same value twice") {
  Container c;
  c.insert(42);
  c.insert(42);
  expect_eq(c, {42});
}

TEST_CASE_METHOD(CorrectnessTest, "Insert many duplicates") {
  Container c;
  mass_insert(c, {8, 4, 2, 4, 4, 4, 8});
  expect_eq(c, {2, 4, 8});
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator validity after insert #1") {
  Container s;
  Container::Iterator i = s.end();

  s.insert(42);
  --i;
  REQUIRE(*i == 42);
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator validity after insert #2") {
  Container c;
  mass_insert(c, {8, 2, 5, 10, 3, 1, 9});

  Container::Iterator i = c.find(5);
  Container::Iterator j = c.find(8);

  c.insert(7);
  REQUIRE(*i == 5);
  REQUIRE(*j == 8);
  REQUIRE(*std::next(i) == 7);
  REQUIRE(*std::prev(j) == 7);
}

TEST_CASE_METHOD(CorrectnessTest, "Insert return value") {
  Container c;
  mass_insert(c, {8, 2, 5, 10, 3, 1, 9});

  auto [it, ins] = c.insert(7);
  REQUIRE(ins);
  REQUIRE(*it == 7);
  REQUIRE(*std::prev(it) == 5);
  REQUIRE(*std::next(it) == 8);
}

TEST_CASE_METHOD(CorrectnessTest, "Insert duplicate return value") {
  Container c;
  mass_insert(c, {8, 2, 5, 10, 7, 3, 1, 9});

  auto [it, ins] = c.insert(7);
  REQUIRE_FALSE(ins);
  REQUIRE(*it == 7);
  REQUIRE(*std::prev(it) == 5);
  REQUIRE(*std::next(it) == 8);
}

TEST_CASE_METHOD(CorrectnessTest, "Reinsert after erase") {
  Container c;
  mass_insert(c, {6, 2, 3, 1, 9, 8});
  c.erase(c.find(6));
  c.insert(6);
  expect_eq(c, {1, 2, 3, 6, 8, 9});
}

TEST_CASE_METHOD(CorrectnessTest, "Copy constructor from ascending") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});

  Container c2 = c;
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Copy constructor from descending") {
  Container c;
  mass_insert(c, {4, 3, 2, 1});

  Container c2 = c;
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Copy constructor from shuffled") {
  Container c;
  mass_insert(c, {8, 4, 2, 10, 5});

  Container c2 = c;
  expect_eq(c2, {2, 4, 5, 8, 10});
}

TEST_CASE_METHOD(CorrectnessTest, "Copy constructor from empty") {
  Container c;
  Container c2 = c;
  expect_empty(c2);
}

TEST_CASE_METHOD(CorrectnessTest, "Copy-assignment") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});

  Container c2;
  mass_insert(c2, {5, 6, 7, 8});

  c2 = c;
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Move-assignment") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});

  Container c2;
  mass_insert(c2, {5, 6, 7, 8});

  c2 = std::move(c);
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Copy-assignment from empty") {
  Container c;

  Container c2;
  mass_insert(c2, {1, 2, 3, 4});

  c2 = c;
  expect_empty(c2);
}

TEST_CASE_METHOD(CorrectnessTest, "Move-assignment from empty") {
  Container c;

  Container c2;
  mass_insert(c2, {1, 2, 3, 4});

  c2 = std::move(c);
  expect_empty(c2);
}

TEST_CASE_METHOD(CorrectnessTest, "Copy-assignment from self") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});

  c = c;
  expect_eq(c, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Copy-assignment from empty self") {
  Container c;
  c = c;
  expect_empty(c);
}

TEST_CASE_METHOD(CorrectnessTest, "Swap") {
  Container c1, c2;
  mass_insert(c1, {1, 2, 3, 4});
  mass_insert(c2, {5, 6, 7, 8, 9});

  swap(c1, c2);
  expect_eq(c1, {5, 6, 7, 8, 9});
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Swap with self") {
  Container c1;
  mass_insert(c1, {1, 2, 3, 4});

  swap(c1, c1);
  expect_eq(c1, {1, 2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Swap with empty") {
  Container c1, c2;
  mass_insert(c1, {1, 2, 3, 4});

  swap(c1, c2);
  expect_empty(c1);
  expect_eq(c2, {1, 2, 3, 4});

  swap(c1, c2);
  expect_eq(c1, {1, 2, 3, 4});
  expect_empty(c2);
}

TEST_CASE_METHOD(CorrectnessTest, "Swap two empty") {
  Container c1, c2;
  swap(c1, c2);
  expect_empty(c1);
  expect_empty(c2);
}

TEST_CASE_METHOD(CorrectnessTest, "Swap with empty self") {
  Container c1;
  swap(c1, c1);
  expect_empty(c1);
}

TEST_CASE_METHOD(CorrectnessTest, "Swap iterators") {
  Container c1, c2;
  mass_insert(c1, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  c2.insert(11);

  Container::ConstIterator c1_begin = c1.begin();
  Container::ConstIterator c1_end = c1.end();

  Container::ConstIterator c2_begin = c2.begin();
  Container::ConstIterator c2_end = c2.end();

  swap(c1, c2);

  REQUIRE(c1.end() == c1_end);
  REQUIRE(c2.end() == c2_end);

  REQUIRE(*c1_begin++ == 1);
  REQUIRE(*c1_begin++ == 2);
  REQUIRE(*c1_begin++ == 3);

  std::advance(c1_begin, 7);
  REQUIRE(c1_begin == c2_end);

  REQUIRE(*c2_begin++ == 11);
  REQUIRE(c2_begin == c1_end);
}

TEST_CASE_METHOD(CorrectnessTest, "empty()") {
  Container c;
  expect_empty(c);

  c.insert(1);
  REQUIRE_FALSE(c.empty());
  REQUIRE(c.size() != 0);
  REQUIRE(c.begin() != c.end());

  c.erase(1);
  expect_empty(c);
}

TEST_CASE_METHOD(CorrectnessTest, "size()") {
  Container c;
  REQUIRE(c.size() == 0);
  c.insert(1);
  REQUIRE(c.size() == 1);
  c.insert(2);
  REQUIRE(c.size() == 2);
  c.insert(2);
  REQUIRE(c.size() == 2);
  c.erase(1);
  REQUIRE(c.size() == 1);
  c.erase(1);
  REQUIRE(c.size() == 1);
  c.erase(2);
  REQUIRE(c.size() == 0);
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator conversions") {
  Container c;
  Container::ConstIterator i1 = c.begin();
  Container::Iterator i2 = c.end();

  REQUIRE(i1 == i1);
  REQUIRE(i1 == i2);
  REQUIRE(i2 == i1);
  REQUIRE(i2 == i2);
  REQUIRE_FALSE(i1 != i1);
  REQUIRE_FALSE(i1 != i2);
  REQUIRE_FALSE(i2 != i1);
  REQUIRE_FALSE(i2 != i2);

  REQUIRE(std::as_const(i1) == i1);
  REQUIRE(std::as_const(i1) == i2);
  REQUIRE(std::as_const(i2) == i1);
  REQUIRE(std::as_const(i2) == i2);
  REQUIRE_FALSE(std::as_const(i1) != i1);
  REQUIRE_FALSE(std::as_const(i1) != i2);
  REQUIRE_FALSE(std::as_const(i2) != i1);
  REQUIRE_FALSE(std::as_const(i2) != i2);

  REQUIRE(i1 == std::as_const(i1));
  REQUIRE(i1 == std::as_const(i2));
  REQUIRE(i2 == std::as_const(i1));
  REQUIRE(i2 == std::as_const(i2));
  REQUIRE_FALSE(i1 != std::as_const(i1));
  REQUIRE_FALSE(i1 != std::as_const(i2));
  REQUIRE_FALSE(i2 != std::as_const(i1));
  REQUIRE_FALSE(i2 != std::as_const(i2));

  REQUIRE(std::as_const(i1) == std::as_const(i1));
  REQUIRE(std::as_const(i1) == std::as_const(i2));
  REQUIRE(std::as_const(i2) == std::as_const(i1));
  REQUIRE(std::as_const(i2) == std::as_const(i2));
  REQUIRE_FALSE(std::as_const(i1) != std::as_const(i1));
  REQUIRE_FALSE(std::as_const(i1) != std::as_const(i2));
  REQUIRE_FALSE(std::as_const(i2) != std::as_const(i1));
  REQUIRE_FALSE(std::as_const(i2) != std::as_const(i2));
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator increment #1") {
  Container c;
  mass_insert(c, {5, 3, 8, 1, 2, 6, 7, 10});

  Container::Iterator i = c.begin();
  REQUIRE(*i == 1);
  REQUIRE(*++i == 2);
  REQUIRE(*++i == 3);
  REQUIRE(*++i == 5);
  REQUIRE(*++i == 6);
  REQUIRE(*++i == 7);
  REQUIRE(*++i == 8);
  REQUIRE(*++i == 10);
  REQUIRE(++i == c.end());
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator increment #2") {
  Container c;
  mass_insert(c, {5, 2, 10, 9, 12, 7});

  Container::Iterator i = c.begin();
  REQUIRE(*i == 2);
  REQUIRE(*++i == 5);
  REQUIRE(*++i == 7);
  REQUIRE(*++i == 9);
  REQUIRE(*++i == 10);
  REQUIRE(*++i == 12);
  REQUIRE(++i == c.end());
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator increment #3") {
  Container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  Container::Iterator i = std::next(c.begin(), 3);
  ++(++i);
  REQUIRE(*i == 6);
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator decrement #1") {
  Container s;
  mass_insert(s, {5, 3, 8, 1, 2, 6, 7, 10});

  Container::Iterator i = s.end();
  REQUIRE(*--i == 10);
  REQUIRE(*--i == 8);
  REQUIRE(*--i == 7);
  REQUIRE(*--i == 6);
  REQUIRE(*--i == 5);
  REQUIRE(*--i == 3);
  REQUIRE(*--i == 2);
  REQUIRE(*--i == 1);
  REQUIRE(i == s.begin());
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator decrement #2") {
  Container s;
  mass_insert(s, {5, 2, 10, 9, 12, 7});

  Container::Iterator i = s.end();
  REQUIRE(*--i == 12);
  REQUIRE(*--i == 10);
  REQUIRE(*--i == 9);
  REQUIRE(*--i == 7);
  REQUIRE(*--i == 5);
  REQUIRE(*--i == 2);
  REQUIRE(i == s.begin());
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator decrement #3") {
  Container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  Container::Iterator i = std::next(c.begin(), 3);
  --(--i);
  REQUIRE(*i == 2);
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator postfix operations") {
  Container c;
  mass_insert(c, {1, 2, 3});

  Container::Iterator i = c.begin();
  REQUIRE(*i == 1);
  Container::Iterator j = i++;
  REQUIRE(*i == 2);
  REQUIRE(*j == 1);
  j = i++;
  REQUIRE(*i == 3);
  REQUIRE(*j == 2);
  j = i++;
  REQUIRE(i == c.end());
  REQUIRE(*j == 3);
  j = i--;
  REQUIRE(*i == 3);
  REQUIRE(j == c.end());
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator dereference") {
  Container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  Container::Iterator i = c.find(4);
  REQUIRE(*i == 4);
  magic(*i);
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  Container::ConstIterator j = c.find(3);
  REQUIRE(*j == 3);
  magic(*j);
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_CASE_METHOD(CorrectnessTest, "Constant iterator dereference") {
  Container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  const Container::Iterator i = c.find(4);
  REQUIRE(*i == 4);
  magic(*i);
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  const Container::ConstIterator j = c.find(3);
  REQUIRE(*j == 3);
  magic(*j);
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator member-access") {
  Container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  Container::Iterator i = c.find(4);
  REQUIRE(*i == 4);
  magic(*i.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  Container::ConstIterator j = c.find(3);
  REQUIRE(*j == 3);
  magic(*j.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_CASE_METHOD(CorrectnessTest, "Constant iterator member-access") {
  Container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  const Container::Iterator i = c.find(4);
  REQUIRE(*i == 4);
  magic(*i.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  const Container::ConstIterator j = c.find(3);
  REQUIRE(*j == 3);
  magic(*j.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator default constructor") {
  Container::Iterator i;
  Container::ConstIterator j;
  Container s;
  mass_insert(s, {4, 1, 8, 6, 3, 2, 6});

  i = s.begin();
  j = s.begin();
  REQUIRE(*i == 1);
  REQUIRE(*j == 1);
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator swap") {
  Container c1;
  mass_insert(c1, {1, 2, 3});

  Container c2;
  mass_insert(c2, {4, 5, 6});

  Container::Iterator i = c1.find(2);
  Container::Iterator j = c2.find(5);

  {
    using std::swap;
    swap(i, j);
  }

  c1.erase(j);
  c2.erase(i);
  expect_eq(c1, {1, 3});
  expect_eq(c2, {4, 6});
}

TEST_CASE_METHOD(CorrectnessTest, "ReverseIterator") {
  Container c;
  mass_insert(c, {3, 1, 2, 4});
  expect_eq(ReverseView(c), {4, 3, 2, 1});

  REQUIRE(*c.rbegin() == 4);
  REQUIRE(*std::next(c.rbegin()) == 3);
  REQUIRE(*std::prev(c.rend()) == 1);
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator constness") {
  Container c;
  mass_insert(c, {1, 2, 3});

  magic(*std::as_const(c).begin());
  magic(*std::prev(std::as_const(c).end()));
  expect_eq(c, {1, 2, 3});
}

TEST_CASE_METHOD(CorrectnessTest, "ReverseIterator constness") {
  Container c;
  mass_insert(c, {1, 2, 3});

  magic(*std::as_const(c).rbegin());
  magic(*std::prev(std::as_const(c).rend()));
  expect_eq(c, {1, 2, 3});
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator::value_type") {
  Container c;
  mass_insert(c, {1, 2, 3});

  Container::Iterator::value_type e = *c.begin();
  e = 42;
  expect_eq(c, {1, 2, 3});
}

TEST_CASE_METHOD(CorrectnessTest, "ConstIterator::value_type") {
  Container c;
  mass_insert(c, {1, 2, 3});

  Container::ConstIterator::value_type e = *std::as_const(c).begin();
  e = 42;
  expect_eq(c, {1, 2, 3});
}

TEST_CASE_METHOD(CorrectnessTest, "clear()") {
  Container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  c.clear();
  expect_empty(c);

  mass_insert(c, {5, 6, 7, 8});
  expect_eq(c, {5, 6, 7, 8});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase iterator - First") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});

  c.erase(c.begin());
  expect_eq(c, {2, 3, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase iterator - Middle") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});

  c.erase(std::next(c.begin(), 2));
  expect_eq(c, {1, 2, 4});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase iterator - Before last") {
  Container c;
  mass_insert(c, {6, 1, 4, 3, 2, 5});

  c.erase(std::next(c.begin(), 4));
  expect_eq(c, {1, 2, 3, 4, 6});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase iterator - Last") {
  Container c;
  mass_insert(c, {1, 2, 3, 4});

  c.erase(std::prev(c.end()));
  expect_eq(c, {1, 2, 3});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #1") {
  Container c;
  mass_insert(c, {5, 3, 8, 1, 2, 7, 9, 10, 11, 12});

  c.erase(8);
  expect_eq(c, {1, 2, 3, 5, 7, 9, 10, 11, 12});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #2") {
  Container c;
  mass_insert(c, {5, 3, 17, 15, 20, 19, 18});

  c.erase(17);
  expect_eq(c, {3, 5, 15, 18, 19, 20});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #3") {
  Container c;
  mass_insert(c, {10, 5, 15, 14, 13});

  c.erase(15);
  expect_eq(c, {5, 10, 13, 14});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #4") {
  Container c;
  mass_insert(c, {10, 5, 15, 3, 4});

  c.erase(5);
  expect_eq(c, {3, 4, 10, 15});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #5") {
  Container c;
  mass_insert(c, {5, 2, 10, 6, 14, 7, 8});

  c.erase(5);
  expect_eq(c, {2, 6, 7, 8, 10, 14});
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #6") {
  Container c;
  mass_insert(c, {7, 3, 2, 6, 10, 9});

  c.erase(3);
  c.erase(6);
  c.erase(7);
  c.erase(10);
  c.erase(2);
  c.erase(9);
  expect_empty(c);
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #7") {
  Container c;
  mass_insert(c, {5, 3, 8});

  c.erase(5);
  expect_eq(c, {3, 8});
  REQUIRE_FALSE(c.empty());
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value #8") {
  Container c;
  mass_insert(c, {5, 3});

  c.erase(5);
  expect_eq(c, {3});
  REQUIRE_FALSE(c.empty());
}

TEST_CASE_METHOD(CorrectnessTest, "Erase iterator - Return value #1") {
  Container c;
  mass_insert(c, {5, 2, 1, 3, 4});

  Container::Iterator i = c.erase(c.find(3));
  REQUIRE(*i == 4);
  i = c.erase(i);
  REQUIRE(*i == 5);
}

TEST_CASE_METHOD(CorrectnessTest, "Erase iterator - Return value #2") {
  Container c;
  mass_insert(c, {1, 4, 3, 2, 5});

  Container::Iterator i = c.erase(c.find(3));
  REQUIRE(*i == 4);
  i = c.erase(i);
  REQUIRE(*i == 5);
}

TEST_CASE_METHOD(CorrectnessTest, "Erase iterator - Return value #3") {
  Container c;
  mass_insert(c, {7, 4, 10, 1, 8, 7, 12});

  Container::Iterator i = c.erase(c.find(7));
  REQUIRE(*i == 8);
  i = c.erase(i);
  REQUIRE(*i == 10);
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value - Return value #1") {
  Container c;
  mass_insert(c, {7, 4, 10, 1, 8, 7, 12});

  size_t i = c.erase(7);
  REQUIRE(i == 1);
}

TEST_CASE_METHOD(CorrectnessTest, "Erase value - Return value #2") {
  Container c;
  mass_insert(c, {7, 4, 10, 1, 8, 7, 12});

  size_t i = c.erase(6);
  REQUIRE(i == 0);
}

TEST_CASE_METHOD(CorrectnessTest, "Iterator validity after erase") {
  Container c;
  mass_insert(c, {8, 2, 6, 10, 3, 1, 9, 7});

  Container::Iterator i = c.find(8);
  Container::Iterator prev = std::prev(i);
  Container::Iterator next = std::next(i);

  c.erase(i);
  REQUIRE(*prev == 7);
  REQUIRE(*next == 9);
  REQUIRE(std::next(prev) == next);
  REQUIRE(std::prev(next) == prev);
}

TEST_CASE_METHOD(CorrectnessTest, "Find in empty") {
  Container c;

  REQUIRE(c.find(0) == c.end());
  REQUIRE(c.find(5) == c.end());
  REQUIRE(c.find(42) == c.end());
}

TEST_CASE_METHOD(CorrectnessTest, "Finds") {
  Container c;
  mass_insert(c, {8, 3, 5, 4, 3, 1, 8, 8, 10, 9});

  REQUIRE(c.find(0) == c.end());
  REQUIRE(c.find(1) == c.begin());
  REQUIRE(c.find(2) == c.end());
  REQUIRE(c.find(3) == std::next(c.begin(), 1));
  REQUIRE(c.find(4) == std::next(c.begin(), 2));
  REQUIRE(c.find(5) == std::next(c.begin(), 3));
  REQUIRE(c.find(6) == c.end());
  REQUIRE(c.find(7) == c.end());
  REQUIRE(c.find(8) == std::next(c.begin(), 4));
  REQUIRE(c.find(9) == std::next(c.begin(), 5));
  REQUIRE(c.find(10) == std::next(c.begin(), 6));
  REQUIRE(c.find(11) == c.end());
}

TEST_CASE_METHOD(CorrectnessTest, "Lower bound in empty") {
  Container c;
  REQUIRE(c.lower_bound(5) == c.end());
}

TEST_CASE_METHOD(CorrectnessTest, "Lower bounds") {
  Container c;
  mass_insert(c, {8, 3, 5, 4, 3, 1, 8, 8, 10, 9});

  REQUIRE(c.lower_bound(0) == c.begin());
  REQUIRE(c.lower_bound(1) == c.begin());
  REQUIRE(c.lower_bound(2) == std::next(c.begin()));
  REQUIRE(c.lower_bound(3) == std::next(c.begin()));
  REQUIRE(c.lower_bound(4) == std::next(c.begin(), 2));
  REQUIRE(c.lower_bound(5) == std::next(c.begin(), 3));
  REQUIRE(c.lower_bound(6) == std::next(c.begin(), 4));
  REQUIRE(c.lower_bound(7) == std::next(c.begin(), 4));
  REQUIRE(c.lower_bound(8) == std::next(c.begin(), 4));
  REQUIRE(c.lower_bound(9) == std::next(c.begin(), 5));
  REQUIRE(c.lower_bound(10) == std::next(c.begin(), 6));
  REQUIRE(c.lower_bound(11) == std::next(c.begin(), 7));
}

TEST_CASE_METHOD(CorrectnessTest, "Upper bound in empty") {
  Container c;
  REQUIRE(c.upper_bound(5) == c.end());
}

TEST_CASE_METHOD(CorrectnessTest, "Upper bounds") {
  Container c;
  mass_insert(c, {8, 3, 5, 4, 3, 1, 8, 8, 10, 9});

  REQUIRE(c.upper_bound(0) == c.begin());
  REQUIRE(c.upper_bound(1) == std::next(c.begin()));
  REQUIRE(c.upper_bound(2) == std::next(c.begin()));
  REQUIRE(c.upper_bound(3) == std::next(c.begin(), 2));
  REQUIRE(c.upper_bound(4) == std::next(c.begin(), 3));
  REQUIRE(c.upper_bound(5) == std::next(c.begin(), 4));
  REQUIRE(c.upper_bound(6) == std::next(c.begin(), 4));
  REQUIRE(c.upper_bound(7) == std::next(c.begin(), 4));
  REQUIRE(c.upper_bound(8) == std::next(c.begin(), 5));
  REQUIRE(c.upper_bound(9) == std::next(c.begin(), 6));
  REQUIRE(c.upper_bound(10) == std::next(c.begin(), 7));
  REQUIRE(c.upper_bound(11) == std::next(c.begin(), 7));
}

TEST_CASE_METHOD(ExceptionSafetyTest, "Default constructor does not throw") {
  faulty_run([] {
    try {
      Container c;
    } catch (...) {
      FaultInjectionDisable dg;
      FAIL("default constructor should not throw");
      throw;
    }
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "Copy constructor is exception-safe") {
  faulty_run([] {
    Container c;
    mass_insert(c, {3, 2, 4, 1});

    StrongExceptionSafetyGuard sg(c);
    Container c2 = c;
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "Move constructor does not throw") {
  faulty_run([] {
    Container c;
    mass_insert(c, {3, 2, 4, 1});

    try {
      Container c2 = std::move(c);
    } catch (...) {
      FaultInjectionDisable dg;
      FAIL("move constructor should not throw");
      throw;
    }
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "clear() does not throw") {
  faulty_run([] {
    Container c;
    mass_insert(c, {3, 2, 4, 1});
    try {
      c.clear();
    } catch (...) {
      FaultInjectionDisable dg;
      FAIL("clear() should not throw");
      throw;
    }
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "Copy-assignment is exception-safe") {
  faulty_run([] {
    Container c;
    mass_insert(c, {3, 2, 4, 1});

    Container c2;
    mass_insert(c2, {8, 7, 2, 14});

    StrongExceptionSafetyGuard sg(c);
    c = c2;
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "Move-assignment does not throw") {
  faulty_run([] {
    Container c;
    mass_insert(c, {3, 2, 4, 1});

    Container c2;
    mass_insert(c2, {8, 7, 2, 14});

    try {
      c = std::move(c2);
    } catch (...) {
      FaultInjectionDisable dg;
      FAIL("move assignment should not throw");
      throw;
    }
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "insert(const T&) is exception-safe") {
  faulty_run([] {
    Container c;
    mass_insert(c, {5, 2, 4, 1});

    StrongExceptionSafetyGuard sg(c);
    Element foo = 4;
    c.insert(foo);
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "insert(T&&) is exception-safe") {
  faulty_run([] {
    Container c;
    mass_insert(c, {3, 2, 5, 1});

    StrongExceptionSafetyGuard sg(c);
    c.insert(4);
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "erase(it) is exception-safe #1") {
  faulty_run([] {
    Container c;
    mass_insert(c, {6, 3, 8, 2, 5, 7, 10});

    StrongExceptionSafetyGuard sg(c);
    Element val = 6;
    c.erase(c.find(val));
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "erase(it) is exception-safe #2") {
  faulty_run([] {
    Container c;
    mass_insert(c, {6, 3, 8, 2, 5, 7, 10});

    StrongExceptionSafetyGuard sg(c);
    Element val = 5;
    c.erase(c.find(val));
  });
}

TEST_CASE_METHOD(ExceptionSafetyTest, "Insert is exception-safe with throwing random generator") {
  class ThrowingRng : public std::mt19937 {
    std::mt19937::result_type operator()() {
      fault_injection_point();
      return std::mt19937::operator()();
    }
  };

  faulty_run([] {
    ThrowingRng rng;
    Container c(rng);
    FaultInjectionDisable dg;
    mass_insert(c, {3, 2, 4, 1});

    StrongExceptionSafetyGuard sg(c);
    dg.reset();

    Element value = 42;
    c.insert(value);
  });
}

TEST_CASE_METHOD(PerformanceTest, "size() is fast") {
  constexpr size_t N = 100'000;
  constexpr size_t K = 1'000'000;

  Container c;
  mass_insert_balanced(c, N);

  for (size_t i = 0; i < K; ++i) {
    REQUIRE(N == c.size());
  }
}

TEST_CASE_METHOD(PerformanceTest, "Iteration is fast") {
  constexpr size_t N = 100'000;
  constexpr size_t K = 5;

  Container c;
  mass_insert_balanced(c, N);

  for (size_t i = 0; i < K; ++i) {
    for (Container::Iterator j = c.begin(); j != c.end(); ++j) {
      [[maybe_unused]] volatile int escape = *j;
    }
  }
}

TEST_CASE_METHOD(PerformanceTest, "lower_bound(const T&) is fast") {
  constexpr size_t N = 100'000;
  constexpr size_t K = 200'000;

  Container c;
  mass_insert_balanced(c, N);

  for (size_t i = 0; i < K; ++i) {
    constexpr int n = N;
    REQUIRE(c.begin() == c.lower_bound(1));
    REQUIRE(std::prev(c.end()) == c.lower_bound(n));
  }
}

TEST_CASE_METHOD(PerformanceTest, "swap(Treap&, Treap&) is fast") {
  constexpr size_t N = 100'000;
  constexpr size_t K = 1'000'000;

  Container c1;
  Container c2;

  mass_insert_balanced(c1, N);
  mass_insert_balanced(c2, N, -1);

  for (size_t i = 0; i < K; ++i) {
    swap(c1, c2);
  }
}

namespace {

struct RandomTestConfig {
  std::mt19937::result_type seed = std::mt19937::default_seed;
  std::uniform_int_distribution<int> value_dist;
  size_t iterations{};
  double p_insert{};
  double p_erase{};
  double p_compare = .1;
};

void run_random_test(RandomTestConfig cfg) {
  std::mt19937 rng(cfg.seed);

  std::uniform_real_distribution real_dist;

  std::set<int> std_set;
  Container treap;

  for (size_t i = 0; i < cfg.iterations; ++i) {
    double op = real_dist(rng);
    int e = cfg.value_dist(rng);

    if (op < cfg.p_insert) {
      auto [std_it, std_ins] = std_set.insert(e);
      auto [treap_it, treap_ins] = treap.insert(e);
      REQUIRE(treap_ins == std_ins);
      REQUIRE(*treap_it == *std_it);
    } else if (op < cfg.p_insert + cfg.p_erase) {
      auto std_erase_result = std_set.erase(e);
      auto treap_erase_result = treap.erase(e);
      REQUIRE(std_erase_result == treap_erase_result);
    } else {
      auto std_it = std_set.find(e);
      auto treap_it = treap.find(e);
      REQUIRE((treap_it == treap.end()) == (std_it == std_set.end()));
    }

    REQUIRE(treap.empty() == std_set.empty());
    REQUIRE(treap.size() == std_set.size());

    if (real_dist(rng) < cfg.p_compare) {
      REQUIRE(std::equal(treap.begin(), treap.end(), std_set.begin()));
    }
  }
}

} // namespace

TEST_CASE_METHOD(RandomTest, "Random insertions (scattered)") {
  RandomTestConfig cfg;
  cfg.seed = 1337;
  cfg.value_dist = std::uniform_int_distribution(1, 10'000);
  cfg.iterations = 10'000;
  cfg.p_insert = .5;
  cfg.p_erase = 0;

  run_random_test(cfg);
}

TEST_CASE_METHOD(RandomTest, "Random insertions (dense)") {
  RandomTestConfig cfg;
  cfg.seed = 1338;
  cfg.value_dist = std::uniform_int_distribution(1, 500);
  cfg.iterations = 100'000;
  cfg.p_insert = .5;
  cfg.p_erase = 0;

  run_random_test(cfg);
}

TEST_CASE_METHOD(RandomTest, "Random erases (scattered, many insertions)") {
  RandomTestConfig cfg;
  cfg.seed = 1339;
  cfg.value_dist = std::uniform_int_distribution(1, 10'000);
  cfg.iterations = 10'000;
  cfg.p_insert = .4;
  cfg.p_erase = .2;

  run_random_test(cfg);
}

TEST_CASE_METHOD(RandomTest, "Random erases (dense, many insertions)") {
  RandomTestConfig cfg;
  cfg.seed = 1340;
  cfg.value_dist = std::uniform_int_distribution(1, 500);
  cfg.iterations = 100'000;
  cfg.p_insert = .4;
  cfg.p_erase = .2;

  run_random_test(cfg);
}

TEST_CASE_METHOD(RandomTest, "Random erases (scattered, few insertions)") {
  RandomTestConfig cfg;
  cfg.seed = 1341;
  cfg.value_dist = std::uniform_int_distribution(1, 10'000);
  cfg.iterations = 10'000;
  cfg.p_insert = .01;
  cfg.p_erase = .7;

  run_random_test(cfg);
}

TEST_CASE_METHOD(RandomTest, "Random erases (dense, few insertions)") {
  RandomTestConfig cfg;
  cfg.seed = 1342;
  cfg.value_dist = std::uniform_int_distribution(1, 500);
  cfg.iterations = 100'000;
  cfg.p_insert = .01;
  cfg.p_erase = .7;

  run_random_test(cfg);
}

} // namespace ct_test
