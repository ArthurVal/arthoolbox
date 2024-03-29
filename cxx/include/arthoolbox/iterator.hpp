#pragma once

#include <iterator>
#include <type_traits>

namespace arthoolbox {
namespace iterator {

/**
 * TODO
 */
template <class _Iterator>
class strided_iterator {
  using __traits_type = std::iterator_traits<_Iterator>;

 public:
  using iterator_type = _Iterator;
  using difference_type = typename __traits_type::difference_type;
  using value_type = typename __traits_type::value_type;
  using pointer = typename __traits_type::pointer;
  using reference = typename __traits_type::reference;
  using iterator_category = typename __traits_type::iterator_category;

 protected:
  _Iterator _current;
  const difference_type _stride;

 public:
  strided_iterator() = delete;

  /**
   * This iterator decorator will move multiple times, based on the _Stride,
   * when incremented/decremented.
   */
  constexpr strided_iterator(_Iterator start, const difference_type &stride)
      : _current(start), _stride(stride) {}

  constexpr explicit strided_iterator(const strided_iterator &other)
      : _current(other._current), _stride(other._stride) {}

  strided_iterator &operator=(const strided_iterator &) = default;

  /**
   * strided_iterator can be copy constructed/assigned from other
   * strided_iterator ONLY if their underlying _Iter type is convertible towards
   * _Iterator.
   */
  template <class _Iter, typename std::enable_if<
                             std::is_convertible<_Iter, _Iterator>::value,
                             bool>::type = true>
  constexpr strided_iterator(const strided_iterator<_Iter> &other)
      : _current(other._current), _stride(other._stride) {}

  template <class _Iter, typename std::enable_if<
                             std::is_convertible<_Iter, _Iterator>::value,
                             bool>::type = true>
  constexpr strided_iterator &operator=(const strided_iterator<_Iter> &other) {
    _current = other._current;
    _stride = other._stride;
    return *this;
  }

  /**
   * @return The current underlying iterator.
   */
  constexpr iterator_type base() const { return _current; }

  /**
   * @return The current stride used.
   */
  constexpr difference_type stride() const { return _stride; }

  /**
   * This require that _current is dereferenceable.
   *
   * @return A reference to the value currently pointed at.
   */
  constexpr reference operator*() const { return *_current; }

  /**
   * @return A pointer to the value currently pointed at.
   */
  constexpr pointer operator->() const { return to_pointer(_current); }

  /**
   * Advance the underlying _current iterator by '_Stride'.
   *
   * @return *this
   */
  constexpr reference &operator++() {
    std::advance(_current, _stride);
    return *this;
  }

  /**
   * Advance the underlying _current iterator by _Stride steps.
   *
   * @return The original value of *this before advancing.
   */
  constexpr strided_iterator operator++(int) {
    strided_iterator tmp = *this;
    operator++();
    return tmp;
  }

  /**
   * Decrement the underlying _current iterator by _Stride steps.
   *
   * _Iterator must be a Bidirectional Iterator.
   *
   * @return *this.
   */
  constexpr reference operator--() {
    static_assert(std::is_base_of<std::bidirectional_iterator_tag,
                                  iterator_category>::value,
                  "The iterator decorated must be BIDIRECTIONAL in order to "
                  "use (--foo) operator.");
    std::advance(_current, -_stride);
    return *this;
  }

  /**
   * Decrement the underlying _current iterator _Stride steps.
   *
   * _Iterator must be a Bidirectional Iterator.
   *
   * @return The original value of *this before advancing.
   */
  constexpr strided_iterator operator--(int) {
    static_assert(std::is_base_of<std::bidirectional_iterator_tag,
                                  iterator_category>::value,
                  "The iterator decorated must be BIDIRECTIONAL in order to "
                  "use (foo--) operator.");
    strided_iterator tmp(*this);
    operator--();
    return tmp;
  }

  /**
   * The underlying _current iterator must be a Random Access Iterator.
   *
   * @return A strided_iterator that refers to (_current + (n*stride))
   */
  constexpr strided_iterator operator+(difference_type n) const {
    static_assert(std::is_base_of<std::random_access_iterator_tag,
                                  iterator_category>::value,
                  "The iterator decorated must be RANDOM ACCESS in order to "
                  "use (foo + N) operator.");
    return strided_iterator(_current + (n * _stride));
  }

  /**
   * Increment the underlying _current iterator by _Stride*n steps.
   *
   * _Iterator must be a Random Access Iterator.
   *
   * @return *this
   */
  constexpr strided_iterator &operator+=(difference_type n) {
    static_assert(std::is_base_of<std::random_access_iterator_tag,
                                  iterator_category>::value,
                  "The iterator decorated must be RANDOM ACCESS in order to "
                  "use (foo += N) operator.");
    _current += (n * _stride);
    return *this;
  }

  /**
   * The underlying _current iterator must be a Random Access Iterator.
   *
   * @return A strided_iterator that refers to (_current - (n*stride))
   */
  constexpr strided_iterator operator-(difference_type n) const {
    static_assert(std::is_base_of<std::random_access_iterator_tag,
                                  iterator_category>::value,
                  "The iterator decorated must be RANDOM ACCESS in order to "
                  "use (foo - N) operator.");
    return strided_iterator(_current - (n * _stride));
  }

  /**
   * Decrement the underlying _current iterator by _Stride*n steps.
   *
   * _Iterator must be a Random Access Iterator.
   *
   * @return *this
   */
  constexpr strided_iterator &operator-=(difference_type n) {
    static_assert(std::is_base_of<std::random_access_iterator_tag,
                                  iterator_category>::value,
                  "The iterator decorated must be RANDOM ACCESS in order to "
                  "use (foo -= N) operator.");
    _current -= (n * _stride);
    return *this;
  }

  /**
   * _Iterator must be a Random Access Iterator.
   *
   * @return The value at _current + (n*stride)
   */
  constexpr reference operator[](difference_type n) const {
    static_assert(std::is_base_of<std::random_access_iterator_tag,
                                  iterator_category>::value,
                  "The iterator decorated must be RANDOM ACCESS in order to "
                  "use foo[N] operator.");
    return *(*this + n);
  }

 private:
  template <class T>
  static constexpr T *to_pointer(T *p) {
    return p;
  }

  template <class T>
  static constexpr pointer to_pointer(T p) {
    return p.operator->();
  }
};

template <class _IterL, class _IterR>
inline constexpr bool operator==(const strided_iterator<_IterL> &lhs,
                                 const strided_iterator<_IterR> &rhs) {
  return lhs.base() == rhs.base();
}

template <class _IterL, class _IterR>
inline constexpr bool operator!=(const strided_iterator<_IterL> &lhs,
                                 const strided_iterator<_IterR> &rhs) {
  return not(lhs == rhs);
}

template <class _IterL, class _IterR>
inline constexpr bool operator<(const strided_iterator<_IterL> &lhs,
                                const strided_iterator<_IterR> &rhs) {
  static_assert(
      std::is_base_of<std::random_access_iterator_tag,
                      typename decltype(lhs)::iterator_category>::value and
          std::is_base_of<std::random_access_iterator_tag,
                          typename decltype(rhs)::iterator_category>::value,
      "Both strided_iterator must be RANDOM ACCESS in order to "
      "use <, >, <= and >= comparisons.");
  return lhs.base() < rhs.base();
}

template <class _IterL, class _IterR>
inline constexpr bool operator>(const strided_iterator<_IterL> &lhs,
                                const strided_iterator<_IterR> &rhs) {
  return rhs < lhs;
}

template <class _IterL, class _IterR>
inline constexpr bool operator<=(const strided_iterator<_IterL> &lhs,
                                 const strided_iterator<_IterR> &rhs) {
  return not(rhs < lhs);
}

template <class _IterL, class _IterR>
inline constexpr bool operator>=(const strided_iterator<_IterL> &lhs,
                                 const strided_iterator<_IterR> &rhs) {
  return not(lhs < rhs);
}

template <class _IterL, class _IterR>
inline constexpr auto operator+(const strided_iterator<_IterL> &lhs,
                                const strided_iterator<_IterR> &rhs)
    -> decltype(lhs.base() + rhs.base()) {
  return lhs.base() + rhs.base();
}

template <class _IterL, class _IterR>
inline constexpr auto operator-(const strided_iterator<_IterL> &lhs,
                                const strided_iterator<_IterR> &rhs)
    -> decltype(lhs.base() - rhs.base()) {
  return lhs.base() - rhs.base();
}

}  // namespace iterator
}  // namespace arthoolbox
