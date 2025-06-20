// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/builtin/builtin.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Aq {
namespace Vm {
namespace Builtin {
void InitializeBuiltinFunction(
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>&
        builtin_functions) {
  // TODO: Implement the initialization of built-in functions.
}
}  // namespace Builtin
}  // namespace Vm
}  // namespace Aq