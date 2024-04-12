// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYNAMIC_ARRAY_DYNAMIC_ARRAY_H_
#define AQ_COMPILER_DYNAMIC_ARRAY_DYNAMIC_ARRAY_H_

#include "compiler/compiler.h"

// Wait development.
namespace Aq {
template <typename ArrayType>
class Compiler::DynamicArray {
 public:
  DynamicArray();
  ~DynamicArray();

  DynamicArray(const DynamicArray&);
  DynamicArray(DynamicArray&&) noexcept;
  DynamicArray& operator=(const DynamicArray&);
  DynamicArray& operator=(DynamicArray&&) noexcept;

  ArrayType& operator[](std::size_t index);
  const ArrayType& operator[](std::size_t index) const;

  ArrayType& at(std::size_t index);
  const ArrayType& at(std::size_t index) const;

  void push_back(const ArrayType& value);
  void push_back(ArrayType&& value);
  void insert(std::size_t index, const ArrayType& value);
  void insert(std::size_t index, ArrayType&& value);

  void pop_back();
  void erase(std::size_t index);
  void clear();

  std::size_t size() const;
  std::size_t capacity() const;

  void resize(std::size_t new_size);
  void reserve(std::size_t new_capacity);

 private:
  ArrayType* array_;
  std::size_t capacity_;
  std::size_t size_;
};
}

#endif