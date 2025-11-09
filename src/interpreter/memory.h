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

// ObjectReference represents a reference to a variable in memory.
// It can reference either a Memory object (for regular variables/arrays)
// or a ClassMemory object (for class member variables).
struct ObjectReference {
  // Indicates whether the reference points to a class member.
  bool is_class = false;
  
  // Union to store either a regular Memory pointer or a ClassMemory pointer.
  // Only one will be valid at a time based on is_class flag.
  union {
    Memory* memory;
    ClassMemory* class_memory;
  } memory;
  
  // Union to store the location within the memory.
  // For Memory: index is a numeric position.
  // For ClassMemory: variable_name is a string identifier.
  union {
    std::size_t index;
    std::string* variable_name;
  } index;
};

// Object represents a single value in the AQ interpreter's memory system.
// It uses a tagged union approach where the 'type' field determines which
// member of the 'data' union is valid.
struct Object {
  // Type tag indicating which data union member is active:
  // 0x00: Uninitialized/null
  // 0x01: byte (int8_t)
  // 0x02: int (int64_t)
  // 0x03: float (double)
  // 0x04: uint64
  // 0x05: string
  // 0x06: array
  // 0x07: reference
  // 0x08: [deprecated] const
  // 0x09: class object
  // 0x0A: pointer
  uint8_t type;
  
  // Tagged union containing the actual value.
  // Only one member is valid at a time, determined by the type field.
  union {
    int8_t byte_data;                 // 0x01 (byte)
    int64_t int_data;                 // 0x02 (int)
    double float_data;                // 0x03 (float)
    uint64_t uint64t_data;            // 0x04 (uint64t)
    std::string* string_data;         // 0x05 (string)
    Memory* array_data;               // 0x06 (array)
    ObjectReference* reference_data;  // 0x07 (reference)
    // [[deprecated]] 0x08 (const)
    ClassMemory* class_data;          // 0x09 (class)
    void* pointer_data;               // 0x0A (pointer)
  } data;
  
  // Indicates whether this object's type can be changed.
  // Constant objects maintain their type and value throughout their lifetime.
  bool constant_type = false;
};

// Memory manages a contiguous vector of Object instances, providing allocation
// and access methods for the AQ interpreter's runtime memory system.
// This class implements reference counting for automatic memory management.
class Memory {
 public:
  Memory() = default;
  virtual ~Memory() = default;

  // Allocates uninitialized objects in memory.
  // Returns the index of the first allocated object.
  // The allocated objects have type 0x00 (uninitialized).
  std::size_t Add(std::size_t size);
  
  // Allocates a single object with the specified type.
  // Returns the index of the allocated object.
  std::size_t AddWithType(uint8_t type);
  
  // Adds a byte value (int8_t) to memory.
  // Returns the index where the value is stored.
  std::size_t AddByte(int8_t value);
  
  // Adds a long integer value (int64_t) to memory.
  // Returns the index where the value is stored.
  std::size_t AddLong(int64_t value);
  
  // Adds a double-precision floating-point value to memory.
  // Returns the index where the value is stored.
  std::size_t AddDouble(double value);
  
  // Adds an unsigned 64-bit integer value to memory.
  // Returns the index where the value is stored.
  std::size_t AddUint64t(uint64_t value);
  
  // Updates an existing memory location with a new uint64_t value.
  void SetUint64tValue(std::size_t index, uint64_t value);
  
  // Adds a string value to memory.
  // The string is stored as a pointer, so the string data is heap-allocated.
  // Returns the index where the string pointer is stored.
  std::size_t AddString(std::string value);
  
  // Adds a reference to a Memory object at a specific index.
  // Returns the index where the reference is stored.
  std::size_t AddReference(Memory* memory, std::size_t index);
  
  // Adds a reference to a ClassMemory object with a string identifier.
  // Returns the index where the reference is stored.
  std::size_t AddReference(ClassMemory* memory, std::string index);

  // Initializes an object at the given index with class data.
  void InitObjectData(std::size_t index, ClassMemory* object);
  
  // Updates an existing object with new class data.
  void SetObjectData(std::size_t index, ClassMemory* object);
  
  // Updates an existing object with array data.
  void SetArrayData(std::size_t index, Memory* object);

  // Returns a direct reference to the Object at the given index.
  // The returned reference allows modification of the object.
  Object& GetOriginData(std::size_t index);
  
  // Retrieves the uint64_t value stored at the given index.
  uint64_t GetUint64tData(std::size_t index);
  
  // Retrieves the string value stored at the given index.
  std::string GetStringData(std::size_t index);

  // Returns a reference to the internal memory vector.
  // This allows direct access to all objects for iteration or bulk operations.
  std::vector<Object>& GetMemory() { return memory_; }
  
  // Replaces the entire memory vector with a new one.
  // Uses move semantics for efficiency.
  void SetMemory(std::vector<Object>& memory) { memory_ = std::move(memory); }

  // Increments the reference count for this Memory object.
  // Used for reference-counted memory management to track how many
  // references exist to this memory region.
  void AddReferenceCount() { reference_count_++; }

  // Decrements the reference count and deletes this object if it reaches zero.
  // This implements automatic memory management: when no more references exist,
  // the Memory object is automatically freed.
  void RemoveReferenceCount() {
    if (reference_count_ > 0) reference_count_--;
    if (reference_count_ <= 0) delete this;
  }

 private:
  // Vector storing all objects in this memory region.
  // Objects are accessed by their index in this vector.
  std::vector<Object> memory_;
  
  // Reference counter for automatic memory management.
  // When this reaches zero, the Memory object can be safely deleted.
  int64_t reference_count_ = 0;
};

// ClassMemory manages named members of a class instance.
// Unlike Memory which uses numeric indices, ClassMemory uses string names
// to identify members (fields and methods) of a class object.
class ClassMemory {
 public:
  ClassMemory() = default;
  virtual ~ClassMemory() = default;

  // Adds an uninitialized member with the given name to the class.
  void Add(std::string name);
  
  // Adds a member with the specified type but no initial value.
  void AddWithType(std::string name, uint8_t type);
  
  // Adds a byte (int8_t) member with the given name and value.
  void AddByte(std::string name, int8_t value);
  
  // Adds a long integer (int64_t) member with the given name and value.
  void AddLong(std::string name, int64_t value);
  
  // Adds a double-precision floating-point member with the given name and value.
  void AddDouble(std::string name, double value);
  
  // Adds an unsigned 64-bit integer member with the given name and value.
  void AddUint64t(std::string name, uint64_t value);
  
  // Adds a string member with the given name and value.
  void AddString(std::string name, std::string value);
  
  // Adds a reference to a Memory object as a member.
  void AddReference(std::string name, Memory* memory, std::size_t index);
  
  // Adds a reference to another ClassMemory object as a member.
  void AddReference(std::string name, ClassMemory* memory, std::string index);

  // Returns a reference to the Object stored under the given name.
  // Allows direct access and modification of the member.
  Object& GetOriginData(std::string index);

  // Returns the internal map of all members.
  // This allows iteration over all members of the class instance.
  std::unordered_map<std::string, Object>& GetMembers() { return members_; }

  // Increments the reference count for this ClassMemory object.
  void AddReferenceCount() { reference_count_++; }

  // Decrements the reference count and deletes this object if it reaches zero.
  // Implements automatic memory management for class instances.
  void RemoveReferenceCount() {
    if (reference_count_ > 0) reference_count_--;
    if (reference_count_ <= 0) delete this;
  }

 private:
  // Map of member names to their Object values.
  // Allows O(1) lookup of members by name.
  std::unordered_map<std::string, Object> members_;
  
  // Reference counter for automatic memory management.
  int64_t reference_count_ = 0;
};

// RunGc performs garbage collection on an Object by freeing any dynamically
// allocated resources it owns. This is called before overwriting or destroying
// an Object to prevent memory leaks.
FORCE_INLINE void RunGc(Object* object) {
  switch (object->type) {
    case 0x05:  // String: delete the heap-allocated string
      delete object->data.string_data;
      break;
    case 0x06:  // Array: decrement the reference count of the array Memory
      object->data.array_data->RemoveReferenceCount();
      break;
    case 0x07:  // Reference: delete the variable name (if class) and the reference struct
      if (object->data.reference_data->is_class)
        delete object->data.reference_data->index.variable_name;
      delete object->data.reference_data;
      break;
    case 0x09:  // Class: decrement the reference count of the class instance
      object->data.class_data->RemoveReferenceCount();
      break;
    default:
      // Other types (byte, int, float, uint64, pointer) don't require cleanup
      break;
  }
}

// GetOrigin dereferences an Object through any chain of references (type 0x07)
// to find the actual target Object being referenced. This is essential for
// operating on the actual value rather than intermediate reference objects.
// Returns a pointer to the final dereferenced Object.
FORCE_INLINE Object* GetOrigin(Object* object) {
  // Follow the reference chain until we reach a non-reference Object
  while (object->type == 0x07) {
    auto reference = object->data.reference_data;
    if (reference->is_class) {
      // Reference to a class member: look up by name
      object = &reference->memory.class_memory
                    ->GetMembers()[*reference->index.variable_name];
    } else {
      // Reference to a Memory location: look up by index
      object = &reference->memory.memory->GetMemory()[reference->index.index];
    }
  }

  return object;
}

// GetByte extracts an int8_t value from an Object, performing automatic type
// conversion if necessary. Dereferences the object first if it's a reference.
// Supports conversion from byte, int, float, and uint64 types.
FORCE_INLINE int8_t GetByte(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x01:  // Already a byte: return directly
      return object->data.byte_data;
    case 0x02:  // Int: cast to byte (may truncate)
      return static_cast<int8_t>(object->data.int_data);
    case 0x03:  // Float: cast to byte (truncates decimal part)
      return static_cast<int8_t>(object->data.float_data);
    case 0x04:  // Uint64: cast to byte (may truncate)
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

FORCE_INLINE void SetPtr(Object* object, void* ptr) {
  if (object->type == 0x07) object = GetOrigin(object);
  RunGc(object);

  if (object->type == 0x0A || !object->constant_type) {
    object->type = 0x0A;
    object->data.pointer_data = ptr;
  } else {
    LOGGING_ERROR("Cannot set ptr to constant type memory.");
  }
}

FORCE_INLINE void* GetPtr(Object* object) {
  if (object->type == 0x07) object = GetOrigin(object);
  switch (object->type) {
    case 0x0A:
      return object->data.pointer_data;
    default:
      LOGGING_ERROR("Unsupported data type: " + std::to_string(object->type));
      break;
  }

  return nullptr;
}
}  // namespace Interpreter
}  // namespace Aq

#endif