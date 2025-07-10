// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_EXPRESSION_GENERATOR_H_
#define AQ_COMPILER_GENERATOR_EXPRESSION_GENERATOR_H_

#include <cstddef>
#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/generator/bytecode.h"
#include "compiler/generator/generator.h"

namespace Aq {
namespace Compiler {
namespace Generator {
// Handles the expression.
std::size_t HandleExpression(Generator& generator, Ast::Expression* expression,
                             std::vector<Bytecode>& code);

// Handles the unary expression.
std::size_t HandleUnaryExpression(Generator& generator, Ast::Unary* expression,
                                  std::vector<Bytecode>& code);

// Handles the binary expression.
std::size_t HandleBinaryExpression(Generator& generator,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code);

// Handles the period expression.
std::size_t HandlePeriodExpression(Generator& generator,
                                   Ast::Binary* expression,
                                   std::vector<Bytecode>& code);

// Handles the function invoke.
std::size_t HandleFunctionInvoke(Generator& generator, Ast::Function* func,
                                 std::vector<Bytecode>& code);

// Handles the function invoke.
std::size_t AddConstInt8t(Generator& generator, int8_t value);

// Handles the function return value.
std::size_t HandleFunctionReturnValue(Generator& generator,
                                      std::vector<Bytecode>& code);

// // Gets the index of the expression in the memory.
std::size_t GetIndex(Generator& generator, Ast::Expression* expression,
                     std::vector<Bytecode>& code);

// Gets the index of the class in the memory.
std::size_t GetClassIndex(Generator& generator, Ast::Expression* expression,
                          std::vector<Bytecode>& code);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif