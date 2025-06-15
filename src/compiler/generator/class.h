// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_CLASS_H_
#define AQ_COMPILER_GENERATOR_CLASS_H_

#include <unordered_map>
#include <unordered_set>
#include <cstdint>

#include "compiler/ast/ast.h"
#include "compiler/generator/bytecode.h"
#include "compiler/generator/function.h"
#include "compiler/generator/memory.h"
#include "compiler/logging/logging.h"

namespace Aq {
namespace Compiler {
namespace Generator {
class Class {
 public:
  Class() = default;
  ~Class() = default;

  void SetClass(Ast::Class* class_declaration) {
    if (class_declaration == nullptr)
      INTERNAL_ERROR("class_declaration is nullptr.");
    class_ = class_declaration;
  }

  bool GetVariable(std::string name, std::size_t& index) {
    if (variables_.find(name) == variables_.end()) return false;

    index = variables_[name];
    return true;
  }

  std::unordered_set<std::string>& GetFunctions() { return functions_; }

  std::vector<Function>& GetFunctionList() { return function_list_; }

  ClassMemory& GetMemory() { return memory_; }

  std::vector<Bytecode>& GetCode() { return code_; }

  std::unordered_map<std::string, std::size_t>& GetVariables() {
    return variables_;
  }

  Ast::Class* GetClassDeclaration() { return class_; }

  void SetName(std::string name) { name_ = name; }

  std::string GetName() { return name_; }

 private:
  std::string name_;
  Ast::Class* class_;
  std::unordered_set<std::string> functions_;
  std::unordered_map<std::string, std::size_t> variables_;
  std::vector<Function> function_list_;
  ClassMemory memory_;
  std::vector<Bytecode> code_;
  std::size_t name_index_ = 0;
};
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif