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
void HandleStatement(Generator& generator, Ast::Statement* statement,
                     std::vector<Bytecode>& code);
void HandleBreakStatement(Generator& generator, std::vector<Bytecode>& code);
void HandleClassStatement(Generator& generator, Ast::Statement* statement,
                          std::vector<Bytecode>& code);
void HandleReturnStatement(Generator& generator, Ast::Return* statement,
                           std::vector<Bytecode>& code);
void HandleCompoundStatement(Generator& generator, Ast::Compound* statement,
                             std::vector<Bytecode>& code);
void HandleIfStatement(Generator& generator, Ast::If* statement,
                       std::vector<Bytecode>& code);
void HandleWhileStatement(Generator& generator, Ast::While* statement,
                          std::vector<Bytecode>& code);
void HandleDowhileStatement(Generator& generator, Ast::DoWhile* statement,
                            std::vector<Bytecode>& code);
void HandleForStatement(Generator& generator, Ast::For* statement,
                        std::vector<Bytecode>& code);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif