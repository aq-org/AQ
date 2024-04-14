// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYNAMIC_ARRAY_CONST_ITERATOR_H_
#define AQ_COMPILER_DYNAMIC_ARRAY_CONST_ITERATOR_H_

#include "compiler/compiler.h"
#include "compiler/dynamic_array/dynamic_array.h"

#include <cstddef>

namespace Aq {
template <typename T>
class Compiler::DynamicArray<T>::const_iterator {
 public:
  const_iterator(DynamicArray<T>* array, std::size_t index);
  ~const_iterator();

  const_iterator(const const_iterator&) = default;
  const_iterator(const_iterator&&) noexcept = default;
  const_iterator& operator=(const const_iterator&) = default;
  const_iterator& operator=(const_iterator&&) noexcept = default;

  const T& operator*() const;
  const T* operator->() const;

  const_iterator& operator++();
  const_iterator operator++(int);

  const_iterator& operator--();
  const_iterator operator--(int);

  const_iterator operator+(std::ptrdiff_t n) const;
  const_iterator& operator+=(std::ptrdiff_t n);

  const_iterator operator-(std::ptrdiff_t n) const;
  const_iterator operator-=(std::ptrdiff_t n);

  const_iterator& operator[](std::size_t n);

  bool operator==(const const_iterator& other) const;
  bool operator!=(const const_iterator& other) const;

  std::ptrdiff_t operator-(const const_iterator& other) const;

 private:
  DynamicArray<T>* array_;
  std::size_t index_;
  const T& data_;
};
}  // namespace Aq

#endif