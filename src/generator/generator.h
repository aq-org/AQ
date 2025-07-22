// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_GENERATOR_GENERATOR_H_
#define AQ_GENERATOR_GENERATOR_H_

#include <cstdarg>
#include <unordered_map>
#include <cstdint>

#include "ast/ast.h"
#include "generator/builtin.h"
#include "generator/class.h"
#include "generator/operator.h"

namespace Aq {
namespace Generator {
struct Generator;

struct Context {
  std::unordered_set<std::string> functions;
  std::unordered_map<std::string, std::size_t> variables;
  std::unordered_map<std::string, Class*> classes;

  FunctionContext* function_context = nullptr;
  std::vector<std::string> scopes;
  Class* current_class = nullptr;
  std::size_t undefined_count = 0;
};

struct Generator {
  Generator() {
    InitBuiltInFunctionDeclaration(*this);
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

void InsertUint64ToCode(uint64_t value, std::vector<uint8_t>& code);
void GenerateBytecodeFile(Generator& generator, const char* output_file);
void GenerateMnemonicFile(Generator& generator, const char* output_file_name);

}  // namespace Generator
}  // namespace Aq

#endif