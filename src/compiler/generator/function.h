// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_FUNCTION_H_
#define AQ_COMPILER_GENERATOR_FUNCTION_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "compiler/generator/bytecode.h"
#include "compiler/generator/operator.h"

namespace Aq {
namespace Compiler {
namespace Generator {
class Function {
 public:
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
  std::vector<std::size_t> exit_index;
  std::vector<int64_t> loop_break_index;
};
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif