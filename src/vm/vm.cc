// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/vm.h"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "vm/builtin/builtin.h"
#include "vm/bytecode/bytecode.h"
#include "vm/logging/logging.h"
#include "vm/utils/utils.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    Aq::Vm::LOGGING_ERROR("Usage: " + std::string(argv[0]) + " <filename>");
    return -1;
  }

  std::vector<char> code;
  Aq::Vm::ReadCodeFromFile(argv[1], code);

  Aq::Vm::CheckBytecode(code);

  Aq::Vm::Vm vm;
  vm.Initialize(code);

  return 0;
}

namespace Aq {
namespace Vm {
void ReadCodeFromFile(const char* filename, std::vector<char>& code) {
  std::ifstream ifs(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if (!ifs.is_open()) {
    LOGGING_ERROR("Error: Could not open file " + std::string(filename));
  }

  std::streampos size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  code.resize(size);
  ifs.read(code.data(), size);
  code.push_back('\0');
  ifs.close();
}

bool CheckBytecode(std::vector<char>& code) {
  // Checks bytecode magic number.
  if (code[0] != 0x41 || code[1] != 0x51 || code[2] != 0x42 ||
      code[3] != 0x43) {
    Aq::Vm::LOGGING_ERROR("Invalid bytecode file.");
    return false;
  }

  // Checks bytecode version.
  if (code[4] != 0x00 || code[5] != 0x00 || code[6] != 0x00 ||
      code[7] != 0x03) {
    Aq::Vm::LOGGING_ERROR(
        "This bytecode version is not supported, please check for updates.");
    return false;
  }

  return true;
}

void Vm::Initialize(std::vector<char>& code) {
  uint16_t test_data = 0x0011;
  is_big_endian_ = *(uint8_t*)&test_data == 0x00;

  // Skips the magic number and version.
  char* bytecode_file = code.data() + 8;

  memory_ = std::make_shared<Memory::Memory>();

  // Gets the bytecode constant pool size.
  std::size_t constant_pool_size = 0;
  bytecode_file += DecodeUleb128((uint8_t*)bytecode_file, &constant_pool_size);

  // Reads the constant pool data.
  for (size_t i = 0; i < constant_pool_size; i++) {
    memory_->constant_pool[i].type.push_back(*bytecode_file);
    bytecode_file += 1;

    switch (memory_->constant_pool[i].type[0]) {
      case 0x01:
        memory_->constant_pool[i].data = *(int8_t*)bytecode_file;
        bytecode_file += 1;
        break;

      case 0x02:
        memory_->constant_pool[i].data =
            is_big_endian_ ? *(int64_t*)bytecode_file
                           : SwapLong(*(int64_t*)bytecode_file);
        bytecode_file += 8;
        break;

      case 0x03:
        memory_->constant_pool[i].data =
            is_big_endian_ ? *(double*)bytecode_file
                           : SwapDouble(*(double*)bytecode_file);
        bytecode_file += 8;
        break;

      case 0x04:
        memory_->constant_pool[i].data =
            is_big_endian_ ? *(uint64_t*)bytecode_file
                           : SwapUint64t(*(uint64_t*)bytecode_file);
        bytecode_file += 8;
        break;

      case 0x05: {
        size_t str_size = 0;
        bytecode_file += DecodeUleb128((uint8_t*)bytecode_file, &str_size);
        memory_->constant_pool[i].data = std::string(bytecode_file);
        bytecode_file += str_size;
        break;
      }

      default:
        Aq::Vm::LOGGING_ERROR("Unknown const data type.");
        break;
    }
  }

  // Gets the object table size.
  std::size_t heap_size = 0;
  bytecode_file += DecodeUleb128((uint8_t*)bytecode_file, &heap_size);

  // Reads the heap type.
  for (size_t i = 0; i < heap_size; i++) {
    memory_->heap[i].type.push_back(*bytecode_file);
    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)bytecode_file) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x09:
          is_type_end = true;
          break;

        case 0x06:
        case 0x07:
        case 0x08:
          bytecode_file = bytecode_file += 1;
          break;

        default:
          Aq::Vm::LOGGING_ERROR("Unsupported data type.");
          break;
      }
    }
    if (memory_->heap[i].type[0] != 0x00) memory_->heap[i].const_type = true;
    bytecode_file += 1;
  }

  // Reads the class declarations.
  while (bytecode_file < code.data() + code.size() - 1) {
    bytecode_file = AddClass(bytecode_file);
  }

  // Initializes the current bytecode file.
  current_bytecode_file_ = "";

  Builtin::InitializeBuiltinFunction(builtin_functions_);

  // Initializes the main class.
  memory_->heap.push_back({{0x05}, true, ".!__start"});
  std::size_t main_class_name_index = memory_->heap.size() - 1;
  memory_->heap.push_back({{0x04}, true, uint64_t(0)});
  std::size_t main_class_count_index = memory_->heap.size() - 1;

  // 2 is the index of the main class in the object table.
  NEW(memory_->heap, 2, main_class_count_index, main_class_name_index);

  // Invokes the main function of the main class.
  InvokeClassFunction(memory_->heap, 2, "!__start", {1});
}

}  // namespace Vm
}  // namespace Aq