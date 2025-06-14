// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_BYTECODE_H_
#define AQ_COMPILER_GENERATOR_BYTECODE_H_

#include <cstdarg>
#include <cstddef>
#include <vector>


namespace Aq {
namespace Compiler {
namespace Generator {
class Bytecode {
 public:
  Bytecode(uint8_t oper, std::size_t arguments_count, ...) {
    oper_ = oper;
    va_list arguments;
    va_start(arguments, arguments_count);
    for (std::size_t i = 0; i < arguments_count; i++) {
      arguments_.push_back(va_arg(arguments, std::size_t));
    }
    va_end(arguments);
  }
  Bytecode(uint8_t oper, std::vector<std::size_t> arguments) {
    oper_ = oper;
    arguments_ = arguments;
  }
  ~Bytecode() = default;

  uint8_t GetOper() { return oper_; }
  std::vector<std::size_t> GetArgs() { return arguments_; }

  void SetOper(uint8_t oper) { oper_ = oper; }
  void SetArgs(std::vector<std::size_t> arguments) { arguments_ = arguments; }
  void SetArgs(std::size_t arguments_count, ...) {
    va_list arguments;
    va_start(arguments, arguments_count);
    for (std::size_t i = 0; i < arguments_count; i++) {
      arguments_.push_back(va_arg(arguments, std::size_t));
    }
    va_end(arguments);
  }

 private:
  uint8_t oper_ = 0x00;
  std::vector<std::size_t> arguments_;
};
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq

#endif
