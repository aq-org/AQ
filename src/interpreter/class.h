// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_CLASS_H_
#define AQ_INTERPRETER_CLASS_H_

#include <unordered_map>

#include "ast/ast.h"
#include "interpreter/bytecode.h"
#include "interpreter/function.h"
#include "interpreter/memory.h"
#include "logging/logging.h"

namespace Aq {
namespace Interpreter {
class Class {
 public:
  Class() {
    members_ = std::make_shared<ClassMemory>();
    class_ = nullptr;
  }
  ~Class() = default;

  void SetClass(Ast::Class* class_declaration) {
    if (class_declaration == nullptr)
      INTERNAL_ERROR("class_declaration is nullptr.");
    class_ = class_declaration;
  }

  bool GetVariable(std::string name, Object& object) {
    if (members_->GetMembers().find(name) == members_->GetMembers().end())
      return false;

    object = members_->GetMembers()[name];
    return true;
  }

  std::unordered_map<std::string, std::vector<Function>>& GetMethods() {
    return methods_;
  }

  std::shared_ptr<ClassMemory> GetMembers() { return members_; }

  std::vector<Bytecode>& GetCode() { return code_; }

  Ast::Class* GetClassDeclaration() { return class_; }

  void SetName(std::string name) { name_ = name; }

  std::string GetName() { return name_; }

 private:
  std::string name_;
  Ast::Class* class_;
  std::unordered_map<std::string, std::vector<Function>> methods_;
  std::shared_ptr<ClassMemory> members_;
  std::vector<Bytecode> code_;
  std::size_t name_index_ = 0;
};
}  // namespace Interpreter
}  // namespace Aq

#endif