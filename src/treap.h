#pragma once

#include <cstddef>
#include <iterator>
#include <random>
#include <type_traits>

namespace ct {

template <typename T, std::uniform_random_bit_generator RandGen = std::mt19937>
class Treap : RandGen {
  static_assert(!std::is_const_v<T>, "T must be non-const");
  static_assert(std::is_copy_constructible_v<T>, "T must have a copy constructor");
  static_assert(std::is_nothrow_move_constructible_v<T>, "T must have a non-throwing move constructor");
  static_assert(
      std::is_nothrow_copy_constructible_v<RandGen>,
      "Random Generator must have a non-throwing copy constructor"
  );
  static_assert(
      std::is_nothrow_move_constructible_v<RandGen>,
      "Random Generator must have a non-throwing move constructor"
  );
  static_assert(std::is_nothrow_swappable_v<RandGen>, "Random Generator must have a non-throwing swap");

public:
  using ValueType = T;

  using Reference = T&;
  using ConstReference = const T&;

  using Pointer = void*;
  using ConstPointer = void*;

  using Iterator = T*;
  using ConstIterator = const T*;

  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

public:
  Treap() noexcept;

  explicit Treap(const RandGen& rg) noexcept;

  Treap(const Treap& other);

  Treap(Treap&& other) noexcept;

  Treap& operator=(const Treap& other);

  Treap& operator=(Treap&& other) noexcept;

  ~Treap();

  void clear() noexcept;

  std::size_t size() const noexcept;

  bool empty() const noexcept;

  ConstIterator begin() const noexcept;

  ConstIterator end() const noexcept;

  ConstReverseIterator rbegin() const noexcept;

  ConstReverseIterator rend() const noexcept;

  std::pair<Iterator, bool> insert(const T& value);

  std::pair<Iterator, bool> insert(T&& value);

  Iterator erase(ConstIterator pos) noexcept;

  std::size_t erase(const T& value);

  ConstIterator lower_bound(const T& value) const;

  ConstIterator upper_bound(const T& value) const;

  ConstIterator find(const T& value) const;

  friend void swap(Treap& lhs, Treap& rhs) noexcept;
};

} // namespace ct
