// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_
#define AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "debugger/debugger.h"

namespace Aq {
template <typename ArrayType, std::size_t InitCapacity>
class Compiler::DynArray {
 public:
  DynArray();
  ~DynArray();

  DynArray(const DynArray&) = default;
  DynArray(DynArray&&) noexcept = default;
  DynArray& operator=(const DynArray&) = default;
  DynArray& operator=(DynArray&&) noexcept = default;

  // Returns the data reference of the corresponding index.
  ArrayType& operator[](std::size_t index) {
    if (index >= size_) {
      Debugger error_info(
          Debugger::Level::ERROR, "Aq::Compiler::DynArray::operator[]",
          "DynArray_IndexError", "Index out of range occurred.", nullptr);
    }
    return data_[index];
  }

  // Adds an element to the end of the container. No return value.
  void PushBack(ArrayType data);

  // Increase the container capacity.
  // If |new_capacity| is 0, it is increased to 2 times |capacity_|. If
  // greater than 0, the container capacity is increased to |new_capacity|.
  // A return of 0 indicates a successful allocation, a return of -1 indicates
  // an error in the allocation.
  int Resize(std::size_t new_capacity = 0);

 private:
  ArrayType* data_;
  std::size_t capacity_;
  std::size_t size_;
};
}  // namespace Aq

#endif