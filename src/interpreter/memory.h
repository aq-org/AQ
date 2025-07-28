// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_MEMORY_H_
#define AQ_INTERPRETER_MEMORY_H_

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <functional>

namespace Aq {
namespace Interpreter {
struct Object {
  std::vector<uint8_t> type;
  std::variant<int8_t, int64_t, double, uint64_t, std::string, void*,
               std::shared_ptr<struct Object>,
               std::reference_wrapper<struct Object>>
      data;
  bool constant;
};

class Memory {
 public:
  Memory() = default;
  virtual ~Memory() = default;

  // Adds the value into the memory and returns the index of the value.
  // The value is added to the end of the memory.
  std::size_t Add(std::size_t size);
  std::size_t AddWithType(std::vector<uint8_t> type);
  std::size_t AddByte(int8_t value);
  std::size_t AddLong(int64_t value);
  std::size_t AddDouble(double value);
  std::size_t AddUint64t(uint64_t value);
  void SetUint64tValue(std::size_t index, uint64_t value);
  std::size_t AddString(std::string value);

  std::shared_ptr<std::vector<Object>> GetMemory() { return memory_; }

 protected:
  std::shared_ptr<std::vector<Object>> memory_;
};

class ClassMemory {
 public:
  ClassMemory() = default;
  virtual ~ClassMemory() = default;

  // Adds the value into the class memory with the name and returns the index of
  // the value in the class memory. The value is added to the end of the memory.
  void Add(std::string name);
  void AddWithType(std::string name, std::vector<uint8_t> type);
  void AddByte(std::string name, int8_t value);
  void AddLong(std::string name, int64_t value);
  void AddDouble(std::string name, double value);
  void AddUint64t(std::string name, uint64_t value);
  void AddString(std::string name, std::string value);

  auto& GetMembers() { return members_; }

 private:
  std::unordered_map<std::string, Object> members_;
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
}  // namespace Interpreter
}  // namespace Aq

#endif