// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_BASIC_PAIR_PAIR_H_
#define AQ_BASIC_PAIR_PAIR_H_

namespace Aq {
namespace Basic {

template <class T1, class T2>
struct pair {
  using first_type = T1;
  using second_type = T2;
  T1 first;
  T2 second;
  pair(const pair&);
  pair(pair&&);
  constexpr explicit pair();
  constexpr explicit pair(const T1& x, const T2& y);
  template <class U1, class U2>
  constexpr explicit pair(U1&& x, U2&& y);
  template <class U1, class U2>
  constexpr explicit pair(const pair<U1, U2>& p);
  template <class U1, class U2>
  constexpr explicit pair(pair<U1, U2>&& p);
  template <class... Args1, class... Args2>
  constexpr pair(piecewise_construct_t, tuple<Args1...> first_args,
                 tuple<Args2...> second_args);
  constexpr pair& operator=(const pair& p);
  template <class U1, class U2>
  constexpr pair& operator=(const pair<U1, U2>& p);
  constexpr pair& operator=(pair&& p) noexcept;
  template <class U1, class U2>
  constexpr pair& operator=(pair<U1, U2>&& p);
  constexpr void swap(pair& p) noexcept;
};
template <class T1, class T2>
pair(T1, T2) -> pair<T1, T2>;
}  // namespace Basic
}  // namespace Aq
#endif