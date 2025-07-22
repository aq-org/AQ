// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "generator/goto_generator.h"

#include "ast/ast.h"
#include "generator/bytecode.h"
#include "generator/generator.h"
#include "logging/logging.h"

namespace Aq {
namespace Generator {
void HandleLabel(Generator& generator, Ast::Label* label,
                 std::vector<Bytecode>& code) {
  if (label == nullptr) INTERNAL_ERROR("label is nullptr.");

  // Gets the reference of context.
  auto& scopes = generator.context.scopes;
  auto& label_map = generator.context.function_context->label_map;

  std::string label_name = scopes.back() + "$" + std::string(label->GetLabel());

  if (label_map.find(label_name) != label_map.end())
    LOGGING_WARNING("Has found same name label.");

  label_map[label_name] = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
}

void HandleGoto(Generator& generator, Ast::Goto* label,
                std::vector<Bytecode>& code) {
  if (label == nullptr) LOGGING_ERROR("label is nullptr.");

  // Gets the reference of context.
  auto& scopes = generator.context.scopes;
  auto& label_map = generator.context.function_context->label_map;
  auto& current_scope = generator.context.function_context->current_scope;
  auto& goto_map = generator.context.function_context->goto_map;
  auto& global_memory = generator.global_memory;

  std::string label_name = std::string(label->GetLabel());

  for (int64_t i = scopes.size() - 1; i >= current_scope; i--) {
    auto iterator = label_map.find(scopes[i] + "$" + label_name);

    // If the label is found in the current scope,
    // it will be replaced with the address of the label.
    if (iterator != label_map.end()) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                              global_memory.AddUint64t(iterator->second)));
      return;
    }

    // If the label is not found in the current scope,
    // it will be added to the goto map for later processing.
    if (i == current_scope) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
      goto_map.push_back(
          std::pair<std::string, std::size_t>(label_name, code.size() - 1));
      return;
    }
  }
}

}  // namespace Generator
}  // namespace Aq