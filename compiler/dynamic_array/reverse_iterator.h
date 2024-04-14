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
class Compiler::DynamicArray<T>::reverse_iterator {
 public:
  reverse_iterator(DynamicArray<T>* array, std::size_t index);
  ~reverse_iterator();

  reverse_iterator(const reverse_iterator&) = default;
  reverse_iterator(reverse_iterator&&) noexcept = default;
  reverse_iterator& operator=(const reverse_iterator&) = default;
  reverse_iterator& operator=(reverse_iterator&&) noexcept = default;

  T& operator*() const;
  T* operator->() const;

  reverse_iterator& operator++();
  reverse_iterator operator++(int);

  reverse_iterator& operator--();
  reverse_iterator operator--(int);

  reverse_iterator operator+(std::ptrdiff_t n) const;
  reverse_iterator& operator+=(std::ptrdiff_t n);

  reverse_iterator operator-(std::ptrdiff_t n) const;
  reverse_iterator operator-=(std::ptrdiff_t n);

  reverse_iterator& operator[](std::size_t n);

  bool operator==(const reverse_iterator& other) const;
  bool operator!=(const reverse_iterator& other) const;

  std::ptrdiff_t operator-(const reverse_iterator& other) const;

 private:
  DynamicArray<T>* array_;
  std::size_t index_;
  T& data_;
};
}  // namespace Aq

#endif