#pragma once

#include <type_traits>

namespace arthoolbox {
namespace traits {
/// Type holding a list of type from a parameter pack
template <class... U> struct type_sequence {};

/// Extract the first type T of a parameter pack
template <class... U> struct head;

template <class T, class... U> struct head<type_sequence<T, U...>> {
  using type = T;
};

template <class T> using head_t = typename head<T>::type;

/// Extract the list of type ...U AFTER the first type T of a parameter pack
template <class... U> struct tail;

template <class T, class... U> struct tail<type_sequence<T, U...>> {
  using type = type_sequence<U...>;
};

template <class T> using tail_t = typename tail<T>::type;

/// Create a new type_sequence by adding T at the begining of the sequence
template <class T, class TypeSequence> struct cons;

template <class T, class... U> struct cons<T, type_sequence<U...>> {
  using type = type_sequence<T, U...>;
};

template <class T, class TypeSequence>
using cons_t = typename cons<T, TypeSequence>::type;

/// Simply checks if T is a type_sequence
template <class T> struct is_type_sequence : std::false_type {};

template <class... Args>
struct is_type_sequence<type_sequence<Args...>> : std::true_type {};

/// Get the size of a type_sequence
template <class T> struct type_sequence_size;

template <class... T>
struct type_sequence_size<type_sequence<T...>>
    : std::integral_constant<std::size_t, sizeof...(T)> {};

template <class T>
constexpr std::size_t type_sequence_size_v = type_sequence_size<T>::value;

/// Find the index of T inside the parameter pack Pack. Return Start +
/// sizeof...(Pack) if failure.
template <std::size_t Start, class Type, class... Pack> struct get_index_of;

template <std::size_t Idx, class T, class U, class... Rest>
struct get_index_of<Idx, T, U, Rest...> : get_index_of<Idx + 1, T, Rest...> {};

template <std::size_t Idx, class T, class... Rest>
struct get_index_of<Idx, T, T, Rest...>
    : std::integral_constant<std::size_t, Idx> {};

template <std::size_t Idx, class T>
struct get_index_of<Idx, T> : std::integral_constant<std::size_t, Idx + 1> {};

template <std::size_t Idx, class T, class... Pack>
struct get_index_of<Idx, T, type_sequence<Pack...>>
    : get_index_of<Idx, T, Pack...> {};

template <std::size_t Start, class T, class... Pack>
constexpr std::size_t get_index_of_v = get_index_of<Start, T, Pack...>::value;

/// Create a type_sequence representing a sub_view of types from a given
/// parameter pack.
template <std::size_t begin, std::size_t end, std::size_t idx, class = void,
          class... Pack>
struct sub_view;

template <std::size_t begin, std::size_t end, std::size_t idx, class First,
          class... Pack>
struct sub_view<begin, end, idx, std::enable_if_t<(idx < begin)>, First,
                Pack...> {
  using type = typename sub_view<begin, end, idx + 1, void, Pack...>::type;
};

template <std::size_t begin, std::size_t end, class First, class... Pack>
struct sub_view<begin, end, begin, void, First, Pack...> {
  using type = typename sub_view<begin, end, begin + 1, void,
                                 type_sequence<First>, Pack...>::type;
};

template <std::size_t begin, std::size_t end, std::size_t idx, class First,
          class... Types, class... Pack>
struct sub_view<begin, end, idx,
                std::enable_if_t<(idx > begin) and (idx < end)>,
                type_sequence<Types...>, First, Pack...> {
  using type = typename sub_view<begin, end, idx + 1, void,
                                 type_sequence<Types..., First>, Pack...>::type;
};

template <std::size_t begin, std::size_t end, class... Types, class... Pack>
struct sub_view<begin, end, end, void, type_sequence<Types...>, Pack...> {
  using type = type_sequence<Types...>;
};

template <std::size_t begin, std::size_t end, class... Pack>
using sub_view_t = typename sub_view<begin, end, 0, void, Pack...>::type;

/// Transform a TypeSequence<U...> into TypeSequence<T<U>...>.
template <template <class...> class T, class TypeSequence> struct transform;

template <template <class...> class T, class... Types>
struct transform<T, type_sequence<Types...>> {
  using type = type_sequence<T<Types>...>;
};

template <template <class...> class T, class TypeSequence>
using transform_t = typename transform<T, TypeSequence>::type;

/// Transform a TypeSequence<U...>  TypeSequence<V...>, into TS<T<U,V>...>.
template <template <class...> class T, class TypeSequence1, class TypeSequence2>
struct transform_2;

template <template <class...> class T, class... Types1, class... Types2>
struct transform_2<T, type_sequence<Types1...>, type_sequence<Types2...>> {
  static_assert(sizeof...(Types1) == sizeof...(Types2));
  using type = type_sequence<T<Types1, Types2>...>;
};

template <template <class...> class T, class TypeSequence1, class TypeSequence2>
using transform_2_t =
    typename transform_2<T, TypeSequence1, TypeSequence2>::type;

/// Reduce the Typesequence<U...> into T, as of T<U...>
template <class TypeSequence, template <class...> class T> struct reduce;

template <template <class...> class T, class... Types>
struct reduce<type_sequence<Types...>, T> {
  using type = T<Types...>;
};

template <class TypeSequence, template <class...> class T>
using reduce_t = typename reduce<TypeSequence, T>::type;

/// Test if all types in T... are unique
template <class... T> struct all_different;

template <class... T>
struct all_different<type_sequence<T...>> : all_different<T...> {};

template <class T> struct all_different<T> : std::true_type {};

template <class T1, class T2, class... Ts>
struct all_different<T1, T2, Ts...>
    : std::conjunction<all_different<T1, T2>, all_different<T1, Ts...>,
                       all_different<T2, Ts...>> {};

template <class T1, class T2>
struct all_different<T1, T2> : std::negation<std::is_same<T1, T2>> {};

template <class... T>
constexpr bool all_different_v = all_different<T...>::value;

/// Test if T is dereferenceable
template <class T, class = void> struct is_dereferenceable : std::false_type {};

template <class T>
struct is_dereferenceable<T, std::void_t<decltype(*std::declval<T>())>>
    : std::true_type {};

template <class T>
constexpr bool is_dereferenceable_v = is_dereferenceable<T>::value;

/// Test if T is testable
template <class T, class = void> struct is_testable : std::false_type {};

template <class T>
struct is_testable<T, std::void_t<decltype(bool(std::declval<T>()))>>
    : std::true_type {};

template <class T> constexpr bool is_testable_v = is_testable<T>::value;

/// TODO
template <class T, template <class> class... U> struct chain;

template <class T, template <class> class U, template <class> class... V>
struct chain<T, U, V...> : chain<typename U<T>::type, V...> {};

template <class T, template <class> class U> struct chain<T, U> : U<T> {};

} // namespace traits
} // namespace arthoolbox
