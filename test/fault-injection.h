#pragma once

#include <functional>
#include <stdexcept>

namespace ct_test {

struct InjectedFault : std::runtime_error {
  using runtime_error::runtime_error;
};

bool should_inject_fault();
void fault_injection_point();
void faulty_run(const std::function<void()>& f);
void assert_nothrow(const std::function<void()>& f);

std::size_t get_new_calls() noexcept;
std::size_t get_delete_calls() noexcept;

struct FaultInjectionDisable {
  FaultInjectionDisable();

  void reset() const;

  FaultInjectionDisable(const FaultInjectionDisable&) = delete;
  FaultInjectionDisable& operator=(const FaultInjectionDisable&) = delete;

  ~FaultInjectionDisable();

private:
  bool was_disabled;
};

} // namespace ct_test
