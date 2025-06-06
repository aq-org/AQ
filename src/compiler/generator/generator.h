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

  void Generate(Ast::Compound* stmt, const char* output_file);

  bool is_big_endian = false;
  Class main_class;

  Context context;

  std::vector<Function> functions;
  std::vector<Class> classes;
  Memory global_memory;
  std::vector<Bytecode> init_code;
  std::vector<Bytecode> global_code;
};

void HandleStmt(Ast::Statement* stmt, std::vector<Bytecode>& code);
void HandleBreakStmt(std::vector<Bytecode>& code);
void HandleClassStmt(Ast::Statement* stmt, std::vector<Bytecode>& code);
void HandleReturn(Ast::Return* stmt, std::vector<Bytecode>& code);
void HandleCompoundStmt(Ast::Compound* stmt, std::vector<Bytecode>& code);
void HandleIfStmt(Ast::If* stmt, std::vector<Bytecode>& code);
void HandleWhileStmt(Ast::While* stmt, std::vector<Bytecode>& code);
void HandleDowhileStmt(Ast::DoWhile* stmt, std::vector<Bytecode>& code);
void HandleForStmt(Ast::For* stmt, std::vector<Bytecode>& code);
std::size_t HandleExpr(Ast::Expression* expr, std::vector<Bytecode>& code);
std::size_t HandleUnaryExpr(Ast::Unary* expr, std::vector<Bytecode>& code);
std::size_t HandleBinaryExpr(Ast::Binary* expr, std::vector<Bytecode>& code);
std::size_t HandlePeriodExpr(Ast::Binary* expr, std::vector<Bytecode>& code);
std::size_t HandleFuncInvoke(Ast::Function* func, std::vector<Bytecode>& code);
std::size_t HandleClassFuncInvoke(Ast::Function* func,
                                  std::vector<Bytecode>& code);
void HandleLabel(Ast::Label* label, std::vector<Bytecode>& code);
void HandleGoto(Ast::Goto* label, std::vector<Bytecode>& code);
void HandleStartGoto(Ast::Goto* label, std::vector<Bytecode>& code);
std::size_t GetIndex(Ast::Expression* expr, std::vector<Bytecode>& code);
std::size_t GetClassIndex(Ast::Expression* expr, std::vector<Bytecode>& code);
std::size_t AddConstInt8t(int8_t value);
int64_t SwapLong(int64_t x);
double SwapDouble(double x);
uint64_t SwapUint64t(uint64_t x);
void InsertUint64ToCode(uint64_t value);
static std::size_t EncodeUleb128(std::size_t value,
                                 std::vector<uint8_t>& output);
void GenerateBytecodeFile(const char* output_file);
void GenerateMnemonicFile(const char* output_file_name);
Ast::Type* GetExprType(Ast::Expression* expr);
std::string GetExprTypeString(Ast::Expression* expr);
bool IsDereferenced(Ast::Expression* expr);
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif