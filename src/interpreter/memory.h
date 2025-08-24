// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_INTERPRETER_MEMORY_H_
#define AQ_INTERPRETER_MEMORY_H_

#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "interpreter/inline.h"
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
  uint64_t GetUint64tData(std::size_t index);
  std::string GetStringData(std::size_t index);

  std::vector<Object>& GetMemory() { return memory_; }
  void SetMemory(std::vector<Object>& memory) { memory_ = std::move(memory); }

  void AddReferenceCount() { reference_count_++; }

  void RemoveReferenceCount() {
    if (reference_count_ > 0) reference_count_--;
    if (reference_count_ <= 0) delete this;
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
    if (reference_count_ <= 0) delete this;
  }

 private:
  std::unordered_map<std::string, Object> members_;
  int64_t reference_count_ = 0;
};

FORCE_INLINE void RunGc(Object* object) {
  switch (object->type) {
    case 0x05:
      delete object->data.string_data;
      break;
    case 0x06:
      object->data.array_data->RemoveReferenceCount();
      break;
    case 0x07:
      if (object->data.reference_data->is_class)
        delete object->data.reference_data->index.variable_name;
      delete object->data.reference_data;
      break;
    case 0x09:
      object->data.class_data->RemoveReferenceCount();
      break;
    default:
      break;
  }
}

FORCE_INLINE Object* GetOrigin(Object* object) {
  while (object->type == 0x07) {
    auto reference = object->data.reference_data;
    if (reference->is_class) {
      object = &reference->memory.class_memory
                    ->GetMembers()[*reference->index.variable_name];
    } else {
      object = &reference->memory.memory->GetMemory()[reference->index.index];
    }
  }

  return object;
}

FORCE_INLINE int8_t GetByte(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      return static_cast<int8_t>(object->data.int_data);
    case 0x03:
      return static_cast<int8_t>(object->data.float_data);
    case 0x04:
      return static_cast<int8_t>(object->data.uint64t_data);
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0;
}

FORCE_INLINE void SetByte(Object* object, int8_t data) {
  if (object->type == 0x07) object = GetOrigin(object);
  if (object->constant_type) {
    switch (object->type) {
      case 0x01:
        object->data.byte_data = data;
        return;
      case 0x02:
        object->data.int_data = data;
        return;
      case 0x03:
        object->data.float_data = data;
        return;
      case 0x04:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }

  RunGc(object);
  object->type = 0x01;
  object->data.byte_data = data;
}

FORCE_INLINE int64_t GetLong(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      return object->data.int_data;
    case 0x03:
      return static_cast<int64_t>(object->data.float_data);
    case 0x04:
      return static_cast<int64_t>(object->data.uint64t_data);
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0;
}

FORCE_INLINE void SetLong(Object* object, int64_t data) {
  if (object->type == 0x07) object = GetOrigin(object);
  if (object->constant_type) {
    switch (object->type) {
      case 0x01:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.byte_data = data;
        return;
      case 0x02:
        object->data.int_data = data;
        return;
      case 0x03:
        object->data.float_data = data;
        return;
      case 0x04:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }

  RunGc(object);
  object->type = 0x02;
  object->data.int_data = data;
}

FORCE_INLINE double GetDouble(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      return static_cast<double>(object->data.int_data);
    case 0x03:
      return object->data.float_data;
    case 0x04:
      return static_cast<double>(object->data.uint64t_data);
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0.0;
}

FORCE_INLINE void SetDouble(Object* object, double data) {
  if (object->type == 0x07) object = GetOrigin(object);
  if (object->constant_type) {
    switch (object->type) {
      case 0x01:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.byte_data = data;
        return;
      case 0x02:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.int_data = data;
        return;
      case 0x03:
        object->data.float_data = data;
        return;
      case 0x04:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }

  RunGc(object);
  object->type = 0x03;
  object->data.float_data = data;
}

FORCE_INLINE uint64_t GetUint64(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x01:
      return static_cast<uint64_t>(object->data.byte_data);
    case 0x02:
      return static_cast<uint64_t>(object->data.int_data);
    case 0x03:
      return static_cast<uint64_t>(object->data.float_data);
    case 0x04:
      return object->data.uint64t_data;
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0;
}

FORCE_INLINE void SetUint64(Object* object, uint64_t data) {
  if (object->type == 0x07) object = GetOrigin(object);
  if (object->constant_type) {
    switch (object->type) {
      case 0x01:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.byte_data = data;
        return;
      case 0x02:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.int_data = data;
        return;
      case 0x03:
        LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.float_data = data;
        return;
      case 0x04:
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }

  RunGc(object);
  object->type = 0x04;
  object->data.uint64t_data = data;
}

FORCE_INLINE std::string GetString(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x05:
      return *object->data.string_data;
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return "";
}

FORCE_INLINE void SetString(Object* object, const std::string& data) {
  if (object->type == 0x07) object = GetOrigin(object);
  if (object->constant_type && object->type != 0x05) {
    LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
    return;
  }

  RunGc(object);
  object->type = 0x05;
  object->data.string_data = new std::string(data);
}

FORCE_INLINE Memory* GetArray(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x06:
      return object->data.array_data;
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return nullptr;
}

FORCE_INLINE void SetArrayContent(Object* object, std::vector<Object>& data) {
  if (object->type == 0x07) object = GetOrigin(object);
  if (object->constant_type && object->type != 0x06) {
    LOGGING_ERROR("Cannot set array to constant type memory.");
  }

  RunGc(object);
  object->type = 0x06;
  object->data.array_data = new Memory();
  object->data.array_data->SetMemory(data);
}

FORCE_INLINE void SetArray(Object* object, Memory* data) {
  if (object->type == 0x07) object = GetOrigin(object);
  RunGc(object);

  if (object->type == 0x06 || !object->constant_type) {
    object->type = 0x06;
    object->data.array_data = data;
    data->AddReferenceCount();
  } else {
    LOGGING_ERROR("Cannot set array to constant type memory.");
  }
}

FORCE_INLINE ClassMemory* GetObject(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x09:
      return object->data.class_data;
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return nullptr;
}

FORCE_INLINE void SetObject(Object* object, ClassMemory* data) {
  if (object->type == 0x07) object = GetOrigin(object);
  RunGc(object);

  if (object->type == 0x09 || !object->constant_type) {
    object->type = 0x09;
    object->data.class_data = data;
    data->AddReferenceCount();
  } else {
    LOGGING_INFO("type: " + std::to_string(object->type));
    LOGGING_ERROR("Cannot set class to constant type memory.");
  }
}

FORCE_INLINE void SetReference(Object* object, ObjectReference reference) {
  RunGc(object);
  if (object->type == 0x07 || !object->constant_type) {
    object->type = 0x07;
    object->data.reference_data = new ObjectReference(reference);
  } else {
    LOGGING_ERROR("Cannot set reference to constant type memory.");
  }
}

FORCE_INLINE void InitGc(Object* object) {
  switch (object->type) {
    case 0x05:
      if (object->data.string_data != nullptr) delete object->data.string_data;
      break;
    case 0x06:
      if (object->data.array_data != nullptr)
        object->data.array_data->RemoveReferenceCount();
      break;
    case 0x07:
      if (object->data.reference_data != nullptr) {
        if (object->data.reference_data->is_class)
          delete object->data.reference_data->index.variable_name;
        delete object->data.reference_data;
      }
      break;
    case 0x09:
      if (object->data.class_data != nullptr)
        object->data.class_data->RemoveReferenceCount();
      break;
    default:
      break;
  }
}

FORCE_INLINE void InitObject(Object* object, ClassMemory* class_memory) {
  InitGc(object);
  if (object->type == 0x09 || !object->constant_type) {
    object->type = 0x09;
    object->data.class_data = class_memory;
    class_memory->AddReferenceCount();
  } else {
    LOGGING_ERROR("Cannot set class to constant type memory.");
  }
}

FORCE_INLINE void InitArray(Object* object, Memory* array_memory) {
  InitGc(object);
  if (object->type == 0x06 || !object->constant_type) {
    object->type = 0x06;
    object->data.array_data = array_memory;
    array_memory->AddReferenceCount();
  } else {
    LOGGING_ERROR("Cannot set array to constant type memory.");
  }
}
}  // namespace Interpreter
}  // namespace Aq

#endif