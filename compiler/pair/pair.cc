/// Copyright 2024 AQ authors, All Rights Reserved.
/// This program is licensed under the AQ License. You can find the AQ license
/// in the root directory.

#include "compiler/pair/pair.h"

namespace Aq {
template <typename T1, typename T2>
Compiler::Pair<T1, T2>::Pair(T1 _first, T2 _second)
    : first(first), second(second) {}
}  // namespace Aq