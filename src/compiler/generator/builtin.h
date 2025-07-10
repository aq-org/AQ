// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_BUILTIN_H_
#define AQ_COMPILER_GENERATOR_BUILTIN_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "compiler/ast/ast.h"

namespace Aq {
namespace Compiler {
namespace Generator {
struct Generator;

// Adds built-in functions to the generator.
void AddBuiltInFunctionDeclaration(Generator& generator, std::string name);

// Initializes built-in function declarations.
void InitBuiltInFunctionDeclaration(Generator& generator);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif