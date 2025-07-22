// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_GENERATOR_BUILTIN_H_
#define AQ_GENERATOR_BUILTIN_H_

#include <string>

namespace Aq {
namespace Generator {
struct Generator;

// Adds built-in functions to the generator.
void AddBuiltInFunctionDeclaration(Generator& generator, std::string name);

// Initializes built-in function declarations.
void InitBuiltInFunctionDeclaration(Generator& generator);
}  // namespace Generator
}  // namespace Aq

#endif