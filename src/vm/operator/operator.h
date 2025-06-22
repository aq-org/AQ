// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_VM_OPERATOR_OPERATOR_H_
#define AQ_VM_OPERATOR_OPERATOR_H_

#include <functional>
#include <string>

#include "vm/logging/logging.h"
#include "vm/memory/memory.h"

namespace Aq {
namespace Vm {
namespace Bytecode {
struct BytecodeFile;
struct Class;
}  // namespace Bytecode

namespace Operator {
enum class Operator {
  NOP = 0x00,
  LOAD,
  STORE,
  NEW,
  ARRAY,
  PTR,
  ADD,
  SUB,
  MUL,
  DIV,
  REM,
  NEG,
  SHL,
  SHR,
  REFER,
  IF,
  AND,
  OR,
  XOR,
  CMP,
  INVOKE,
  EQUAL,
  GOTO,
  LOAD_CONST,
  CONVERT,
  CONST,
  INVOKE_METHOD,
  LOAD_MEMBER,
  WIDE = 0xFF
};

int NOP();

int LOAD(std::vector<Memory::Object>& heap, std::size_t ptr,
         std::size_t operand);

int STORE(std::vector<Memory::Object>& heap, std::size_t ptr,
          std::size_t operand);

int NEW(std::vector<Memory::Object>& heap,
        std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
        std::string& current_bytecode_file,
        std::unordered_map<std::string, Bytecode::Class> classes,
        bool is_big_endian, std::size_t ptr, std::size_t size, std::size_t type,
        std::shared_ptr<Memory::Memory>& memory,
        std::unordered_map<std::string,
                           std::function<int(std::vector<std::size_t>)>>
            builtin_functions);

int CrossMemoryNew(std::shared_ptr<Memory::Memory> memory,
                   std::unordered_map<std::string, Bytecode::Class> classes,
                   std::size_t ptr, std::size_t size, std::size_t type);

int ARRAY(
    std::vector<Memory::Object>& heap, std::size_t result, std::size_t ptr,
    std::size_t index,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>
        builtin_functions,
    std::string& current_bytecode_file, bool is_big_endian,
    std::shared_ptr<Memory::Memory>& memory);

int PTR(std::vector<Memory::Object>& heap, std::size_t index, std::size_t ptr);

int ADD(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int SUB(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int MUL(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int DIV(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int REM(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);
int NEG(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1);

int SHL(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int SHR(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int REFER(std::vector<Memory::Object>& heap, std::size_t result,
          std::size_t operand1);

size_t IF(std::vector<Memory::Object>& heap, std::size_t condition,
          std::size_t true_branche, std::size_t false_branche);

int AND(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int OR(std::vector<Memory::Object>& heap, std::size_t result,
       std::size_t operand1, std::size_t operand2);

int XOR(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2);

int CMP(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t opcode, std::size_t operand1, std::size_t operand2);

int INVOKE(
    std::vector<Memory::Object>& heap,
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::string& current_bytecode_file, bool is_big_endian,
    std::shared_ptr<Memory::Memory>& memory);
int EQUAL(std::vector<Memory::Object>& heap, std::size_t result,
          std::size_t value);

int CrossMemoryEqual(std::vector<Memory::Object>& result_heap,
                     std::size_t result,
                     std::vector<Memory::Object>& value_heap,
                     std::size_t value);

size_t GOTO(std::vector<Memory::Object>& heap, std::size_t location);
int LOAD_CONST(std::vector<Memory::Object>& heap,
               std::vector<Memory::Object>& constant_pool, std::size_t object,
               std::size_t const_object);

int CONVERT(std::vector<Memory::Object>& heap, std::size_t result,
            std::size_t operand1);
int _CONST(std::vector<Memory::Object>& heap, std::size_t result,
           std::size_t operand1);

int INVOKE_METHOD(
    std::vector<Memory::Object>& heap, std::string& current_bytecode_file,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::shared_ptr<Memory::Memory>& memory,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>&
        builtin_functions,
    bool is_big_endian, std::vector<size_t> arguments);

int LOAD_MEMBER(std::vector<Memory::Object>& heap,
                std::unordered_map<std::string, Bytecode::Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand);

int WIDE();

}  // namespace Operator
}  // namespace Vm
}  // namespace Aq

#endif