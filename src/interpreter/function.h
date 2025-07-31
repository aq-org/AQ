// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_FUNCTION_H_
#define AQ_INTERPRETER_FUNCTION_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "interpreter/bytecode.h"

namespace Aq {
namespace Interpreter {
class Function {
 public:
  Function() = default;
  Function(std::string name, std::vector<std::size_t> parameters,
           std::vector<Bytecode> code) {
    name_ = name;
    parameters_ = parameters;
    code_ = code;
  }
  ~Function() = default;

  std::string GetName() { return name_; }

  std::vector<std::size_t> GetParameters() { return parameters_; }

  std::vector<Bytecode> GetCode() { return code_; }

  void EnableVariadic() { is_variadic_ = true; }

  bool IsVariadic() { return is_variadic_; }

 private:
  std::string name_;
  std::vector<std::size_t> parameters_;
  std::vector<Bytecode> code_;
  bool is_variadic_ = false;
};

struct FunctionContext {
  std::vector<std::pair<std::string, std::size_t>> goto_map;
  std::unordered_map<std::string, std::size_t> label_map;
  std::size_t current_scope = 0;
  std::vector<std::size_t> exit_index;
  std::vector<int64_t> loop_break_index;
};
}  // namespace Interpreter
}  // namespace Aq

#endif