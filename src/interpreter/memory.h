// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_MEMORY_H_
#define AQ_INTERPRETER_MEMORY_H_

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "logging/logging.h"

namespace Aq {
namespace Interpreter {
class Memory;
class ClassMemory;

struct ObjectReference {
  std::variant<std::shared_ptr<Memory>,
               std::shared_ptr<ClassMemory>>
      memory;
  std::variant<std::size_t, std::string> index;
};

struct Object {
  std::vector<uint8_t> type;
  std::variant<int8_t,                   // 0x01 (byte)
               int64_t,                  // 0x02 (int)
               double,                   // 0x03 (float)
               uint64_t,                 // 0x04 (uint64t)
               std::string,              // 0x05 (string)
               std::shared_ptr<Memory>,  // 0x06 (array)
               ObjectReference,          // 0x07 (reference)
               // [[deprecated]] 0x08 (const)
               std::shared_ptr<ClassMemory>,  // 0x09 (class)
               void*                          // [[UNUSED]] 0x0A (pointer)
               >
      data;
  bool constant_type = false;
  bool constant_data = false;

  // |guard_tag| is used to inline cache tag types during execution to ensure
  // the validity of the types used in the operator. Among them, the value 0x00
  // indicates that no inline cache tag has been executed or that the object is
  // a reference. The values 0x01~0x0A indicate that it is a basic type. If the
  // value exceeds 0x0A, it indicates that it is a class and a specific type is
  // defined within the class.
  uint64_t guard_tag = 0;

  // |guard_ptr| is used to cache data pointers (or object pointers if
  // referenced) during execution to ensure the validity of references used in
  // operators. If it is null ptr, it means that no inline cache marking has
  // been made. Otherwise, if the object has a reference, store a pointer to the
  // object it ultimately references. If the object does not have a reference,
  // store its data pointer.
  void* guard_ptr = nullptr;
};

class Memory {
 public:
  Memory() = default;
  virtual ~Memory() = default;

  // Adds the value into the memory and returns the index of the value.
  // The value is added to the end of the memory.
  std::size_t Add(std::size_t size, bool is_constant_data = false);
  std::size_t AddWithType(std::vector<uint8_t> type,
                          bool is_constant_data = false);
  std::size_t AddByte(int8_t value, bool is_constant_data = false);
  std::size_t AddLong(int64_t value, bool is_constant_data = false);
  std::size_t AddDouble(double value, bool is_constant_data = false);
  std::size_t AddUint64t(uint64_t value, bool is_constant_data = false);
  void SetUint64tValue(std::size_t index, uint64_t value,
                       bool is_constant_data = false);
  std::size_t AddString(std::string value, bool is_constant_data = false);
  std::size_t AddReference(std::shared_ptr<Memory> memory, std::size_t index,
                           bool is_constant_data = false);
  std::size_t AddReference(std::shared_ptr<ClassMemory> memory, std::string index,
                           bool is_constant_data = false);

  Object GetOriginData(std::size_t index);
  uint64_t GetUint64tData(std::size_t index);
  std::string GetStringData(std::size_t index);

  std::vector<Object>& GetMemory() { return memory_; }

 private:
  std::vector<Object> memory_;
};

class ClassMemory {
 public:
  ClassMemory() = default;
  virtual ~ClassMemory() = default;

  // Adds the value into the class memory with the name and returns the index of
  // the value in the class memory. The value is added to the end of the memory.
  void Add(std::string name, bool is_constant_data = false);
  void AddWithType(std::string name, std::vector<uint8_t> type,
                   bool is_constant_data = false);
  void AddByte(std::string name, int8_t value, bool is_constant_data = false);
  void AddLong(std::string name, int64_t value, bool is_constant_data = false);
  void AddDouble(std::string name, double value, bool is_constant_data = false);
  void AddUint64t(std::string name, uint64_t value,
                  bool is_constant_data = false);
  void AddString(std::string name, std::string value,
                 bool is_constant_data = false);
  void AddReference(std::string name, std::shared_ptr<Memory> memory, std::size_t index,
                    bool is_constant_data = false);
  void AddReference(std::string name, std::shared_ptr<ClassMemory> memory, std::string index,
                    bool is_constant_data = false);

  Object GetOriginData(std::string index);

  std::unordered_map<std::string, Object>& GetMembers() { return members_; }

 private:
  std::unordered_map<std::string, Object> members_;
};

extern bool is_run;

}  // namespace Interpreter
}  // namespace Aq

#endif