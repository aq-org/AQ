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

inline
    //__attribute__((always_inline))
    void
    RunGc(Object* object) {
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

inline
    //__attribute__((always_inline))
    int8_t
    GetByte(Object* object) {
  switch (object->type) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(object->data.int_data);
    case 0x03:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(object->data.float_data);
    case 0x04:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(object->data.uint64t_data);
    /*case 0x07:
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      return GetByte(object);*/
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0;
}

inline
    //__attribute__((always_inline))
    void
    SetByte(Object* object, int8_t data) {
  /*if (object->constant_type) {
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
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }*/

  //RunGc(object);
  object->type = 0x01;
  object->data.byte_data = data;
}

inline
    //__attribute__((always_inline))
    int64_t
    GetLong(Object* object) {
  switch (object->type) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      return object->data.int_data;
    case 0x03:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int64_t>(object->data.float_data);
    case 0x04:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int64_t>(object->data.uint64t_data);
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0;
}

inline
    //__attribute__((always_inline))
    void
    SetLong(Object* object, int64_t data) {
  /*if (object->constant_type) {
    switch (object->type) {
      case 0x01:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.byte_data = data;
        return;
      case 0x02:
        object->data.int_data = data;
        return;
      case 0x03:
        object->data.float_data = data;
        return;
      case 0x04:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }*/

  //RunGc(object);
  object->type = 0x02;
  object->data.int_data = data;
}

inline
    //__attribute__((always_inline))
    double
    GetDouble(Object* object) {
  switch (object->type) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<double>(object->data.int_data);
    case 0x03:
      return object->data.float_data;
    case 0x04:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<double>(object->data.uint64t_data);
    /*case 0x07:
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      return GetDouble(object);*/
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0.0;
}

inline
    //__attribute__((always_inline))
    void
    SetDouble(Object* object, double data) {
  /*if (object->constant_type) {
    switch (object->type) {
      case 0x01:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.byte_data = data;
        return;
      case 0x02:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.int_data = data;
        return;
      case 0x03:
        object->data.float_data = data;
        return;
      case 0x04:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }*/

  //RunGc(object);
  object->type = 0x03;
  object->data.float_data = data;
}

inline
    //__attribute__((always_inline))
    uint64_t
    GetUint64(Object* object) {
  switch (object->type) {
    case 0x01:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<uint64_t>(object->data.byte_data);
    case 0x02:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<uint64_t>(object->data.int_data);
    case 0x03:
      // LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<uint64_t>(object->data.float_data);
    case 0x04:
      return object->data.uint64t_data;
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return 0;
}

inline
    //__attribute__((always_inline))
    void
    SetUint64(Object* object, uint64_t data) {
  /*if (object->constant_type) {
    switch (object->type) {
      case 0x01:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.byte_data = data;
        return;
      case 0x02:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.int_data = data;
        return;
      case 0x03:
        // LOGGING_WARNING("Implicit conversion may changes value.");
        object->data.float_data = data;
        return;
      case 0x04:
        object->data.uint64t_data = data;
        return;
      default:
        LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
        return;
    }
  }*/

  //RunGc(object);
  object->type = 0x04;
  object->data.uint64t_data = data;
}

inline
    //__attribute__((always_inline))
    std::string
    GetString(Object* object) {
  switch (object->type) {
    case 0x05:
      return *object->data.string_data;
    /*case 0x07:
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      return GetString(object);*/
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return "";
}

inline
    //__attribute__((always_inline))
    void
    SetString(Object* object, const std::string& data) {
  if (object->constant_type && object->type != 0x05) {
    /*if (object->type == 0x07) {
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      SetString(object, data);
      return;
    }*/
    LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
    return;
  }

  //RunGc(object);
  object->type = 0x05;
  object->data.string_data = new std::string(data);
}

inline
    //__attribute__((always_inline))
    Memory*
    GetArray(Object* object) {
  switch (object->type) {
    case 0x06:
      return object->data.array_data;
    /*case 0x07:
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      return GetArray(object);*/
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return nullptr;
}

inline
    //__attribute__((always_inline))
    void
    SetArrayContent(Object* object, std::vector<Object>& data) {
  if (object->constant_type && object->type != 0x06) {
    /*if (object->type == 0x07) {
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      SetArrayContent(object, data);
      return;
    }*/
    LOGGING_ERROR("Cannot set array to constant type memory.");
  }

  //RunGc(object);
  object->type = 0x06;
  object->data.array_data = new Memory();
  object->data.array_data->SetMemory(data);
}

inline
    //__attribute__((always_inline))
    void
    SetArray(Object* object, Memory* data) {
  //RunGc(object);

  if (object->type == 0x06 || !object->constant_type) {
    object->type = 0x06;
    object->data.array_data = data;
    data->AddReferenceCount();
  } else {
    /*if (object->type == 0x07) {
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      SetArray(object, data);
      return;
    }*/
    LOGGING_ERROR("Cannot set array to constant type memory.");
  }
}

inline
    //__attribute__((always_inline))
    ClassMemory*
    GetObject(Object* object) {
  switch (object->type) {
    case 0x09:
      return object->data.class_data;
    /*case 0x07:
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      return GetObject(object);*/
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return nullptr;
}

inline
    //__attribute__((always_inline))
    void
    SetObject(Object* object, ClassMemory* data) {
  //RunGc(object);

  if (object->type == 0x09 || !object->constant_type) {
    object->type = 0x09;
    object->data.class_data = data;
    data->AddReferenceCount();
  } else {
    /*if (object->type == 0x07) {
      while (object->type == 0x07) {
        auto reference = object->data.reference_data;
        if (reference->is_class) {
          object = &reference->memory.class_memory
                        ->GetMembers()[*reference->index.variable_name];
        } else {
          object =
              &reference->memory.memory->GetMemory()[reference->index.index];
        }
      }
      SetObject(object, data);
      return;
    }*/
    LOGGING_INFO("type: " + std::to_string(object->type));
    LOGGING_ERROR("Cannot set class to constant type memory.");
  }
}

inline
    //__attribute__((always_inline))
    Object*
    GetOrigin(Object* object) {
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

inline
    //__attribute__((always_inline))
    void
    SetReference(Object* object, ObjectReference reference) {
  //RunGc(object);
  if (object->type == 0x07 || !object->constant_type) {
    object->type = 0x07;
    object->data.reference_data = new ObjectReference(reference);
  } else {
    LOGGING_ERROR("Cannot set reference to constant type memory.");
  }
}

inline
    //__attribute__((always_inline))
    void
    InitGc(Object* object) {
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

inline
    //__attribute__((always_inline))
    void
    InitObject(Object* object, ClassMemory* class_memory) {
  InitGc(object);
  if (object->type == 0x09 || !object->constant_type) {
    object->type = 0x09;
    object->data.class_data = class_memory;
    class_memory->AddReferenceCount();
  } else {
    LOGGING_ERROR("Cannot set class to constant type memory.");
  }
}

inline
    //__attribute__((always_inline))
    void
    InitArray(Object* object, Memory* array_memory) {
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

#define GET_ORIGIN(object)                                               \
  {                                                                      \
    while (object->type == 0x07) {                                       \
      const auto& __reference = object->data.reference_data;             \
      if (__reference->is_class) {                                       \
        object = &__reference->memory.class_memory                       \
                      ->GetMembers()[*__reference->index.variable_name]; \
      } else {                                                           \
        object = &__reference->memory.memory                             \
                      ->GetMemory()[__reference->index.index];           \
      }                                                                  \
    }                                                                    \
  }

#define RUN_GC(object)                                             \
  {                                                                \
    switch (object->type) {                                        \
      case 0x05:                                                   \
        delete object->data.string_data;                           \
        break;                                                     \
      case 0x06:                                                   \
        object->data.array_data->RemoveReferenceCount();           \
        break;                                                     \
      case 0x07:                                                   \
        if (object->data.reference_data->is_class)                 \
          delete object->data.reference_data->index.variable_name; \
        delete object->data.reference_data;                        \
        break;                                                     \
      case 0x09:                                                   \
        object->data.class_data->RemoveReferenceCount();           \
        break;                                                     \
      default:                                                     \
        break;                                                     \
    }                                                              \
  }

#define SET_INT(object, value)                                       \
  {                                                                  \
    if (object->constant_type) {                                     \
      switch (object->type) {                                        \
        case 0x01:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.byte_data = value;                            \
          break;                                                     \
        case 0x02:                                                   \
          object->data.int_data = value;                             \
          break;                                                     \
        case 0x03:                                                   \
          object->data.float_data = value;                           \
          break;                                                     \
        case 0x04:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.uint64t_data = value;                         \
          break;                                                     \
        default:                                                     \
          LOGGING_ERROR("Unsupported data type: " +                  \
                        std::to_string(object->type));               \
          break;                                                     \
      }                                                              \
    } else {                                                         \
      RUN_GC(object);                                                \
      object->type = 0x02;                                           \
      object->data.int_data = value;                                 \
    }                                                                \
  }

#define SET_FLOAT(object, value)                                     \
  {                                                                  \
    if (object->constant_type) {                                     \
      switch (object->type) {                                        \
        case 0x01:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.byte_data = value;                            \
          break;                                                     \
        case 0x02:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.int_data = value;                             \
          break;                                                     \
        case 0x03:                                                   \
          object->data.float_data = value;                           \
          break;                                                     \
        case 0x04:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.uint64t_data = value;                         \
          break;                                                     \
        default:                                                     \
          LOGGING_ERROR("Unsupported data type: " +                  \
                        std::to_string(object->type));               \
          break;                                                     \
      }                                                              \
    } else {                                                         \
      RUN_GC(object);                                                \
      object->type = 0x03;                                           \
      object->data.float_data = value;                               \
    }                                                                \
  }

#define SET_UINT64T(object, value)                                   \
  {                                                                  \
    if (object->constant_type) {                                     \
      switch (object->type) {                                        \
        case 0x01:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.byte_data = value;                            \
          break;                                                     \
        case 0x02:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.int_data = value;                             \
          break;                                                     \
        case 0x03:                                                   \
          LOGGING_WARNING("Implicit conversion may changes value."); \
          object->data.float_data = value;                           \
          break;                                                     \
        case 0x04:                                                   \
          object->data.uint64t_data = value;                         \
          break;                                                     \
        default:                                                     \
          LOGGING_ERROR("Unsupported data type: " +                  \
                        std::to_string(object->type));               \
          break;                                                     \
      }                                                              \
    } else {                                                         \
      RUN_GC(object);                                                \
      object->type = 0x04;                                           \
      object->data.uint64t_data = value;                             \
    }                                                                \
  }

#define SET_STRING(object, value)                                              \
  {                                                                            \
    if (!object->constant_type || object->type == 0x05) {                      \
      RUN_GC(object);                                                          \
      object->type = 0x05;                                                     \
      object->data.string_data = new std::string(value);                       \
    } else {                                                                   \
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type)); \
    }                                                                          \
  }

#define SET_ARRAY_CONTENT(object, value)                          \
  {                                                               \
    if (object->type == 0x06) {                                   \
      object->data.array_data->SetMemory(value);                  \
    } else if (!object->constant_type) {                          \
      RUN_GC(object);                                             \
      object->type = 0x06;                                        \
      object->data.array_data = new Memory();                     \
      object->data.array_data->SetMemory(value);                  \
    } else {                                                      \
      LOGGING_ERROR("Cannot set array to constant type memory."); \
    }                                                             \
  }

#endif