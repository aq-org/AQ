// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "basic/pair/pair.h"

namespace Aq {
namespace Basic {

template <class T1, class T2>
pair<T1, T2>::pair(const pair&) = default;

template <class T1, class T2>
pair<T1, T2>::pair(pair&&) = default;

template <class T1, class T2>
constexpr pair<T1, T2>::pair() = default;

template <class T1, class T2>
constexpr pair<T1, T2>::pair(const T1& x, const T2& y) : first(x), second(y) {}

template <class T1, class T2>
template <class U1, class U2>
constexpr pair<T1, T2>::pair(U1&& x, U2&& y)
    : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}

template <class T1, class T2>
template <class U1, class U2>
constexpr pair<T1, T2>::pair(const pair<U1, U2>& p)
    : first(p.first), second(p.second) {}

template <class T1, class T2>
template <class U1, class U2>
constexpr pair<T1, T2>::pair(pair<U1, U2>&& p)
    : first(std::move(p.first)), second(std::move(p.second)) {}

template <class T1, class T2>
template <class... Args1, class... Args2>
constexpr pair<T1, T2>::pair(piecewise_construct_t,
                             std::tuple<Args1...> first_args,
                             std::tuple<Args2...> second_args)
    : first(std::forward<Args1>(std::get<I>(first_args))...),
      second(std::forward<Args2>(std::get<J>(second_args))...) {}

template <class T1, class T2>
constexpr pair<T1, T2>& pair<T1, T2>::operator=(const pair& p) {
  first = p.first;
  second = p.second;
  return *this;
}

template <class T1, class T2>
template <class U1, class U2>
constexpr pair<T1, T2>& pair<T1, T2>::operator=(const pair<U1, U2>& p) {
  first = p.first;
  second = p.second;
  return *this;
}

template <class T1, class T2>
constexpr pair<T1, T2>& pair<T1, T2>::operator=(pair&& p) noexcept {
  first = std::move(p.first);
  second = std::move(p.second);
  return *this;
}

template <class T1, class T2>
template <class U1, class U2>
constexpr pair<T1, T2>& pair<T1, T2>::operator=(pair<U1, U2>&& p) {
  first = std::move(p.first);
  second = std::move(p.second);
  return *this;
}

template <class T1, class T2>
constexpr void pair<T1, T2>::swap(pair& p) noexcept {
  std::swap(first, p.first);
  std::swap(second, p.second);
}

}  // namespace Basic
}  // namespace Aq