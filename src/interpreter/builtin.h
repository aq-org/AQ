// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_BUILTIN_H_
#define AQ_INTERPRETER_BUILTIN_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "interpreter/memory.h"

namespace Aq {
namespace Interpreter {
struct Interpreter;

// Adds built-in functions to the interpreter.
void AddBuiltInFunctionDeclaration(
    Interpreter& interpreter, std::string name,
    std::function<int(Memory*, std::vector<std::size_t>)>
        function);

// Initializes built-in function declarations.
void InitBuiltInFunctionDeclaration(Interpreter& interpreter);

int __builtin_void(Memory* memory,
                   std::vector<std::size_t> arguments);

int __builtin_print(Memory* memory,
                    std::vector<std::size_t> arguments);

int __builtin_vaprint(Memory* memory,
                      std::vector<std::size_t> arguments);
}  // namespace Interpreter
}  // namespace Aq

#endif