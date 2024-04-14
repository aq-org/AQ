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
  DynamicArray(std::size_t capacity, const T& value = T());
  ~DynamicArray();

  DynamicArray(const DynamicArray&);
  DynamicArray(DynamicArray&&) noexcept;
  DynamicArray& operator=(const DynamicArray&);
  DynamicArray& operator=(DynamicArray&&) noexcept;

  class iterator;
  class const_iterator;
  class reverse_iterator;
  class const_reverse_iterator;

  iterator begin();
  const_iterator begin() const;
  const_iterator cbegin() const;
  reverse_iterator rbegin();
  const_reverse_iterator rbegin() const;
  const_reverse_iterator crbegin() const;
  iterator end();
  const_iterator end() const;
  const_iterator cend() const;
  reverse_iterator rend();
  const_reverse_iterator rend() const;
  const_reverse_iterator crend() const;

  std::size_t size() const;
  bool empty() const;
  void reserve(std::size_t count);
  std::size_t capacity() const;
  void shrink_to_fit();

  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;
  T& at(std::size_t index);
  const T& at(std::size_t index) const;
  T& front();
  const T& front() const;
  T& back();
  const T& back() const;
  T* data();
  const T* data() const;

  void push_back(const T& value);
  void push_back(T&& value);
  iterator insert(const_iterator position, const T& value);
  iterator insert(const_iterator position, T&& value);
  iterator insert(const_iterator position, std::size_t count, const T& value);
  iterator insert(const_iterator position, iterator first, iterator last);

  iterator erase(const_iterator position);
  iterator erase(const_iterator first, const_iterator last);
  void pop_back();
  void clear();

  void swap(DynamicArray& other);
  void resize(std::size_t count);
  void resize(std::size_t count, const T& value);
  void fill(const T& value);

  void assign(iterator first, iterator last);
  void assign(std::size_t count, const T& value);
  bool operator==(const DynamicArray& other) const;
  bool operator!=(const DynamicArray& other) const;
  bool operator<(const DynamicArray& other) const;
  bool operator>(const DynamicArray& other) const;
  bool operator<=(const DynamicArray& other) const;
  bool operator>=(const DynamicArray& other) const;

 private:
  T* array_ = nullptr;
  std::size_t capacity_ = 0;
  std::size_t size_ = 0;
};
}  // namespace Aq

#endif