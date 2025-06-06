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
// Preprocesses the declaration statements in the given compound statement.
// This function processes class declarations, function declarations, and
// static declarations, and imports. It updates the current scope and
// declaration maps accordingly.
void PreProcessDeclaration(
    Ast::Compound* statements, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations,
    std::unordered_map<std::string, std::size_t>& variable_declaration_map,
    std::unordered_map<std::string, Generator::Class*>& class_declaration_map,
    Memory& global_memory);

// Preprocesses the function declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessFunctionDeclaration(
    Ast::FunctionDeclaration* statement, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations);

// Preprocesses the class declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessClassDeclaration(
    Ast::Class* statement, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations,
    std::unordered_map<std::string, Generator::Class*>& class_declaration_map,
    Memory& global_memory);

// Preprocesses the static declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessStaticDeclaration(
    Ast::Class* statement, std::vector<std::string>& scopes,
    std::unordered_set<std::string>& function_declarations,
    std::unordered_map<std::string, Generator::Class*>& class_declaration_map,
    Memory& global_memory);

// Preprocesses the import statement in the given statement. It updates
// the global memory with the imported variables.
void PreProcessImport(
    Ast::Import* statement,
    std::unordered_map<std::string, std::size_t>& variable_declaration_map,
    Memory& global_memory);

}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif
