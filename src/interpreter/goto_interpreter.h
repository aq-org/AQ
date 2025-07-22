// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_GENERATOR_GOTO_GENERATOR_H_
#define AQ_GENERATOR_GOTO_GENERATOR_H_

#include <vector>

#include "ast/ast.h"
#include "interpreter/bytecode.h"
#include "interpreter/interpreter.h"

namespace Aq {
namespace Interpreter {
// Handles the label.
void HandleLabel(Interpreter& interpreter, Ast::Label* label,
                 std::vector<Bytecode>& code);

// Handles the goto.
void HandleGoto(Interpreter& interpreter, Ast::Goto* label,
                std::vector<Bytecode>& code);
}  // namespace Interpreter
}  // namespace Aq

#endif