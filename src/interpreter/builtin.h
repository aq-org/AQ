// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_BUILTIN_H_
#define AQ_INTERPRETER_BUILTIN_H_

#include <functional>
#include <string>
#include <vector>

#include "interpreter/memory.h"

namespace Aq {
namespace Interpreter {
struct Interpreter;

// Adds built-in functions to the interpreter.
void AddBuiltInFunctionDeclaration(
    Interpreter& interpreter, std::string name,
    std::function<int(Memory*, std::vector<std::size_t>)> function);

// Initializes built-in function declarations.
void InitBuiltInFunctionDeclaration(Interpreter& interpreter);

int __builtin_void(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_print(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_vaprint(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_abs(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_open(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_close(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_read(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_write(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_input(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_GUI_CreateWindow(Memory* memory,
                               std::vector<std::size_t> arguments);

int __builtin_math_acos(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_asin(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_atan(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_atan2(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_ceil(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_cos(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_cosh(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_exp(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_fabs(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_floor(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_fmod(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_frexp(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_hypot(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_ldexp(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_log(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_log10(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_modf(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_pow(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_sin(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_sinh(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_sqrt(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_tan(Memory* memory, std::vector<std::size_t> arguments);

int __builtin_math_tanh(Memory* memory, std::vector<std::size_t> arguments);

}  // namespace Interpreter
}  // namespace Aq

#endif