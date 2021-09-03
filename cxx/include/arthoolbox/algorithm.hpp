#pragma once

#include <algorithm>
#include <iterator>

namespace arthoolbox {
namespace algorithm {

/**
 * @brief Mutate a range of elements by swaping the first element with value if
 * value is found
 *
 * @tparam ForwardOutputIt Iterator type (Must be writable, ie non const)
 * @tparam T Value type (must be comparable with
 * std::iterator_traits<ForwardOutputIt>::value_type)

 * @param[in] first The begin iterator of the range
 * @param[in] last The end iterator of the range
 * @param[in] value The value to swap in front of the range
 *
 * @returns ForwardOutputIt first if the value couldn't be find, otherwise the
 * iterator past first if swap ovccured
 */
template<class ForwardOutputIt, class T>
constexpr ForwardOutputIt swap_front(
  ForwardOutputIt first, ForwardOutputIt last, const T& value) {
  auto value_found = std::find(first, last, value);
  if(value_found != last ) {
    if(value_found != first) std::iter_swap(value_found, first);
    return std::next(first);
  } else return first;
}

/**
 * @brief Swap elements of [first1, last1[ found inside [first2, last2[ at the
 * beginning of [first2, last2[ and put missing elements in d_first
 *
 * In order to compute the difference of A-B, elements of the 2nd list B will be
 * SWAPPED. Every elements of A found inside B will be swapped at the beginning
 * of the list B. Additionally, all elements from A that are not contained
 * inside B will be pushed into the d_first destination range.
 *
 * The 'mutation' occurs in order to 'move out' elements that we already found,
 * this way, when we checks for the next value in the range B, the range will be
 * shorten by the elements already found and that don't need to be look at
 * again.
 *
 * @tparam InputIt Input Iterator (not mutated) of the first range
 * @tparam ForwardOutputIt1 Iterator of the 2nd list (Elements in the list will
 * be swapped)
 * @tparam OutputIt2 Output iterator use to store missing values
 *
 * @param[in] first1,last1 The range of elements to examine
 * @param[in] first2,last2 The range of elements to search in
 * @param[out] d_first The beginning of the destination range
 *
 * @returns ForwardOutputIt1 Iterator to the first element in [first2, last2[
 * that as not been found inside [first1, last1[
 */
template <class InputIt, class ForwardOutputIt1, class OutputIt2>
constexpr ForwardOutputIt1 array_difference(InputIt first1, InputIt last1,
                                            ForwardOutputIt1 first2,
                                            ForwardOutputIt1 last2,
                                            OutputIt2 d_first) {
  auto first2_not_found_value = first2;
  for (auto value = first1; value != last1; ++value) {
    auto value_found = std::find(first2_not_found_value, last2, *value);
    if (value_found == last2) {
      *d_first++ = *value;
    } else {
      std::iter_swap(first2_not_found_value, value_found);
      std::advance(first2_not_found_value, 1);
    }
  }

  return first2_not_found_value;
}

}
}
