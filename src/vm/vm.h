// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_VM_VM_H_
#define AQ_VM_VM_H_

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "vm/bytecode/bytecode.h"
#include "vm/memory/memory.h"

namespace Aq {
namespace Vm {

// Reads the code from a file and stores it in a vector.
void ReadCodeFromFile(const char* filename, std::vector<char>& code);

bool CheckBytecode(std::vector<char>& code);

class Vm {
 public:
  Vm() = default;
  ~Vm() = default;

  void Initialize(std::vector<char>& code);

 private:
  bool is_big_endian_ = false;

  std::string current_bytecode_file_;
  std::shared_ptr<Memory::Memory> memory_;
  std::unordered_map<std::string, std::function<int(std::vector<std::size_t>)>>
      builtin_functions_;
  std::unordered_map<std::string, Bytecode::BytecodeFile> bytecode_files_;
  std::unordered_map<std::string, Bytecode::Class> classes_;
};
}  // namespace Vm
}  // namespace Aq

#endif