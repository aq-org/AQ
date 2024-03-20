// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/dyn_array/dyn_array.h"

#include <cstddef>

#include "debugger/debugger.h"

namespace Aq {
template <typename ArrayType, std::size_t InitCapacity>
Compiler::DynArray<ArrayType, InitCapacity>::DynArray() {
  data_ = new ArrayType[InitCapacity];
  if (data_ != nullptr) {
    capacity_ = InitCapacity;
  } else {
    data_ = new ArrayType[1];
    if (data_ != nullptr) {
      Debugger warning_info(Debugger::Level::WARNING,
                            "Aq::Compiler::DynArray::DynArray",
                            "DynArray_InitWarning",
                            "InitCapacity out of memory occurred.", nullptr);
      capacity_ = 1;
    } else {
      Debugger error_info(
          Debugger::Level::ERROR, "Aq::Compiler::DynArray::DynArray",
          "DynArray_InitError", "Size 1 out of memory occurred.", nullptr);
      capacity_ = 0;
    }
  }
  size_ = 0;
}

template <typename ArrayType, std::size_t InitCapacity>
Compiler::DynArray<ArrayType, InitCapacity>::~DynArray() {
  delete[] data_;
}

template <typename ArrayType, std::size_t InitCapacity>
void Compiler::DynArray<ArrayType, InitCapacity>::PushBack(ArrayType data) {
  if (capacity_ == 0 && Resize(1) == -1) {
    return;
  }
  if (size_ > capacity_) {
    Debugger error_info(
        Debugger::Level::ERROR, "Aq::Compiler::DynArray::PushBack",
        "PushBack_SizeError", "Size out of capacity occurred.", nullptr);
  }
  if (size_ == capacity_) {
    Resize(capacity_ * 2);
  }
  data_[size_] = data;
  size_++;
}

template <typename ArrayType, std::size_t InitCapacity>
int Compiler::DynArray<ArrayType, InitCapacity>::Resize(
    std::size_t new_capacity) {
  if (new_capacity == 0) {
    capacity_ *= 2;
  } else {
    capacity_ = new_capacity;
  }
  ArrayType* new_data = new ArrayType[capacity_];
  if (new_data == nullptr) {
    Debugger error_info(Debugger::Level::ERROR,
                        "Aq::Compiler::DynArray::Resize", "Resize_ResizeError",
                        "New capacity out of memory occurred.", nullptr);
    return -1;
  }
  for (std::size_t i = 0; i < size_ && i < capacity_; i++) {
    new_data[i] = data_[i];
  }
  delete[] data_;
  data_ = new_data;
  return 0;
}
}  // namespace Aq