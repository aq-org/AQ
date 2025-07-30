// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/operator.h"

namespace Aq {
namespace Interpreter {

int NOP() { return 0; }

[[deprecated]] int LOAD(Memory memory, std::size_t ptr, std::size_t operand) {
  return 0;
}

[[deprecated]] int STORE(Memory memory, std::size_t ptr, std::size_t operand) {
  return 0;
}

int NEW(Memory memory, std::string& current_bytecode_file,
        std::unordered_map<std::string, Class> classes, std::size_t ptr,
        std::size_t size, std::size_t type,
        std::unordered_map<
            std::string, std::function<int(Memory, std::vector<std::size_t>)>>&
            builtin_functions) {}

int CrossMemoryNew(Memory memory,
                   std::unordered_map<std::string, Class> classes,
                   std::size_t ptr, std::size_t size, std::size_t type) {}

int ARRAY(
    Memory memory, std::size_t result, std::size_t ptr, std::size_t index,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory, std::vector<std::size_t>)>>&
        builtin_functions,
    std::string& current_bytecode_file) {}

[[deprecated]] int PTR(Memory memory, std::size_t index, std::size_t ptr) {
  return 0;
}

int ADD(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int SUB(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int MUL(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int DIV(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int REM(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}
int NEG(Memory memory, std::size_t result, std::size_t operand1) {}

int SHL(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int SHR(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int REFER(Memory memory, std::size_t result, std::size_t operand1) {}

size_t IF(Memory memory, std::size_t condition, std::size_t true_branche,
          std::size_t false_branche) {}

int AND(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int OR(Memory memory, std::size_t result, std::size_t operand1,
       std::size_t operand2) {}

int XOR(Memory memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {}

int CMP(Memory memory, std::size_t result, std::size_t opcode,
        std::size_t operand1, std::size_t operand2) {}

int INVOKE(
    Memory memory,
    std::unordered_map<std::string,
                       std::function<int(Memory, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Class>& classes,
    std::string& current_bytecode_file) {}
int EQUAL(Memory memory, std::size_t result, std::size_t value) {}

int CrossMemoryEqual(Memory result_heap, std::size_t result, Memory value_heap,
                     std::size_t value) {}

size_t GOTO(Memory memory, std::size_t location) {}
int LOAD_CONST(Memory memory, Memory constant_pool, std::size_t object,
               std::size_t const_object) {}

[[deprecated]] int CONVERT(Memory memory, std::size_t result,
                           std::size_t operand1) {
  return 0;
}
[[deprecated]] int CONST(Memory memory, std::size_t result,
                         std::size_t operand1) {
  return 0;
}

int INVOKE_METHOD(
    Memory memory, std::string& current_bytecode_file,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<size_t> arguments) {}

int LOAD_MEMBER(Memory memory, std::unordered_map<std::string, Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand) {}

int WIDE() {}
}  // namespace Interpreter
}  // namespace Aq