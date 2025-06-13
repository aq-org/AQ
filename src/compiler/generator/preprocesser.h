// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_PREPROCESSER_H_
#define AQ_COMPILER_GENERATOR_PREPROCESSER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/generator/class.h"
#include "compiler/generator/memory.h"

namespace Aq {
namespace Compiler {
namespace Generator {
class Generator;

// Preprocesses the declaration statements in the given compound statement.
// This function processes class declarations, function declarations, and
// static declarations, and imports. It updates the current scope and
// declaration maps accordingly.
void PreProcessDeclaration(Generator& generator, Ast::Compound* statements);

// Preprocesses the function declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessFunctionDeclaration(Generator& generator,
                                   Ast::FunctionDeclaration* statement);

// Preprocesses the class declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessClassDeclaration(Generator& generator, Ast::Class* statement);

// Preprocesses the static declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessStaticDeclaration(Generator& generator, Ast::Class* statement);

// Preprocesses the import statement in the given statement. It updates
// the global memory with the imported variables.
void PreProcessImport(Generator& generator, Ast::Import* statement);

}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif
