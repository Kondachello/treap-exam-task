#pragma once

#include <stdexcept>

namespace ct {

inline void throwing_func() {
  throw std::logic_error("some exception");
}

} // namespace ct
