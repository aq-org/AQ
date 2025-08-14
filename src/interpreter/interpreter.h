// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_INTERPRETER_H_
#define AQ_INTERPRETER_INTERPRETER_H_

#include <functional>
#include <unordered_map>

#include "ast/ast.h"
#include "interpreter/builtin.h"
#include "interpreter/class.h"


namespace Aq {
namespace Interpreter {
struct Interpreter;

struct Context {
  // std::unordered_set<std::string> functions;
  std::unordered_map<std::string, std::size_t> variables;
  // std::unordered_map<std::string, Class*> classes;

  FunctionContext* function_context = nullptr;
  std::vector<std::string> scopes;
  Class* current_class = nullptr;
  std::size_t undefined_count = 0;
};

struct Interpreter {
  Interpreter() {
    InitBuiltInFunctionDeclaration(*this);
    global_memory = new Memory();
  }
  virtual ~Interpreter() = default;

  void Generate(Ast::Compound* statement);

  void Run();

  Class main_class;

  Context context;

  std::unordered_map<std::string, std::vector<Function>> functions;
  std::unordered_map<std::string, Class> classes;
  Memory* global_memory;
  std::vector<Bytecode> init_code;
  std::vector<Bytecode> global_code;
  std::unordered_map<std::string, std::function<int(Memory*,
                                                    std::vector<std::size_t>)>>
      builtin_functions;

  std::size_t current_class_index = 0;
};

}  // namespace Interpreter
}  // namespace Aq

#endif