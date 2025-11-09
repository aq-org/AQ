// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_EXPRESSION_INTERPRETER_H_
#define AQ_INTERPRETER_EXPRESSION_INTERPRETER_H_

#include <cstddef>
#include <vector>

#include "ast/ast.h"
#include "interpreter/bytecode.h"
#include "interpreter/interpreter.h"

namespace Aq {
namespace Interpreter {
// Handles the expression.
std::size_t HandleExpression(Interpreter& interpreter,
                             Ast::Expression* expression,
                             std::vector<Bytecode>& code,
                             std::size_t result_index = 0);

// Handles the unary expression.
std::size_t HandleUnaryExpression(Interpreter& interpreter,
                                  Ast::Unary* expression,
                                  std::vector<Bytecode>& code,
                                  std::size_t result_index = 0);

// Handles the binary expression.
std::size_t HandleBinaryExpression(Interpreter& interpreter,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code,
                                   std::size_t result_index = 0);

// Handles the period expression.
std::size_t HandlePeriodExpression(Interpreter& interpreter,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code,
                                   std::size_t result_index = 0);

// Handles the function invoke.
std::size_t HandleFunctionInvoke(Interpreter& interpreter, Ast::Function* func,
                                 std::vector<Bytecode>& code,
                                 std::size_t result_index = 0);

// Handles the lambda expression.
std::size_t HandleLambdaExpression(Interpreter& interpreter,
                                   Ast::Lambda* lambda,
                                   std::vector<Bytecode>& code);

// Handles the function invoke.
std::size_t AddConstInt8t(Interpreter& interpreter, int8_t value);

// Handles the function return value.
std::size_t HandleFunctionReturnValue(Interpreter& interpreter,
                                      std::vector<Bytecode>& code);

// // Gets the index of the expression in the memory.
std::size_t GetIndex(Interpreter& interpreter, Ast::Expression* expression,
                     std::vector<Bytecode>& code);

// Gets the index of the class in the memory.
std::size_t GetClassIndex(Interpreter& interpreter, Ast::Expression* expression,
                          std::vector<Bytecode>& code);
}  // namespace Interpreter
}  // namespace Aq

#endif