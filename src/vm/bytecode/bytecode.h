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

char* Get1Parament(char* ptr, size_t* first);
char* Get2Parament(char* ptr, size_t* first);
char* Get3Parament(char* ptr, size_t* first);
char* Get4Parament(char* ptr, size_t* first);
std::vector<std::size_t> GetUnknownCountParament(char*& ptr);
std::vector<std::size_t> GetUnknownCountParamentForClass(char*& ptr);
char* AddClassMethod(char* location,
    std::vector<Memory::Object>& heap,
                     std::unordered_map<std::string, Function>& functions);
char* AddClass(char* location, 
               std::unordered_map<std::string, Bytecode::Class>& classes,
               std::shared_ptr<Memory::Memory> memory);
int InvokeClassFunction(
    std::vector<Memory::Object>& heap, size_t class_index,
    std::string function_name, std::vector<size_t> arguments,
    std::string& current_bytecode_file,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::shared_ptr<Memory::Memory>& memory,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>
        builtin_functions,
    bool is_big_endian);
int InvokeCustomFunction(
    std::vector<Memory::Object>& heap, std::string name,
    std::vector<size_t> arguments,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>
        builtin_functions,
    std::string& current_bytecode_file, bool is_big_endian,
    std::shared_ptr<Memory::Memory>& memory);
char* AddBytecodeFileClass(
    std::string prefix_name, std::shared_ptr<Memory::Memory> memory,
    char* location, std::unordered_map<std::string, Bytecode::Class>& classes);
char* HandleBytecodeFile(
    std::string name, char* bytecode_file, size_t size, bool is_big_endian,
    std::unordered_map<std::string, Bytecode::Class>& classes);
void AddBytecodeFile(
    const char* file,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    bool is_big_endian,
    std::unordered_map<std::string, Bytecode::Class>& classes);

}  // namespace Bytecode
}  // namespace Vm
}  // namespace Aq

#endif