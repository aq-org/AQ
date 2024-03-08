// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_
#define AQ_COMPILER_DYN_ARRAY_DYN_ARRAY_H_

#include <cstddef>

#include "compiler/compiler.h"

namespace Aq {
template <typename ArrayType,std::size_t InitCapacity>
class Compiler::DynArray {
 public:
  DynArray();
  ~DynArray();

  DynArray(const DynArray&) = default;
  DynArray(DynArray&&) noexcept = default;
  DynArray& operator=(const DynArray&) = default;
  DynArray& operator=(DynArray&&) noexcept = default;

  // TODO: Waiting for developing.

 private:
  ArrayType* array_;
  std::size_t capacity_;
  std::size_t size_;
};
}  // namespace Aq

#endif