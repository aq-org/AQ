// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_VM_BUILTIN_BUILTIN_H_
#define AQ_VM_BUILTIN_BUILTIN_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "vm/memory/memory.h"

namespace Aq {
namespace Vm {
namespace Builtin {
void InitializeBuiltinFunction(
    std::unordered_map<std::string,
                       std::function<int(std::vector<Memory::Object>&,std::vector<std::size_t>)>>&
        builtin_functions);

int print(std::vector<Memory::Object>& heap,std::vector<std::size_t> arguments);
}
}  // namespace Vm
}  // namespace Aq

#endif