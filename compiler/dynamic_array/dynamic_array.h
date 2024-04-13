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
  ~DynamicArray();

  DynamicArray(const DynamicArray&);
  DynamicArray(DynamicArray&&) noexcept;
  DynamicArray& operator=(const DynamicArray&);
  DynamicArray& operator=(DynamicArray&&) noexcept;

  class iterator;

  T& at(std::size_t index);
  T& operator[](std::size_t index);
  const T& operator[](std::size_t index) const;
  T& front();
  T& back();
  T* data();

  typename DynamicArray<T>::iterator begin();
  typename DynamicArray<T>::iterator end();
  typename DynamicArray<T>::reverse_iterator rbegin();
  typename DynamicArray<T>::reverse_iterator rend();

  bool empty() const;
  std::size_t size() const;
  std::size_t max_size() const;
  void reserve(std::size_t new_cap);
  std::size_t capacity() const;
  void shrink_to_fit();

  void clear();
  typename DynamicArray<T>::iterator insert(typename DynamicArray<T>::iterator pos, const T& value);
  typename DynamicArray<T>::iterator insert(typename DynamicArray<T>::iterator pos, std::size_t count, const T& value);
  template<typename InputIt>
  typename DynamicArray<T>::iterator insert(typename DynamicArray<T>::iterator pos, InputIt first, InputIt last);
  typename DynamicArray<T>::iterator erase(typename DynamicArray<T>::iterator pos);
  typename DynamicArray<T>::iterator erase(typename DynamicArray<T>::iterator first, typename DynamicArray<T>::iterator last);
  void push_back(const T& value);
  void pop_back();
  void resize(std::size_t count);
  void swap(DynamicArray<T>& other);
  void assign(std::size_t count, const T& value);
  template<typename InputIt>
  void assign(InputIt first, InputIt last);

  bool operator==(const DynamicArray& other) const;
  bool operator!=(const DynamicArray& other) const;

 private:
  T* array_;
  std::size_t capacity_;
  std::size_t size_;
};
}

#endif