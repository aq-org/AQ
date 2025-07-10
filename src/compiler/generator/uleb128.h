// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_GENERATOR_ULEB128_H_
#define AQ_COMPILER_GENERATOR_ULEB128_H_

#include <cstddef>
#include <vector>
#include <cstdint>

namespace Aq {
namespace Compiler {
namespace Generator {
inline std::size_t EncodeUleb128(std::size_t value,
                                 std::vector<uint8_t>& output) {
  std::size_t count = 0;
  do {
    uint8_t byte_data = value & 0x7F;
    value >>= 7;
    if (value != 0) {
      byte_data |= 0x80;
    }
    output.push_back(byte_data);
    count++;
  } while (value != 0);
  return count;
}
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq
#endif