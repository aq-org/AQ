// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_PREPROCESSER_H_
#define AQ_INTERPRETER_PREPROCESSER_H_

#include "ast/ast.h"

namespace Aq {
namespace Interpreter {
struct Interpreter;

// Preprocesses the declaration statements in the given compound statement.
// This function processes class declarations, function declarations, and
// static declarations, and imports. It updates the current scope and
// declaration maps accordingly.
void PreProcessDeclaration(Interpreter& interpreter, Ast::Compound* statements);

// Preprocesses the function declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessFunctionDeclaration(Interpreter& interpreter,
                                   Ast::FunctionDeclaration* statement);

// Preprocesses the class declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessClassDeclaration(Interpreter& interpreter, Ast::Class* statement);

// Preprocesses the static declaration in the given statement. It updates
// the current scope and declaration maps accordingly.
void PreProcessStaticDeclaration(Interpreter& interpreter, Ast::Class* statement);

// Preprocesses the import statement in the given statement. It updates
// the global memory with the imported variables.
void PreProcessImport(Interpreter& interpreter, Ast::Import* statement);

}  // namespace Interpreter
}  // namespace Aq

#endif
