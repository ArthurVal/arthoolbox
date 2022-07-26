#pragma once

#include <type_traits>

namespace arthoolbox {
namespace traits {
/// Type holding a list of type from a parameter pack
template <class... T> struct TypeSequence {};

/// Extract the first type T of a parameter pack
template <class... T> struct Head;

template <class... T> struct First : Head<T...> {};

template <class T, class... U> struct Head<TypeSequence<T, U...>> {
  using type = T;
};

template <class T> using Head_t = typename Head<T>::type;

template <class T> using First_t = typename First<T>::type;

/// Extract the list of type ...U AFTER the first type T of a parameter pack
template <class... U> struct Tail;

template <class T, class... U> struct Tail<TypeSequence<T, U...>> {
  using type = TypeSequence<U...>;
};

template <> struct Tail<TypeSequence<>> { using type = TypeSequence<>; };

template <class T> using Tail_t = typename Tail<T>::type;

/// Create a new type_sequence by adding T at the begining of the sequence
template <class... T> struct Cons;

template <class T, class U> struct Cons<T, U> {
  using type = TypeSequence<T, U>;
};

template <class T, class... U> struct Cons<T, TypeSequence<U...>> {
  using type = TypeSequence<T, U...>;
};

template <class... T, class U> struct Cons<TypeSequence<T...>, U> {
  using type = TypeSequence<T..., U>;
};

template <class... T, class... U>
struct Cons<TypeSequence<T...>, TypeSequence<U...>> {
  using type = TypeSequence<T..., U...>;
};

template <class... T> using Cons_t = typename Cons<T...>::type;

/// Extract the last type of TypeSequence
template <class... T> struct Last;

template <class T> struct Last<TypeSequence<T>> { using type = T; };

template <class... T>
struct Last<TypeSequence<T...>> : Last<Tail_t<TypeSequence<T...>>> {};

template <class T> using Last_t = typename Last<T>::type;

/// Get the size of a TypeSequence
template <class... T> struct SizeOf {
  static constexpr auto value = sizeof...(T);
};

template <class... T> struct SizeOf<TypeSequence<T...>> : SizeOf<T...> {};

template <class T> constexpr auto SizeOf_v = SizeOf<T>::value;

/// Get the Type T at index I in Sequence
template <class Sequence, std::size_t I, std::size_t Idx>
struct AtImpl : AtImpl<Tail_t<Sequence>, I, Idx + 1> {
  static_assert(Idx < I);
};

template <std::size_t I, class Sequence>
struct AtImpl<Sequence, I, I> : Head<Sequence> {};

template <class Sequence, std::size_t I> struct At : AtImpl<Sequence, I, 0> {
  static_assert(I < SizeOf_v<Sequence>);
};

template <class Sequence, std::size_t I>
using At_t = typename At<Sequence, I>::type;

/// Create a sub sequence from given indexes
template <class InSequence, class OutSequence, std::size_t... Indexes>
struct SubViewImpl;

template <class InSequence, class OutSequence, std::size_t I,
          std::size_t... Others>
struct SubViewImpl<InSequence, OutSequence, I, Others...>
    : SubViewImpl<InSequence, Cons_t<OutSequence, At_t<InSequence, I>>,
                  Others...> {};

template <class InSequence, class OutSequence, std::size_t I>
struct SubViewImpl<InSequence, OutSequence, I>
    : Cons<OutSequence, At_t<InSequence, I>> {};

template <class InSequence, class OutSequence>
struct SubViewImpl<InSequence, OutSequence> {
  using type = OutSequence;
};

template <class Sequence, class IdxSeq> struct SubView;

template <class Sequence, std::size_t... Indexes>
struct SubView<Sequence, std::integer_sequence<std::size_t, Indexes...>>
    : SubViewImpl<Sequence, TypeSequence<>, Indexes...> {};

template <class Sequence, class IdxSeq>
using SubView_t = typename SubView<Sequence, IdxSeq>::type;

/// Offset a std::index_sequence by N
template <std::size_t N, class IdxSeq> struct OffsetIndexSequence;

template <std::size_t N, std::size_t... Indexes>
struct OffsetIndexSequence<N, std::integer_sequence<std::size_t, Indexes...>> {
  using type = std::integer_sequence<std::size_t, (Indexes + N)...>;
};

template <std::size_t N, class IdxSeq>
using OffsetIndexSequence_t = typename OffsetIndexSequence<N, IdxSeq>::type;

/// Inside a Sequence, replace the type at index I, by NewT
template <class Sequence, std::size_t I, class NewT> struct Replace {
  static_assert(I < SizeOf_v<Sequence>);
  using Before_I = SubView_t<Sequence, std::make_index_sequence<I>>;
  using After_I =
      SubView_t<Sequence,
                OffsetIndexSequence_t<I + 1, std::make_index_sequence<
                                                 SizeOf_v<Sequence> - I - 1>>>;

public:
  using type = Cons_t<Before_I, Cons_t<NewT, After_I>>;
};

template <class Sequence, class NewT>
struct Replace<Sequence, 0, NewT> : Cons<NewT, Tail_t<Sequence>> {};

template <class Sequence, std::size_t I, class NewT>
using Replace_t = typename Replace<Sequence, I, NewT>::type;

/// Create T using all types contained within the TypeSequence
template <template <class...> class T, class TypeSequence> struct FromSequence;

template <template <class...> class T, class... Types>
struct FromSequence<T, TypeSequence<Types...>> {
  using type = T<Types...>;
};

template <template <class...> class T, class TypeSequence>
using FromSequence_t = typename FromSequence<T, TypeSequence>::type;

} // namespace traits
} // namespace arthoolbox
