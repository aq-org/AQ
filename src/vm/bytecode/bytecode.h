// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_VM_BYTECODE_BYTECODE_H_
#define AQ_VM_BYTECODE_BYTECODE_H_

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "vm/memory/memory.h"
#include "vm/operator/operator.h"

namespace Aq {
namespace Vm {
namespace Bytecode {
struct Instruction {
  Operator::Operator oper;
  std::vector<std::size_t> arguments;
};

struct Function {
  std::string name;
  std::vector<std::size_t> arguments;
  bool is_variadic;
  std::vector<Instruction> instructions;
};

struct Class {
  std::string name;
  std::vector<Memory::Object> members;
  std::unordered_map<std::string, std::size_t> variables;
  std::unordered_map<std::string, Function> functions;
  std::unordered_map<std::string, std::size_t> bytecodes;
  std::shared_ptr<Memory::Memory> memory;
};

struct BytecodeFile {
  std::string name;
  std::shared_ptr<Memory::Object> object;
  int index;
};

}  // namespace Bytecode
}  // namespace Vm
}  // namespace Aq

#endif