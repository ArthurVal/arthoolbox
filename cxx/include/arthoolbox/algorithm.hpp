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
    return first + 1;
  } else return first;
}

}
}
