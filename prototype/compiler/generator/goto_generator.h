// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_GOTO_GENERATOR_H_
#define AQ_COMPILER_GENERATOR_GOTO_GENERATOR_H_

#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/generator/bytecode.h"
#include "compiler/generator/generator.h"

namespace Aq {
namespace Compiler {
namespace Generator {
// Handles the label.
void HandleLabel(Generator& generator, Ast::Label* label,
                 std::vector<Bytecode>& code);

// Handles the goto.
void HandleGoto(Generator& generator, Ast::Goto* label,
                std::vector<Bytecode>& code);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif