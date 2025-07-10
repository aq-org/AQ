// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_STATEMENT_GENERATOR_H_
#define AQ_COMPILER_GENERATOR_STATEMENT_GENERATOR_H_

#include "compiler/ast/ast.h"
#include "compiler/generator/generator.h"

namespace Aq {
namespace Compiler {
namespace Generator {
// Handles the statement, except for function declaration and class declaration.
void HandleStatement(Generator& generator, Ast::Statement* statement,
                     std::vector<Bytecode>& code);

// Handles the break statement.
void HandleBreakStatement(Generator& generator, std::vector<Bytecode>& code);

// Handles the statement in the class function body.
void HandleClassStatement(Generator& generator, Ast::Statement* statement,
                          std::vector<Bytecode>& code);

// Handles the return statement in the function body.
void HandleReturnStatement(Generator& generator, Ast::Return* statement,
                           std::vector<Bytecode>& code);

// Handles the compound statement in the function body.
void HandleCompoundStatement(Generator& generator, Ast::Compound* statement,
                             std::vector<Bytecode>& code);

// Handles the function invoke statement.
void HandleIfStatement(Generator& generator, Ast::If* statement,
                       std::vector<Bytecode>& code);

// Handles the while statement.
void HandleWhileStatement(Generator& generator, Ast::While* statement,
                          std::vector<Bytecode>& code);

// Handles the do-while statement.
void HandleDowhileStatement(Generator& generator, Ast::DoWhile* statement,
                            std::vector<Bytecode>& code);

// Handles the for statement.
void HandleForStatement(Generator& generator, Ast::For* statement,
                        std::vector<Bytecode>& code);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif