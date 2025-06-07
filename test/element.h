#pragma once

#include <set>

namespace ct_test {

struct Element {
  struct NoNewInstancesGuard;

  Element() = delete;
  Element(int data);
  Element(const Element& other);
  Element(Element&& other) noexcept;
  ~Element();

  Element& operator=(const Element& c);
  Element& operator=(Element&& c) noexcept;
  operator int() const;

  friend bool operator==(const Element& a, const Element& b);
  friend bool operator!=(const Element& a, const Element& b);
  friend bool operator<(const Element& a, const Element& b);
  friend bool operator<=(const Element& a, const Element& b);
  friend bool operator>(const Element& a, const Element& b);
  friend bool operator>=(const Element& a, const Element& b);

  friend bool operator==(const Element& a, int b);
  friend bool operator!=(const Element& a, int b);
  friend bool operator<(const Element& a, int b);
  friend bool operator<=(const Element& a, int b);
  friend bool operator>(const Element& a, int b);
  friend bool operator>=(const Element& a, int b);

  friend bool operator==(int a, const Element& b);
  friend bool operator!=(int a, const Element& b);
  friend bool operator<(int a, const Element& b);
  friend bool operator<=(int a, const Element& b);
  friend bool operator>(int a, const Element& b);
  friend bool operator>=(int a, const Element& b);

private:
  void add_instance();
  void delete_instance();
  void assert_exists() const;

private:
  int data;

  static std::set<const Element*> instances;
};

struct Element::NoNewInstancesGuard {
  NoNewInstancesGuard();

  NoNewInstancesGuard(const NoNewInstancesGuard&) = delete;
  NoNewInstancesGuard& operator=(const NoNewInstancesGuard&) = delete;

  ~NoNewInstancesGuard();

  void expect_no_instances();

private:
  std::set<const Element*> old_instances;
};

} // namespace ct_test
