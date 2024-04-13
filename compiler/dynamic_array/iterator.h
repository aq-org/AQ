// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYNAMIC_ARRAY_ITERATOR_H_
#define AQ_COMPILER_DYNAMIC_ARRAY_ITERATOR_H_

#include "compiler/compiler.h"
#include "compiler/dynamic_array/dynamic_array.h"

#include <cstddef>

namespace Aq {
template <typename T>
class Compiler::DynamicArray<T>::iterator {
 public:
  iterator();
  ~iterator();

  iterator(const iterator&) = default;
  iterator(iterator&&) noexcept = default;
  iterator& operator=(const iterator&) = default;
  iterator& operator=(iterator&&) noexcept = default;

  T& operator*() const;
  T* operator->() const;

  iterator& operator++();
  iterator operator++(int);

  iterator& operator--();
  iterator operator--(int);

  iterator operator+(std::ptrdiff_t n) const;
  iterator& operator+=(std::ptrdiff_t n);

  iterator operator-(std::ptrdiff_t n) const;
  iterator operator-=(std::ptrdiff_t n);

  iterator& operator[](std::ptrdiff_t n);

  bool operator==(const iterator& other) const;
  bool operator!=(const iterator& other) const;

  std::ptrdiff_t operator-(const Iterator& other) const;
};
}  // namespace Aq

#endif