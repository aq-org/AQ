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

namespace Aq {
namespace Interpreter {
class Memory;
class ClassMemory;

struct ObjectReference {
  bool is_class = false;
  union {
    Memory* memory;
    ClassMemory* class_memory;
  } memory;
  union {
    std::size_t index;
    std::string* variable_name;
  } index;
};

struct Object {
  uint8_t type;
  union {
    int8_t byte_data;                 // 0x01 (byte)
    int64_t int_data;                 // 0x02 (int)
    double float_data;                // 0x03 (float)
    uint64_t uint64t_data;            // 0x04 (uint64t)
    std::string* string_data;         // 0x05 (string)
    Memory* array_data;               // 0x06 (array)
    ObjectReference* reference_data;  // 0x07 (reference)
    // [[deprecated]] 0x08 (const)
    ClassMemory* class_data;  // 0x09 (class)
    void* pointer_data;       // [[UNUSED]] 0x0A (pointer)
  } data;
  bool constant_type = false;
};

class Memory {
 public:
  Memory() = default;
  virtual ~Memory() = default;

  // Adds the value into the memory and returns the index of the value.
  // The value is added to the end of the memory.
  std::size_t Add(std::size_t size);
  std::size_t AddWithType(uint8_t type);
  std::size_t AddByte(int8_t value);
  std::size_t AddLong(int64_t value);
  std::size_t AddDouble(double value);
  std::size_t AddUint64t(uint64_t value);
  void SetUint64tValue(std::size_t index, uint64_t value);
  std::size_t AddString(std::string value);
  std::size_t AddReference(Memory* memory, std::size_t index);
  std::size_t AddReference(ClassMemory* memory, std::string index);

  void InitObjectData(std::size_t index, ClassMemory* object);
  void SetObjectData(std::size_t index, ClassMemory* object);
  void SetArrayData(std::size_t index, Memory* object);

  Object& GetOriginData(std::size_t index);
  void GetLastReference(ObjectReference& object);
  uint64_t GetUint64tData(std::size_t index);
  std::string GetStringData(std::size_t index);

  std::vector<Object>& GetMemory() { return memory_; }
  void SetMemory(std::vector<Object>& memory) { memory_ = std::move(memory); }

  void AddReferenceCount() { reference_count_++; }

  void RemoveReferenceCount() {
    if (reference_count_ > 0) reference_count_--;
    if (reference_count_ == 0) delete this;
  }

 private:
  std::vector<Object> memory_;
  int64_t reference_count_ = 0;
};

class ClassMemory {
 public:
  ClassMemory() = default;
  virtual ~ClassMemory() = default;

  // Adds the value into the class memory with the name and returns the index of
  // the value in the class memory. The value is added to the end of the memory.
  void Add(std::string name);
  void AddWithType(std::string name, uint8_t type);
  void AddByte(std::string name, int8_t value);
  void AddLong(std::string name, int64_t value);
  void AddDouble(std::string name, double value);
  void AddUint64t(std::string name, uint64_t value);
  void AddString(std::string name, std::string value);
  void AddReference(std::string name, Memory* memory, std::size_t index);
  void AddReference(std::string name, ClassMemory* memory, std::string* index);

  Object& GetOriginData(std::string index);

  std::unordered_map<std::string, Object>& GetMembers() { return members_; }

  void AddReferenceCount() { reference_count_++; }

  void RemoveReferenceCount() {
    if (reference_count_ > 0) reference_count_--;
    if (reference_count_ == 0) delete this;
  }

 private:
  std::unordered_map<std::string, Object> members_;
  int64_t reference_count_ = 0;
};

}  // namespace Interpreter
}  // namespace Aq

#endif