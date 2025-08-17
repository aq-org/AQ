// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_BYTECODE_H_
#define AQ_INTERPRETER_BYTECODE_H_

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Aq {
namespace Interpreter {
struct Bytecode {
  uint8_t oper = 0x00;
  std::vector<std::size_t> arguments;
};
}  // namespace Interpreter
}  // namespace Aq

#endif
