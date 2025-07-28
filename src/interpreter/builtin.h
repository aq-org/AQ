// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_BUILTIN_H_
#define AQ_INTERPRETER_BUILTIN_H_

#include <string>

namespace Aq {
namespace Interpreter {
struct Interpreter;

// Adds built-in functions to the interpreter.
void AddBuiltInFunctionDeclaration(Interpreter& interpreter, std::string name);

// Initializes built-in function declarations.
void InitBuiltInFunctionDeclaration(Interpreter& interpreter);
}  // namespace Interpreter
}  // namespace Aq

#endif