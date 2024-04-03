// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_
#define AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "debugger/debugger.h"

namespace Aq {
template <typename ArrayType>
class Compiler::DynArray {
 public:
  DynArray(std::size_t InitCapacity = 1);
  ~DynArray();

  DynArray(const DynArray&) = default;
  DynArray(DynArray&&) noexcept = default;
  DynArray& operator=(const DynArray&) = default;
  DynArray& operator=(DynArray&&) noexcept = default;

  // Returns the data reference of the corresponding index.
  ArrayType& operator[](std::size_t index);

  // Adds an element to the end of the container. No return value.
  void Insert(ArrayType data);

  // Increase the container capacity.
  // If |new_capacity| is 0, it is increased to 2 times |capacity_|. If
  // greater than 0, the container capacity is increased to |new_capacity|.
  // A return of 0 indicates a successful allocation, a return of -1 indicates
  // an error in the allocation.
  int Resize(std::size_t new_capacity = 0);

  // Returns the number of elements in the container.
  std::size_t Size() const;

  // The iterator of the container.
  class Iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;

    Iterator(DynArray<ArrayType>* array, std::size_t index = 0);

    ~Iterator();

    Iterator operator+(std::size_t size) const;
    Iterator& operator++();
    Iterator operator++(int);
    Iterator operator-(std::size_t size) const;
    std::size_t operator-(const Iterator& other) const;
    Iterator& operator--();
    Iterator operator--(int);
    Iterator& operator+=(std::size_t size);
    Iterator& operator-=(std::size_t size);
    ArrayType& operator*() const;
    ArrayType* operator->() const;
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;
    bool operator<(const Iterator& other) const;
    bool operator<=(const Iterator& other) const;
    bool operator>(const Iterator& other) const;
    bool operator>=(const Iterator& other) const;

   private:
    DynArray<ArrayType>* array_;
    std::size_t index_;
  };

  // Returns an iterator to the beginning of the container.
  Iterator Begin() { return Iterator(this, 0); }

  // Returns an iterator to the end of the container.
  Iterator End() { return Iterator(this, size_); }

 private:
  ArrayType* data_;
  std::size_t capacity_;
  std::size_t size_;
};
}  // namespace Aq

#endif