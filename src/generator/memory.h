// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_GENERATOR_MEMORY_H_
#define AQ_GENERATOR_MEMORY_H_

#include <sys/types.h>
#include <string>
#include <cstring>
#include <cstdint>

#include "generator/bytecode.h"
#include "generator/operator.h"
#include "generator/uleb128.h"

namespace Aq {
namespace Generator {
class Memory {
 public:
  Memory() {
    uint16_t test_data = 0x0011;
    is_big_endian_ = *(uint8_t*)&test_data == 0x00;
  }
  virtual ~Memory() = default;

  void SetCode(std::vector<Bytecode>* code) { init_code_ = code; }

  // Adds the value into the memory and returns the index of the value.
  // The value is added to the end of the memory.
  std::size_t Add(std::size_t size);
  std::size_t AddWithType(std::vector<uint8_t> type);
  std::size_t AddByte(int8_t value);
  std::size_t AddLong(int64_t value);
  std::size_t AddDouble(double value);
  std::size_t AddUint64t(uint64_t value);
  std::size_t AddUint64tWithoutValue(std::size_t& code);
  void SetUint64tValue(std::size_t code, uint64_t value);
  std::size_t AddString(std::string value);

  std::vector<uint8_t>& GetMemoryType() { return memory_type_; }

  std::vector<uint8_t>& GetConstTable() { return constant_table_; }

  std::size_t& GetConstTableSize() { return constant_table_size_; }

  std::size_t GetMemorySize() { return memory_size_; }

 protected:
  bool is_big_endian_ = false;
  std::vector<Bytecode>* init_code_;
  std::vector<uint8_t> constant_table_;
  std::size_t constant_table_size_ = 0;
  std::vector<uint8_t> memory_type_;
  std::size_t memory_size_ = 0;
};

class ClassMemory : public Memory {
 public:
  ClassMemory() {
    uint16_t test_data = 0x0011;
    is_big_endian_ = *(uint8_t*)&test_data == 0x00;
  }
  virtual ~ClassMemory() = default;

  void SetGlobalMemory(Memory* global_memory) {
    global_memory_ = global_memory;
  }

  // Adds the value into the class memory with the name and returns the index of
  // the value in the class memory. The value is added to the end of the memory.
  std::size_t Add(std::string name);
  std::size_t AddWithType(std::string name, std::vector<uint8_t> type);
  std::size_t AddByte(std::string name, int8_t value);
  std::size_t AddLong(std::string name, int64_t value);
  std::size_t AddDouble(std::string name, double value);
  std::size_t AddUint64t(std::string name, uint64_t value);
  std::size_t AddString(std::string name, std::string value);

  std::vector<std::string>& GetVarName() { return variable_name_; }

  std::vector<uint8_t>& GetMemoryInfo() { return memory_info_; }

 private:
  Memory* global_memory_ = nullptr;
  std::vector<std::string> variable_name_;
  std::vector<uint8_t> memory_info_;
};

// Swaps the byte order of a 64-bit integer.
inline int64_t SwapLong(int64_t x) {
  uint64_t ux = (uint64_t)x;
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  return (int64_t)ux;
}

// Swaps the byte order of a double.
inline double SwapDouble(double x) {
  uint64_t ux;
  memcpy(&ux, &x, sizeof(uint64_t));
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  double result;
  memcpy(&result, &ux, sizeof(double));
  return result;
}

// Swaps the byte order of a 64-bit unsigned integer.
inline uint64_t SwapUint64t(uint64_t x) {
  x = ((x << 56) & 0xFF00000000000000ULL) |
      ((x << 40) & 0x00FF000000000000ULL) |
      ((x << 24) & 0x0000FF0000000000ULL) | ((x << 8) & 0x000000FF00000000ULL) |
      ((x >> 8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
      ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
  return x;
}
}  // namespace Generator
}  // namespace Aq

#endif