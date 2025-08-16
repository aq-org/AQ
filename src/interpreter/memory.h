// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_MEMORY_H_
#define AQ_INTERPRETER_MEMORY_H_

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "logging/logging.h"

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
  void AddReference(std::string name, ClassMemory* memory, std::string index);

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

inline __attribute__((always_inline)) void RunGc(Object* memory,
                                                 std::size_t index) {
  switch (memory[index].type) {
    case 0x05:
      delete memory[index].data.string_data;
      break;
    case 0x06:
      delete memory[index].data.array_data;
      break;
    case 0x07:
      delete memory[index].data.reference_data;
      break;
    case 0x09:
      memory[index].data.class_data->RemoveReferenceCount();
      break;
    default:
      break;
  }
}

inline __attribute__((always_inline)) int8_t GetByte(Object* memory,
                                                     std::size_t index) {
  switch (memory[index].type) {
    case 0x01:
      return memory[index].data.byte_data;
    case 0x02:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(memory[index].data.int_data);
    case 0x03:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(memory[index].data.float_data);
    case 0x04:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(memory[index].data.uint64t_data);
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(memory[index].type));
      break;
  }

  return 0;
}

inline __attribute__((always_inline)) void SetByte(Object* memory,
                                                   std::size_t index,
                                                   int8_t data) {
  if (memory[index].type == 0x01) {
    memory[index].data.byte_data = data;
    return;
  }

  if (memory[index].constant_type) {
    switch (memory[index].type) {
      case 0x02:
        memory[index].data.int_data = data;
        return;
      case 0x03:
        memory[index].data.float_data = data;
        return;
      case 0x04:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(memory[index].type));
        return;
    }
  }

  RunGc(memory, index);
  memory[index].type = 0x01;
  memory[index].data.byte_data = data;
}

inline __attribute__((always_inline)) int64_t GetLong(Object* memory,
                                                      std::size_t index) {
  switch (memory[index].type) {
    case 0x01:
      return memory[index].data.byte_data;
    case 0x02:
      return memory[index].data.int_data;
    case 0x03:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int64_t>(memory[index].data.float_data);
    case 0x04:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int64_t>(memory[index].data.uint64t_data);
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(memory[index].type));
      break;
  }

  return 0;
}

inline __attribute__((always_inline)) void SetLong(Object* memory,
                                                   std::size_t index,
                                                   int64_t data) {
  if (memory[index].type == 0x02) {
    memory[index].data.int_data = data;
    return;
  }

  if (memory[index].constant_type) {
    switch (memory[index].type) {
      case 0x01:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.byte_data = data;
        return;
      case 0x03:
        memory[index].data.float_data = data;
        return;
      case 0x04:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(memory[index].type));
        return;
    }
  }

  RunGc(memory, index);
  memory[index].type = 0x02;
  memory[index].data.int_data = data;
}

inline __attribute__((always_inline)) double GetDouble(Object* memory,
                                                       std::size_t index) {
  switch (memory[index].type) {
    case 0x01:
      return memory[index].data.byte_data;
    case 0x02:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<double>(memory[index].data.int_data);
    case 0x03:
      return memory[index].data.float_data;
    case 0x04:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<double>(memory[index].data.uint64t_data);
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(memory[index].type));
      break;
  }

  return 0.0;
}

inline __attribute__((always_inline)) void SetDouble(Object* memory,
                                                     std::size_t index,
                                                     double data) {
  if (memory[index].type == 0x03) {
    memory[index].data.float_data = data;
    return;
  }

  if (memory[index].constant_type) {
    switch (memory[index].type) {
      case 0x01:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.byte_data = data;
        return;
      case 0x02:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.int_data = data;
        return;
      case 0x04:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(memory[index].type));
        return;
    }
  }

  RunGc(memory, index);
  memory[index].type = 0x03;
  memory[index].data.float_data = data;
}

inline __attribute__((always_inline)) uint64_t GetUint64(Object* memory,
                                                         std::size_t index) {
  switch (memory[index].type) {
    case 0x01:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<uint64_t>(memory[index].data.byte_data);
    case 0x02:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<uint64_t>(memory[index].data.int_data);
    case 0x03:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<uint64_t>(memory[index].data.float_data);
    case 0x04:
      return memory[index].data.uint64t_data;
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(memory[index].type));
      break;
  }

  return 0;
}

inline __attribute__((always_inline)) void SetUint64(Object* memory,
                                                     std::size_t index,
                                                     uint64_t data) {
  if (memory[index].type == 0x04) {
    memory[index].data.uint64t_data = data;
    return;
  }

  if (memory[index].constant_type) {
    switch (memory[index].type) {
      case 0x01:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.byte_data = data;
        return;
      case 0x02:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.int_data = data;
        return;
      case 0x03:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        memory[index].data.float_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(memory[index].type));
        return;
    }
  }

  RunGc(memory, index);
  memory[index].type = 0x04;
  memory[index].data.uint64t_data = data;
}

inline __attribute__((always_inline)) std::string GetString(Object* memory,
                                                            std::size_t index) {
  switch (memory[index].type) {
    case 0x05:
      return *memory[index].data.string_data;
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(memory[index].type));
      break;
  }

  return "";
}

inline __attribute__((always_inline)) void SetString(Object* memory,
                                                     std::size_t index,
                                                     const std::string& data) {
  if (memory[index].type == 0x05) {
    *memory[index].data.string_data = data;
    return;
  }

  if (memory[index].constant_type) {
    LOGGING_ERROR("Unsupported data type: " +
                  std::to_string(memory[index].type));
    return;
  }

  RunGc(memory, index);
  memory[index].type = 0x05;
  memory[index].data.string_data = new std::string(data);
}

inline __attribute__((always_inline)) Memory* GetArray(Object* memory,
                                                       std::size_t index) {
  switch (memory[index].type) {
    case 0x06:
      return memory[index].data.array_data;
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(memory[index].type));
      break;
  }

  return nullptr;
}

inline __attribute__((always_inline)) void SetArrayContent(
    Object* memory, std::size_t index, std::vector<Object>& data) {
  if (memory[index].type == 0x06) {
    memory[index].data.array_data->SetMemory(data);
    return;
  }

  if (memory[index].constant_type) {
    LOGGING_ERROR("Cannot set array to constant type memory.");
  }

  RunGc(memory, index);
  memory[index].type = 0x06;
  memory[index].data.array_data = new Memory();
  memory[index].data.array_data->SetMemory(data);
}

inline __attribute__((always_inline)) void SetArray(Object* memory,
                                                    std::size_t index,
                                                    Memory* data) {
  RunGc(memory, index);
  if (memory[index].type == 0x06 || !memory[index].constant_type) {
    memory[index].type = 0x06;
    memory[index].data.array_data = data;
    data->AddReferenceCount();
  } else {
    LOGGING_ERROR("Cannot set array to constant type memory.");
  }
}

inline __attribute__((always_inline)) ClassMemory* GetObject(
    Object* memory, std::size_t index) {
  switch (memory[index].type) {
    case 0x09:
      return memory[index].data.class_data;
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(memory[index].type));
      break;
  }

  return nullptr;
}

inline __attribute__((always_inline)) void SetObject(Object* memory,
                                                     std::size_t index,
                                                     ClassMemory* data) {
  RunGc(memory, index);
  if (memory[index].type == 0x09 || !memory[index].constant_type) {
    memory[index].type = 0x09;
    memory[index].data.class_data = data;
    data->AddReferenceCount();
  } else {
    LOGGING_ERROR("Cannot set class to constant type memory.");
  }
}

inline __attribute__((always_inline)) Object& GetOrigin(Object* memory,
                                                        std::size_t index) {
  std::reference_wrapper<Object> object = memory[index];

  while (object.get().type == 0x07) {
    auto reference = object.get().data.reference_data;
    if (reference->is_class) {
      object = std::ref(reference->memory.class_memory
                            ->GetMembers()[*reference->index.variable_name]);
    } else {
      object = std::ref(
          reference->memory.memory->GetMemory()[reference->index.index]);
    }
  }

  return object;
}

inline __attribute__((always_inline)) void SetReference(
    Object* memory, std::size_t index, ObjectReference reference) {
  RunGc(memory, index);
  if (memory[index].type == 0x07 || !memory[index].constant_type) {
    memory[index].type = 0x07;
    memory[index].data.reference_data = new ObjectReference(reference);
  } else {
    LOGGING_ERROR("Cannot set reference to constant type memory.");
  }
}

}  // namespace Interpreter
}  // namespace Aq

#endif