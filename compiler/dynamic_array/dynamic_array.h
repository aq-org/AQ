// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYNAMIC_ARRAY_DYNAMIC_ARRAY_H_
#define AQ_COMPILER_DYNAMIC_ARRAY_DYNAMIC_ARRAY_H_

#include "compiler/compiler.h"

// Wait development.
namespace Aq {
template <typename T>
class Compiler::DynamicArray {
 public:
  DynamicArray();
  DynamicArray(size_t count, const T& value = T());
  ~DynamicArray();

  DynamicArray(const DynamicArray&);
  DynamicArray(DynamicArray&&) noexcept;
  DynamicArray& operator=(const DynamicArray&);
  DynamicArray& operator=(DynamicArray&&) noexcept;

  class iterator;

  iterator begin();
  const iterator begin() const;
  iterator end();
  const iterator end() const;
  iterator cbegin() const;
  iterator cend() const;

  size_t size() const;
  bool empty() const;
  void reserve(size_t new_cap);
  size_t capacity() const;
  void shrink_to_fit();

  T& operator[](size_t index);
  const T& operator[](size_t index) const;
  T& at(size_t index);
  const T& at(size_t index) const;
  T& front();
  const T& front() const;
  T& back();
  const T& back() const;
  T* data();
  const T* data() const;

  void push_back(const T& value);
  void push_back(T&& value);
  iterator insert(const iterator pos, const T& value);
  iterator insert(const iterator pos, T&& value);
  iterator insert(const iterator pos, size_t count, const T& value);
  iterator insert(const iterator pos, iterator first, iterator last);

  iterator erase(const iterator pos);
  iterator erase(const iterator first, const iterator last);
  void pop_back();
  void clear();

  void swap(DynamicArray& other);
  void resize(size_t count);
  void resize(size_t count, const T& value);
  void fill(const T& value);

  void assign(iterator first, iterator last);
  void assign(size_t count, const T& value);
  bool operator==(const DynamicArray& other) const;
  bool operator!=(const DynamicArray& other) const;
  bool operator<(const DynamicArray& other) const;
  bool operator>(const DynamicArray& other) const;
  bool operator<=(const DynamicArray& other) const;
  bool operator>=(const DynamicArray& other) const;

 private:
  T* array_;
  std::size_t capacity_;
  std::size_t size_;
};
}  // namespace Aq

#endif