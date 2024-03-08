// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_HASH_MAP_HASH_MAP_H_
#define AQ_COMPILER_HASH_MAP_HASH_MAP_H_

#include "compiler/compiler.h"

namespace Aq {
template <typename ValueType>
class Compiler::HashMap {
 public:
  HashMap();
  ~HashMap();

  HashMap(const HashMap&) = default;
  HashMap(HashMap&&) noexcept = default;
  HashMap& operator=(const HashMap&) = default;
  HashMap& operator=(HashMap&&) noexcept = default;

 private:
  // TODO: Hash Map
};
}  // namespace Aq

#endif