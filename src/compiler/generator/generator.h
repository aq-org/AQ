// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_GENERATOR_H_
#define AQ_COMPILER_GENERATOR_GENERATOR_H_

#include <cstdarg>
#include <unordered_map>

#include "compiler/ast/ast.h"
#include "compiler/generator/builtin.h"
#include "compiler/generator/class.h"
#include "compiler/generator/operator.h"

namespace Aq {
namespace Compiler {
namespace Generator {
struct Generator;

struct Context {
  std::unordered_set<std::string> functions;
  std::unordered_map<std::string, std::size_t> variables;
  std::unordered_map<std::string, Class*> classes;

  FunctionContext* function_context = nullptr;
  std::vector<std::string> scopes;
  Class* current_class = nullptr;
  std::vector<std::pair<std::string, std::size_t>> main_goto_map;
  std::size_t undefined_count = 0;
};

struct Generator {
  Generator() {
    InitBuiltInFuncDecl();
    uint16_t test_data = 0x0011;
    is_big_endian = *(uint8_t*)&test_data == 0x00;
  }
  virtual ~Generator() = default;

  void Generate(Ast::Compound* statement, const char* output_file);

  bool is_big_endian = false;
  Class main_class;

  Context context;

  std::vector<Function> functions;
  std::vector<Class> classes;
  Memory global_memory;
  std::vector<Bytecode> init_code;
  std::vector<Bytecode> global_code;
};


std::size_t HandleExpression(Generator& generator, Ast::Expression* expression,
                       std::vector<Bytecode>& code);
std::size_t HandleUnaryExpression(Generator& generator, Ast::Unary* expression,
                            std::vector<Bytecode>& code);
std::size_t HandleBinaryExpression(Generator& generator, Ast::Binary* expression,
                             std::vector<Bytecode>& code);
std::size_t HandlePeriodExpression(Generator& generator, Ast::Binary* expression,
                             std::vector<Bytecode>& code);
std::size_t HandleFuncInvoke(Generator& generator, Ast::Function* func,
                             std::vector<Bytecode>& code);
std::size_t HandleClassFuncInvoke(Generator& generator, Ast::Function* func,
                                  std::vector<Bytecode>& code);
void HandleLabel(Generator& generator, Ast::Label* label,
                 std::vector<Bytecode>& code);
void HandleGoto(Generator& generator, Ast::Goto* label,
                std::vector<Bytecode>& code);
void HandleStartGoto(Generator& generator, Ast::Goto* label,
                     std::vector<Bytecode>& code);
std::size_t GetIndex(Generator& generator, Ast::Expression* expression,
                     std::vector<Bytecode>& code);
std::size_t GetClassIndex(Generator& generator, Ast::Expression* expression,
                          std::vector<Bytecode>& code);
std::size_t AddConstInt8t(Generator& generator, int8_t value);
int64_t SwapLong(Generator& generator, int64_t x);
double SwapDouble(Generator& generator, double x);
uint64_t SwapUint64t(Generator& generator, uint64_t x);
void InsertUint64ToCode(Generator& generator, uint64_t value);
static std::size_t EncodeUleb128(Generator& generator, std::size_t value,
                                 std::vector<uint8_t>& output);
void GenerateBytecodeFile(Generator& generator, const char* output_file);
void GenerateMnemonicFile(Generator& generator, const char* output_file_name);
Ast::Type* GetExprType(Generator& generator, Ast::Expression* expression);
std::string GetExprTypeString(Generator& generator, Ast::Expression* expression);
bool IsDereferenced(Generator& generator, Ast::Expression* expression);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif