// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_PAIR_PAIR_H_
#define AQ_COMPILER_PAIR_PAIR_H_

#include "compiler/compiler.h"

namespace Aq {
template <typename T1, typename T2>
struct Compiler::Pair {
  T1 first;
  T2 second;

  Pair(T1 _first, T2 _second);
};
}  // namespace Aq

#endif