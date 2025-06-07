#include "element.h"

#include "fault-injection.h"

#include <catch2/catch_all.hpp>

ct_test::Element::Element(int data)
    : data(data) {
  fault_injection_point();
  add_instance();
}

ct_test::Element::Element(const Element& other)
    : data(other.data) {
  other.assert_exists();
  fault_injection_point();
  add_instance();
}

ct_test::Element::Element(ct_test::Element&& other) noexcept {
  other.assert_exists();
  FaultInjectionDisable dg;
  add_instance();
  data = std::exchange(other.data, -1);
}

ct_test::Element::~Element() {
  delete_instance();
}

ct_test::Element& ct_test::Element::operator=(const Element& c) {
  assert_exists();
  c.assert_exists();
  fault_injection_point();
  data = c.data;
  return *this;
}

ct_test::Element& ct_test::Element::operator=(Element&& c) noexcept {
  assert_exists();
  c.assert_exists();
  FaultInjectionDisable dg;
  data = std::exchange(c.data, -1);
  return *this;
}

ct_test::Element::operator int() const {
  assert_exists();
  fault_injection_point();

  return data;
}

namespace ct_test {

bool operator==(const Element& a, const Element& b) {
  a.assert_exists();
  b.assert_exists();
  fault_injection_point();
  return a.data == b.data;
}

bool operator!=(const Element& a, const Element& b) {
  a.assert_exists();
  b.assert_exists();
  fault_injection_point();
  return a.data != b.data;
}

bool operator<(const Element& a, const Element& b) {
  a.assert_exists();
  b.assert_exists();
  fault_injection_point();
  return a.data < b.data;
}

bool operator<=(const Element& a, const Element& b) {
  a.assert_exists();
  b.assert_exists();
  fault_injection_point();
  return a.data <= b.data;
}

bool operator>(const Element& a, const Element& b) {
  a.assert_exists();
  b.assert_exists();
  fault_injection_point();
  return a.data > b.data;
}

bool operator>=(const Element& a, const Element& b) {
  a.assert_exists();
  b.assert_exists();
  fault_injection_point();
  return a.data >= b.data;
}

bool operator==(const Element& a, int b) {
  a.assert_exists();
  fault_injection_point();
  return a.data == b;
}

bool operator!=(const Element& a, int b) {
  a.assert_exists();
  fault_injection_point();
  return a.data != b;
}

bool operator<(const Element& a, int b) {
  a.assert_exists();
  fault_injection_point();
  return a.data < b;
}

bool operator<=(const Element& a, int b) {
  a.assert_exists();
  fault_injection_point();
  return a.data <= b;
}

bool operator>(const Element& a, int b) {
  a.assert_exists();
  fault_injection_point();
  return a.data > b;
}

bool operator>=(const Element& a, int b) {
  a.assert_exists();
  fault_injection_point();
  return a.data >= b;
}

bool operator==(int a, const Element& b) {
  b.assert_exists();
  fault_injection_point();
  return a == b.data;
}

bool operator!=(int a, const Element& b) {
  b.assert_exists();
  fault_injection_point();
  return a != b.data;
}

bool operator<(int a, const Element& b) {
  b.assert_exists();
  fault_injection_point();
  return a < b.data;
}

bool operator<=(int a, const Element& b) {
  b.assert_exists();
  fault_injection_point();
  return a <= b.data;
}

bool operator>(int a, const Element& b) {
  b.assert_exists();
  fault_injection_point();
  return a > b.data;
}

bool operator>=(int a, const Element& b) {
  b.assert_exists();
  fault_injection_point();
  return a >= b.data;
}

} // namespace ct_test

void ct_test::Element::add_instance() {
  FaultInjectionDisable dg;
  auto p = instances.insert(this);
  if (!p.second) {
    FAIL_CHECK(
        "A new object is created at the address " << static_cast<void*>(this)
                                                  << " while the previous object at this address was not destroyed"
    );
  }
}

void ct_test::Element::delete_instance() {
  FaultInjectionDisable dg;
  size_t erased = instances.erase(this);
  if (erased != 1) {
    FAIL_CHECK("Attempt of destroying non-existing object at address " << static_cast<void*>(this) << '\n');
  }
}

void ct_test::Element::assert_exists() const {
  FaultInjectionDisable dg;
  bool exists = instances.find(this) != instances.end();
  if (!exists) {
    FAIL_CHECK("Accessing a non-existing object at address " << static_cast<const void*>(this));
  }
}

std::set<const ct_test::Element*> ct_test::Element::instances;

ct_test::Element::NoNewInstancesGuard::NoNewInstancesGuard()
    : old_instances(instances) {}

ct_test::Element::NoNewInstancesGuard::~NoNewInstancesGuard() {
  REQUIRE(old_instances == instances);
}

void ct_test::Element::NoNewInstancesGuard::expect_no_instances() {
  REQUIRE(old_instances == instances);
}
