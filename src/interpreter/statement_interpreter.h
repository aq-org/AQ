// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_GENERATOR_STATEMENT_GENERATOR_H_
#define AQ_GENERATOR_STATEMENT_GENERATOR_H_

#include "ast/ast.h"
#include "interpreter/interpreter.h"

namespace Aq {
namespace Interpreter {
// Handles the statement, except for function declaration and class declaration.
void HandleStatement(Interpreter& interpreter, Ast::Statement* statement,
                     std::vector<Bytecode>& code);

// Handles the break statement.
void HandleBreakStatement(Interpreter& interpreter, std::vector<Bytecode>& code);

// Handles the statement in the class function body.
void HandleClassStatement(Interpreter& interpreter, Ast::Statement* statement,
                          std::vector<Bytecode>& code);

// Handles the return statement in the function body.
void HandleReturnStatement(Interpreter& interpreter, Ast::Return* statement,
                           std::vector<Bytecode>& code);

// Handles the compound statement in the function body.
void HandleCompoundStatement(Interpreter& interpreter, Ast::Compound* statement,
                             std::vector<Bytecode>& code);

// Handles the function invoke statement.
void HandleIfStatement(Interpreter& interpreter, Ast::If* statement,
                       std::vector<Bytecode>& code);

// Handles the while statement.
void HandleWhileStatement(Interpreter& interpreter, Ast::While* statement,
                          std::vector<Bytecode>& code);

// Handles the do-while statement.
void HandleDowhileStatement(Interpreter& interpreter, Ast::DoWhile* statement,
                            std::vector<Bytecode>& code);

// Handles the for statement.
void HandleForStatement(Interpreter& interpreter, Ast::For* statement,
                        std::vector<Bytecode>& code);
}  // namespace Interpreter
}  // namespace Aq

#endif