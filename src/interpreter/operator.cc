// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/operator.h"

#include "interpreter/interpreter.h"
#include "logging/logging.h"
#include "memory.h"

namespace Aq {
namespace Interpreter {
// Global index used for tracking class instances
std::size_t current_class_index = 2;

// Global pointer to the interpreter's memory for quick access
Object* global_memory_ptr = nullptr;

// NOP (No Operation) is a placeholder operator that does nothing.
// Used as a default or padding in bytecode.
int NOP() { return 0; }

// NEW allocates a new object or array in memory.
// This operator handles three cases:
// 1. Auto/dynamic type allocation (type == 0)
// 2. Class instantiation (type is a string naming a class)
// 3. Array allocation (type specifies element type, size > 0)
//
// Parameters:
//   memory: Pointer to the memory array
//   classes: Map of class definitions
//   ptr: Memory location where to store the allocated object
//   size: Number of elements (0 for single object, >0 for array)
//   type: Type information (0 for auto, string for class, type code for array)
//   builtin_functions: Map of built-in function implementations
int NEW(Object* memory, std::unordered_map<std::string, Class>& classes,
        std::size_t ptr, std::size_t size, std::size_t type,
        std::unordered_map<
            std::string, std::function<int(Memory*, std::vector<std::size_t>)>>&
            builtin_functions) {
  Object type_data = memory[type];

  std::size_t size_value = GetUint64(memory + size);

  // If no size specified and not a class, default to allocating one element
  if ((type == 0 ||
       (type_data.type != 0x05 || type_data.data.string_data == nullptr)) &&
      size_value == 0)
    size_value = 1;

  // Prepare memory structures for the new object
  Memory* array_memory = new Memory();
  ClassMemory* class_memory = new ClassMemory();

  auto& data = array_memory->GetMemory();

  // Auto/Dynamic type.
  if (type == 0) {
    Object object;
    object.type = 0x00;
    object.constant_type = false;
    data.push_back(object);

  } else {
    // Class type.
    if (type_data.type == 0x05 && type_data.data.string_data != nullptr) {
      // Class only.
      if (size_value == 0) {
        std::string class_name = GetString(memory + type);
        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");
        Class& class_data = classes[class_name];
        
        class_memory->GetMembers() = class_data.GetMembers()->GetMembers();
        
        // Clear constant_type for all members (except special members) to allow mutation
        for (auto& member_pair : class_memory->GetMembers()) {
          // Skip special members like @name
          if (member_pair.first.length() > 0 && member_pair.first[0] != '@') {
            member_pair.second.constant_type = false;
          }
        }
        
        Object object;
        object.type = 0x09;
        object.data.class_data = class_memory;
        object.constant_type = true;
        data.push_back(object);

      } else {
        // Class array type.
        std::string class_name = GetString(memory + type);
        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");
        Class& class_data = classes[class_name];

        auto class_memory = new ClassMemory();
        *class_memory = *class_data.GetMembers();

        Object object;
        object.type = 0x09;
        object.data.class_data = class_memory;
        object.constant_type = false;
        data.push_back(object);
      }

    } else {
      // Array type.
      for (size_t i = 0; i < size_value; i++) {
        Object object;
        object.type = type_data.type;
        object.constant_type = true;
        data.push_back(object);
      }
    }
  }

  if (size_value == 0 && type_data.type == 0x05 &&
      type_data.data.string_data != nullptr) {
    InitObject(GetOrigin(memory + ptr), class_memory);
  } else {
    InitArray(GetOrigin(memory + ptr), array_memory);
  }

  return 0;
}

int ARRAY(
    Object* memory, std::size_t result, std::size_t ptr, std::size_t index,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions) {
  auto array_object = memory[ptr];
  auto array = array_object.data.array_data;

  auto index_object = memory[index];
  index = GetUint64(memory + index);

  if (index >= array->GetMemory().size()) array->GetMemory().resize(index + 1);

  SetReference(memory + result, ObjectReference{false, array, index});

  return 0;
}

int ADD(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) + GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) + GetLong(operand2_object));
      break;
    case 0x03:
      SetDouble(memory + result,
                GetDouble(operand1_object) + GetDouble(operand2_object));
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) + GetUint64(operand2_object));
      break;
    case 0x05:
      SetString(memory + result,
                GetString(operand1_object) + GetString(operand2_object));
      break;
    case 0x06: {
      auto& array1 = GetArray(operand1_object)->GetMemory();
      auto& array2 = GetArray(operand2_object)->GetMemory();
      std::vector<Object> new_array;
      new_array.reserve(array1.size() + array2.size());
      new_array.insert(new_array.end(), array1.begin(), array1.end());
      new_array.insert(new_array.end(), array2.begin(), array2.end());
      SetArrayContent(memory + result, new_array);
      break;
    }
    case 0x09:
      LOGGING_ERROR(
          "Custom ADD operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for addition: " +
                    std::to_string(type));
      break;
  }

  return 0;
}

int SUB(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;
  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) - GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) - GetLong(operand2_object));
      break;
    case 0x03:
      SetDouble(memory + result,
                GetDouble(operand1_object) - GetDouble(operand2_object));
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) - GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot subtract strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot subtract arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom SUB operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for subtraction: " +
                    std::to_string(type));
      break;
  }

  return 0;
}

int MUL(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) * GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) * GetLong(operand2_object));
      break;
    case 0x03:
      SetDouble(memory + result,
                GetDouble(operand1_object) * GetDouble(operand2_object));
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) * GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot multiply strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot multiply arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom MUL operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for multiplication: " +
                    std::to_string(type));
      break;
  }
  return 0;
}

int DIV(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) / GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) / GetLong(operand2_object));
      break;
    case 0x03:
      SetDouble(memory + result,
                GetDouble(operand1_object) / GetDouble(operand2_object));
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) / GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot divide strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot divide arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom DIV operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for division: " +
                    std::to_string(type));
      break;
  }
  return 0;
}

int REM(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) % GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) % GetLong(operand2_object));
      break;
    case 0x03:
      LOGGING_ERROR("Cannot calculate remainder for doubles.");
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) % GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot calculate remainder for strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot calculate remainder for arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom REM operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for remainder: " +
                    std::to_string(type));
      break;
  }
  return 0;
}
int NEG(Object* memory, std::size_t result, std::size_t operand1) {
  Object* operand1_object = GetOrigin(memory + operand1);
  std::size_t type = operand1_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result, -GetByte(operand1_object));
      break;
    case 0x02:
      SetLong(memory + result, -GetLong(operand1_object));
      break;
    case 0x03:
      SetDouble(memory + result, -GetDouble(operand1_object));
      break;
    case 0x04:
      SetUint64(memory + result, -GetUint64(operand1_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot negate strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot negate arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom NEG operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for negation: " +
                    std::to_string(type));
      break;
  }
  return 0;
}

int SHL(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result, GetByte(operand1_object)
                                   << GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result, GetLong(operand1_object)
                                   << GetLong(operand2_object));
      break;
    case 0x03:
      LOGGING_ERROR("Cannot shift doubles. Use multiplication for shifting.");
      break;
    case 0x04:
      SetUint64(memory + result, GetUint64(operand1_object)
                                     << GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot shift strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot shift arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom SHL operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for left shift: " +
                    std::to_string(type));
      break;
  }
  return 0;
}

int SHR(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) >> GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) >> GetLong(operand2_object));
      break;
    case 0x03:
      LOGGING_ERROR("Cannot shift doubles. Use multiplication for shifting.");
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) >> GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot shift strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot shift arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom SHR operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for right shift: " +
                    std::to_string(type));
      break;
  }
  return 0;
}

int REFER(Memory* memory, std::size_t result, std::size_t operand1) {
  if (result >= memory->GetMemory().size()) INTERNAL_ERROR("Out of memory.");

  Object& object = memory->GetMemory()[result];

  ObjectReference reference;
  reference.is_class = false;
  reference.memory.memory = memory;
  reference.index.index = operand1;

  SetReference(memory->GetMemory().data() + result, reference);

  return 0;
}

std::size_t IF(Object* memory, std::size_t condition, std::size_t true_branche,
               std::size_t false_branche) {
  auto condition_data = GetByte(memory + condition);

  if (condition_data != 0) return true_branche;

  return false_branche;
}

int AND(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) & GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) & GetLong(operand2_object));
      break;
    case 0x03:
      SetDouble(memory + result, static_cast<double>(GetByte(operand1_object) &
                                                     GetByte(operand2_object)));
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) & GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot perform AND operation on strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot perform AND operation on arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom AND operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for AND operation: " +
                    std::to_string(type));
      break;
  }

  return 0;
}

int OR(Object* memory, std::size_t result, std::size_t operand1,
       std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) | GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) | GetLong(operand2_object));
      break;
    case 0x03:
      SetDouble(memory + result, static_cast<double>(GetByte(operand1_object) |
                                                     GetByte(operand2_object)));
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) | GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot perform OR operation on strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot perform OR operation on arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom OR operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for OR operation: " +
                    std::to_string(type));
      break;
  }

  return 0;
}

int XOR(Object* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (type) {
    case 0x01:
      SetLong(memory + result,
              GetByte(operand1_object) ^ GetByte(operand2_object));
      break;
    case 0x02:
      SetLong(memory + result,
              GetLong(operand1_object) ^ GetLong(operand2_object));
      break;
    case 0x03:
      SetDouble(memory + result, static_cast<double>(GetByte(operand1_object) ^
                                                     GetByte(operand2_object)));
      break;
    case 0x04:
      SetUint64(memory + result,
                GetUint64(operand1_object) ^ GetUint64(operand2_object));
      break;
    case 0x05:
      LOGGING_ERROR("Cannot perform XOR operation on strings.");
      break;
    case 0x06:
      LOGGING_ERROR("Cannot perform XOR operation on arrays.");
      break;
    case 0x09:
      LOGGING_ERROR(
          "Custom XOR operator functions for classes are not currently "
          "supported.");
      break;
    default:
      LOGGING_ERROR("Unsupported data type for XOR operation: " +
                    std::to_string(type));
      break;
  }

  return 0;
}

int CMP(Object* memory, std::size_t result, std::size_t opcode,
        std::size_t operand1, std::size_t operand2) {
  Object* operand1_object = GetOrigin(memory + operand1);
  Object* operand2_object = GetOrigin(memory + operand2);

  std::size_t type = operand1_object->type > operand2_object->type
                         ? operand1_object->type
                         : operand2_object->type;

  switch (opcode) {
    case 0x00:  // ==
      switch (type) {
        case 0x01:
          SetByte(memory + result,
                  GetByte(operand1_object) == GetByte(operand2_object));
          return 0;
        case 0x02:
          SetByte(memory + result,
                  GetLong(operand1_object) == GetLong(operand2_object));
          return 0;
        case 0x03:
          SetByte(memory + result,
                  GetDouble(operand1_object) == GetDouble(operand2_object));
          return 0;
        case 0x04:
          SetByte(memory + result,
                  GetUint64(operand1_object) == GetUint64(operand2_object));
          return 0;
        case 0x05:
          if (memory[operand1].type != memory[operand2].type)
            LOGGING_ERROR("Cannot compare strings with different types.");
          SetByte(memory + result,
                  GetString(operand1_object) == GetString(operand2_object));
          return 0;
        case 0x06:
          if (memory[operand1].type != memory[operand2].type)
            LOGGING_ERROR("Cannot compare arrays with different types.");
          SetByte(memory + result,
                  GetArray(operand1_object) == GetArray(operand2_object));
          return 0;
        case 0x09:
          if (memory[operand1].type != memory[operand2].type)
            LOGGING_ERROR("Cannot compare classes with different types.");
          SetByte(memory + result,
                  GetObject(operand1_object) == GetObject(operand2_object));
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    case 0x01:  // !=
      switch (type) {
        case 0x01:
          SetByte(memory + result,
                  GetByte(operand1_object) != GetByte(operand2_object));
          return 0;
        case 0x02:
          SetByte(memory + result,
                  GetLong(operand1_object) != GetLong(operand2_object));
          return 0;
        case 0x03:
          SetByte(memory + result,
                  GetDouble(operand1_object) != GetDouble(operand2_object));
          return 0;
        case 0x04:
          SetByte(memory + result,
                  GetUint64(operand1_object) != GetUint64(operand2_object));
          return 0;
        case 0x05:
          if (memory[operand1].type != memory[operand2].type)
            LOGGING_ERROR("Cannot compare strings with different types.");
          SetByte(memory + result,
                  GetString(operand1_object) != GetString(operand2_object));
          return 0;
        case 0x06:
          if (memory[operand1].type != memory[operand2].type)
            LOGGING_ERROR("Cannot compare arrays with different types.");
          SetByte(memory + result,
                  GetArray(operand1_object) != GetArray(operand2_object));
          return 0;
        case 0x09:
          if (memory[operand1].type != memory[operand2].type)
            LOGGING_ERROR("Cannot compare classes with different types.");
          SetByte(memory + result,
                  GetObject(operand1_object) != GetObject(operand2_object));
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    case 0x02:  // >
      switch (type) {
        case 0x01:
          SetByte(memory + result,
                  GetByte(operand1_object) > GetByte(operand2_object));
          return 0;
        case 0x02:
          SetByte(memory + result,
                  GetLong(operand1_object) > GetLong(operand2_object));
          return 0;
        case 0x03:
          SetByte(memory + result,
                  GetDouble(operand1_object) > GetDouble(operand2_object));
          return 0;
        case 0x04:
          SetByte(memory + result,
                  GetUint64(operand1_object) > GetUint64(operand2_object));
          return 0;
        case 0x05:
          LOGGING_ERROR("Cannot compare strings with > operator.");
          return 0;
        case 0x06:
          LOGGING_ERROR("Cannot compare arrays with > operator.");
          return 0;
        case 0x09:
          LOGGING_ERROR("Cannot compare classes with > operator.");
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    case 0x03:  // >=
      switch (type) {
        case 0x01:
          SetByte(memory + result,
                  GetByte(operand1_object) >= GetByte(operand2_object));
          return 0;
        case 0x02:
          SetByte(memory + result,
                  GetLong(operand1_object) >= GetLong(operand2_object));
          return 0;
        case 0x03:
          SetByte(memory + result,
                  GetDouble(operand1_object) >= GetDouble(operand2_object));
          return 0;
        case 0x04:
          SetByte(memory + result,
                  GetUint64(operand1_object) >= GetUint64(operand2_object));
          return 0;
        case 0x05:
          LOGGING_ERROR("Cannot compare strings with >= operator.");
          return 0;
        case 0x06:
          LOGGING_ERROR("Cannot compare arrays with >= operator.");
          return 0;
        case 0x09:
          LOGGING_ERROR("Cannot compare classes with >= operator.");
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    case 0x04:  // <
      switch (type) {
        case 0x01:
          SetByte(memory + result,
                  GetByte(operand1_object) < GetByte(operand2_object));
          return 0;
        case 0x02:
          SetByte(memory + result,
                  GetLong(operand1_object) < GetLong(operand2_object));
          return 0;
        case 0x03:
          SetByte(memory + result,
                  GetDouble(operand1_object) < GetDouble(operand2_object));
          return 0;
        case 0x04:
          SetByte(memory + result,
                  GetUint64(operand1_object) < GetUint64(operand2_object));
          return 0;
        case 0x05:
          LOGGING_ERROR("Cannot compare strings with < operator.");
          return 0;
        case 0x06:
          LOGGING_ERROR("Cannot compare arrays with < operator.");
          return 0;
        case 0x09:
          LOGGING_ERROR("Cannot compare classes with < operator.");
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    case 0x05:  // <=
      switch (type) {
        case 0x01:
          SetByte(memory + result,
                  GetByte(operand1_object) <= GetByte(operand2_object));
          return 0;
        case 0x02:
          SetByte(memory + result,
                  GetLong(operand1_object) <= GetLong(operand2_object));
          return 0;
        case 0x03:
          SetByte(memory + result,
                  GetDouble(operand1_object) <= GetDouble(operand2_object));
          return 0;
        case 0x04:
          SetByte(memory + result,
                  GetUint64(operand1_object) <= GetUint64(operand2_object));
          return 0;
        case 0x05:
          LOGGING_ERROR("Cannot compare strings with <= operator.");
          return 0;
        case 0x06:
          LOGGING_ERROR("Cannot compare arrays with <= operator.");
          return 0;
        case 0x09:
          LOGGING_ERROR("Cannot compare classes with <= operator.");
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    default:
      LOGGING_ERROR("Unsupported comparison opcode: " + std::to_string(opcode));
      break;
  }

  return -1;
}

// EQUAL performs an assignment operation, copying a value from one memory
// location to another. This is the implementation of the '=' operator.
// It handles all AQ data types and performs automatic type-preserving copies.
//
// Parameters:
//   memory: Pointer to the memory array
//   result: Destination memory location for the assignment
//   value: Source memory location containing the value to copy
//
// Returns: 0 on success
int EQUAL(Object* memory, std::size_t result, std::size_t value) {
  // Dereference the source value if it's a reference
  Object* value_object = GetOrigin(memory + value);

  std::size_t type = value_object->type;

  // Copy the value based on its type
  switch (type) {
    case 0x00:
      // Handle uninitialized type - this can occur when a function doesn't
      // properly return a value or when using uninitialized variables.
      // Initialize the destination as an integer with value 0 for safety.
      LOGGING_WARNING(
          "Attempting to assign from uninitialized memory (type 0x00). "
          "Initializing destination as 0.");
      SetLong(memory + result, 0);
      break;
    case 0x01:  // Byte: copy int8_t value
      SetByte(memory + result, GetByte(value_object));
      break;
    case 0x02:  // Int: copy int64_t value
      SetLong(memory + result, GetLong(value_object));
      break;
    case 0x03:  // Float: copy double value
      SetDouble(memory + result, GetDouble(value_object));
      break;
    case 0x04:  // Uint64: copy uint64_t value
      SetUint64(memory + result, GetUint64(value_object));
      break;
    case 0x05:  // String: copy string value
      SetString(memory + result, GetString(value_object));
      break;
    case 0x06:
      SetArrayContent(memory + result, GetArray(value_object)->GetMemory());
      break;
    case 0x09:
      SetObject(memory + result, GetObject(value_object));
      break;
    case 0x0A:
      SetPtr(memory + result, GetPtr(value_object));
      break;
    default:
      LOGGING_ERROR("Unsupported data type for EQUAL operation: " +
                    std::to_string(value_object->type));
      break;
  }

  return 0;
}

std::size_t GOTO(Object* memory, std::size_t location) {
  return GetUint64(memory + location);
}

int INVOKE_METHOD(
    Memory* memory, std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<size_t> arguments) {
  if (arguments.size() < 3) INTERNAL_ERROR("Invalid arguments.");

  auto memory_ptr = memory->GetMemory().data();

  std::size_t class_object = arguments[0];
  std::size_t method_name_object = arguments[1];

  arguments.erase(arguments.begin(), arguments.begin() + 2);

  auto invoke_function =
      builtin_functions.find(GetString(memory_ptr + method_name_object));
  if (invoke_function != builtin_functions.end()) {
    return invoke_function->second(memory, arguments);
  }

  return InvokeClassMethod(memory, class_object, method_name_object, arguments,
                           classes, builtin_functions);
}

int InvokeClassMethod(
    Memory* memory, std::size_t class_object, std::size_t method_name_object,
    std::vector<size_t> arguments,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions) {
  auto memory_ptr = memory->GetMemory().data();

  auto class_members = GetObject(memory_ptr + class_object)->GetMembers();
  std::string class_name = *class_members["@name"].data.string_data;
  std::string method_name = GetString(memory_ptr + method_name_object);

  // Check if this is an imported module class (has @source_interpreter member)
  auto source_interp_it = class_members.find("@source_interpreter");
  if (source_interp_it != class_members.end() && source_interp_it->second.type == 0x0A) {
    // This is an imported module! We need to execute in the source interpreter
    // But first check if we're ALREADY in a cross-interpreter call to prevent infinite recursion
    static thread_local bool in_cross_interpreter_call = false;
    if (in_cross_interpreter_call) {
      // Already in cross-interpreter call, don't recurse
      LOGGING_ERROR("Recursive cross-interpreter call detected, aborting to prevent infinite loop");
      return -1;
    }
    
    in_cross_interpreter_call = true;
    LOGGING_INFO("Cross-interpreter invocation for method: " + method_name);
    
    Interpreter* source_interpreter = static_cast<Interpreter*>(source_interp_it->second.data.pointer_data);
    
    if (source_interpreter != nullptr) {
      // Get the source class name (without the ~import~ prefix)
      std::string source_class_name = ".!__start";  // Imported modules always use .!__start
      
      LOGGING_INFO("Source interpreter found, looking for class: " + source_class_name);
      
      // Find the method in the source interpreter's classes
      auto& source_classes = source_interpreter->classes;
      auto source_class_it = source_classes.find(source_class_name);
      if (source_class_it != source_classes.end()) {
        // Try to find the method with various naming conventions
        auto& methods = source_class_it->second.GetMethods();
        auto source_method_it = methods.find("." + method_name);
        if (source_method_it == methods.end()) {
          source_method_it = methods.find(method_name);
        }
        if (source_method_it == methods.end()) {
          // Try with .!__start prefix
          source_method_it = methods.find(".!__start." + method_name);
        }
        
        if (source_method_it != methods.end()) {
          // Found the method in source interpreter! Execute it there by calling InvokeClassMethod
          // recursively with the source interpreter's context
          
          auto source_memory = source_interpreter->global_memory;
          std::size_t source_method_name_idx = source_memory->AddString(method_name);
          
          // Build arguments for source: [return_ref, arg1, arg2, ...]
          // Create references in source memory pointing to local memory
          std::vector<std::size_t> source_arguments;
          std::size_t return_ref_idx = source_memory->AddReference(memory, arguments[0]);
          source_arguments.push_back(return_ref_idx);
          
          for (std::size_t i = 1; i < arguments.size(); i++) {
            std::size_t arg_ref_idx = source_memory->AddReference(memory, arguments[i]);
            source_arguments.push_back(arg_ref_idx);
          }
          
          // Call in source interpreter context (class index 2 is the main class)
          int result = InvokeClassMethod(source_memory, 2, source_method_name_idx,
                                        source_arguments, source_interpreter->classes,
                                        source_interpreter->builtin_functions);
          
          in_cross_interpreter_call = false;
          return result;
        }
      }
    }
    
    // If we get here, something went wrong with cross-interpreter invocation
    in_cross_interpreter_call = false;
    LOGGING_ERROR("Failed to invoke method '" + method_name + "' in source interpreter for class '" + class_name + "'");
    return -1;
  }

  // Normal (non-imported) method invocation
  auto class_it = classes.find(class_name);
  if (class_it == classes.end()) {
    LOGGING_ERROR("Class not found: " + class_name);
    return -1;
  }

  auto method_it = class_it->second.GetMethods().find(method_name);
  if (method_it == class_it->second.GetMethods().end()) {
    LOGGING_ERROR("Method not found: " + method_name);
    return -1;
  }

  Function method =
      SelectBestFunction(memory_ptr, method_it->second, arguments);

  auto function_arguments = method.GetParameters();

  // Return value.
  ObjectReference reference;
  reference.is_class = false;
  reference.memory.memory = memory;
  reference.index.index = arguments[0];
  SetReference(memory_ptr + function_arguments[0], reference);

  for (std::size_t i = 1;
       i < (method.IsVariadic() ? function_arguments.size() - 1
                                : function_arguments.size());
       i++) {
    auto argument_object = function_arguments[i];

    if (memory_ptr[function_arguments[i]].type == 0x07) {
      ObjectReference argument_reference;
      argument_reference.is_class = false;
      argument_reference.memory.memory = memory;
      argument_reference.index.index = arguments[i];
      SetReference(memory_ptr + argument_object, argument_reference);

    } else {
      if (memory_ptr[argument_object].constant_type) {
        switch (memory_ptr[argument_object].type) {
          case 0x01:
            SetByte(memory_ptr + argument_object,
                    GetByte(memory_ptr + arguments[i]));
            break;
          case 0x02:
            SetLong(memory_ptr + argument_object,
                    GetLong(memory_ptr + arguments[i]));
            break;
          case 0x03:
            SetDouble(memory_ptr + argument_object,
                      GetDouble(memory_ptr + arguments[i]));
            break;
          case 0x04:
            SetUint64(memory_ptr + argument_object,
                      GetUint64(memory_ptr + arguments[i]));
            break;
          case 0x05:
            SetString(memory_ptr + argument_object,
                      GetString(memory_ptr + arguments[i]));
            break;
          case 0x06:
            SetArrayContent(memory_ptr + argument_object,
                            GetArray(memory_ptr + arguments[i])->GetMemory());
            break;
          case 0x09:
            SetObject(memory_ptr + argument_object,
                      GetObject(memory_ptr + arguments[i]));
            break;
          default:
            LOGGING_ERROR("Unsupported data type for function argument: " +
                          std::to_string(memory_ptr[argument_object].type));
            return -1;
        }
      } else {
        auto& reference = arguments[i];
        switch (memory_ptr[reference].type) {
          case 0x01:
            SetByte(memory_ptr + argument_object,
                    GetByte(memory_ptr + reference));
            break;
          case 0x02:
            SetLong(memory_ptr + argument_object,
                    GetLong(memory_ptr + reference));
            break;
          case 0x03:
            SetDouble(memory_ptr + argument_object,
                      GetDouble(memory_ptr + reference));
            break;
          case 0x04:
            SetUint64(memory_ptr + argument_object,
                      GetUint64(memory_ptr + reference));
            break;
          case 0x05:
            SetString(memory_ptr + argument_object,
                      GetString(memory_ptr + reference));
            break;
          case 0x06:
            SetArrayContent(memory_ptr + argument_object,
                            GetArray(memory_ptr + reference)->GetMemory());
            break;
          case 0x09:
            SetObject(memory_ptr + argument_object,
                      GetObject(memory_ptr + reference));
            break;
          default:
            LOGGING_ERROR("Unsupported data type for function argument: " +
                          std::to_string(memory_ptr[reference].type));
            return -1;
        }
      }
    }
  }

  if (method.IsVariadic()) {
    auto array = new Memory();
    memory->GetMemory()[function_arguments.back()].type = 0x06;
    memory->GetMemory()[function_arguments.back()].constant_type = true;
    memory->GetMemory()[function_arguments.back()].data.array_data = array;
    for (std::size_t i = function_arguments.size() - 1; i < arguments.size();
         i++) {
      array->GetMemory().push_back(memory->GetOriginData(arguments[i]));
    }
  }

  const auto& instructions = method.GetCode();
  auto instructions_ptr = instructions.data();
  std::size_t instructions_size = instructions.size();

#if defined(__GNUC__) || defined(__clang__)
  static const void* dispatch_table[] = {
      &&op_NOP,  &&op_NOP,           &&op_NOP,         &&op_NEW,  &&op_ARRAY,
      &&op_NOP,  &&op_ADD,           &&op_SUB,         &&op_MUL,  &&op_DIV,
      &&op_REM,  &&op_NEG,           &&op_SHL,         &&op_SHR,  &&op_REFER,
      &&op_IF,   &&op_AND,           &&op_OR,          &&op_XOR,  &&op_CMP,
      &&op_NOP,  &&op_EQUAL,         &&op_GOTO,        &&op_NOP,  &&op_NOP,
      &&op_NOP,  &&op_INVOKE_METHOD, &&op_LOAD_MEMBER, &&op_ADDI, &&op_SUBI,
      &&op_MULI, &&op_DIVI,          &&op_REMI,        &&op_ADDF, &&op_SUBF,
      &&op_MULF, &&op_DIVF};
#endif

  struct Argument {
    std::size_t operand1 = 0;
    std::size_t operand2 = 0;
    std::size_t operand3 = 0;
    std::size_t operand4 = 0;
  };

  auto instructions_arguments = std::vector<Argument>(instructions_size);
  for (int64_t i = 0; i < instructions_size; i++) {
    const auto& instruction = instructions_ptr[i];
    const auto& arguments = instruction.arguments;
    auto& arg = instructions_arguments[i];

    if (arguments.size() <= 0) continue;
    arg.operand1 = arguments[0];
    if (arguments.size() <= 1) continue;
    arg.operand2 = arguments[1];
    if (arguments.size() <= 2) continue;
    arg.operand3 = arguments[2];
    if (arguments.size() <= 3) continue;
    arg.operand4 = arguments[3];
  }

  for (int64_t i = 0; i < instructions_size; i++) {
    const auto& instruction = instructions_ptr[i];
    const auto arguments = instructions_arguments[i];

#if defined(__GNUC__) || defined(__clang__)
    goto* dispatch_table[instruction.oper];

  op_NOP:
    NOP();
    continue;
  op_NEW:
    NEW(memory_ptr, classes, arguments.operand1, arguments.operand2,
        arguments.operand3, builtin_functions);
    continue;
  op_ARRAY:
    ARRAY(memory_ptr, arguments.operand1, arguments.operand2,
          arguments.operand3, classes, builtin_functions);
    continue;
  op_ADD:
    if (memory_ptr[arguments.operand1].type == 0x02 &&
        memory_ptr[arguments.operand2].type == 0x02 &&
        memory_ptr[arguments.operand3].type == 0x02) {
      int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
      const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
      const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
      *result = op1 + op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_ADDI;
      }
    } else if (memory_ptr[arguments.operand1].type == 0x03 &&
               memory_ptr[arguments.operand2].type == 0x03 &&
               memory_ptr[arguments.operand3].type == 0x03) {
      double* result = &memory_ptr[arguments.operand1].data.float_data;
      const double op1 = memory_ptr[arguments.operand2].data.float_data;
      const double op2 = memory_ptr[arguments.operand3].data.float_data;
      *result = op1 + op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_ADDF;
      }
    } else {
      ADD(memory_ptr, arguments.operand1, arguments.operand2,
          arguments.operand3);
    }
    continue;
  op_SUB:
    if (memory_ptr[arguments.operand1].type == 0x02 &&
        memory_ptr[arguments.operand2].type == 0x02 &&
        memory_ptr[arguments.operand3].type == 0x02) {
      int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
      const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
      const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
      *result = op1 - op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_SUBI;
      }
    } else if (memory_ptr[arguments.operand1].type == 0x03 &&
               memory_ptr[arguments.operand2].type == 0x03 &&
               memory_ptr[arguments.operand3].type == 0x03) {
      double* result = &memory_ptr[arguments.operand1].data.float_data;
      const double op1 = memory_ptr[arguments.operand2].data.float_data;
      const double op2 = memory_ptr[arguments.operand3].data.float_data;
      *result = op1 - op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_SUBF;
      }
    } else {
      SUB(memory_ptr, arguments.operand1, arguments.operand2,
          arguments.operand3);
    }
    continue;
  op_MUL:
    if (memory_ptr[arguments.operand1].type == 0x02 &&
        memory_ptr[arguments.operand2].type == 0x02 &&
        memory_ptr[arguments.operand3].type == 0x02) {
      int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
      const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
      const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
      *result = op1 * op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_MULI;
      }
    } else if (memory_ptr[arguments.operand1].type == 0x03 &&
               memory_ptr[arguments.operand2].type == 0x03 &&
               memory_ptr[arguments.operand3].type == 0x03) {
      double* result = &memory_ptr[arguments.operand1].data.float_data;
      const double op1 = memory_ptr[arguments.operand2].data.float_data;
      const double op2 = memory_ptr[arguments.operand3].data.float_data;
      *result = op1 * op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_MULF;
      }
    } else {
      MUL(memory_ptr, arguments.operand1, arguments.operand2,
          arguments.operand3);
    }
    continue;
  op_DIV:
    if (memory_ptr[arguments.operand1].type == 0x02 &&
        memory_ptr[arguments.operand2].type == 0x02 &&
        memory_ptr[arguments.operand3].type == 0x02) {
      int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
      const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
      const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
      *result = op1 / op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_DIVI;
      }
    } else if (memory_ptr[arguments.operand1].type == 0x03 &&
               memory_ptr[arguments.operand2].type == 0x03 &&
               memory_ptr[arguments.operand3].type == 0x03) {
      double* result = &memory_ptr[arguments.operand1].data.float_data;
      const double op1 = memory_ptr[arguments.operand2].data.float_data;
      const double op2 = memory_ptr[arguments.operand3].data.float_data;
      *result = op1 / op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_DIVF;
      }
    } else {
      DIV(memory_ptr, arguments.operand1, arguments.operand2,
          arguments.operand3);
    }
    continue;
  op_REM:
    if (memory_ptr[arguments.operand1].type == 0x02 &&
        memory_ptr[arguments.operand2].type == 0x02 &&
        memory_ptr[arguments.operand3].type == 0x02) {
      int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
      const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
      const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
      *result = op1 % op2;
      if (memory_ptr[arguments.operand1].constant_type &&
          memory_ptr[arguments.operand2].constant_type &&
          memory_ptr[arguments.operand3].constant_type) {
        method.GetCode()[i].oper = _AQVM_OPERATOR_REMI;
      }
    } else {
      REM(memory_ptr, arguments.operand1, arguments.operand2,
          arguments.operand3);
    }
    continue;
  op_NEG:
    if (memory_ptr[arguments.operand2].type == 0x02) {
      SetLong(memory_ptr + arguments.operand1,
              -memory_ptr[arguments.operand2].data.int_data);
    } else if (memory_ptr[arguments.operand2].type == 0x03) {
      SetDouble(memory_ptr + arguments.operand1,
                -memory_ptr[arguments.operand2].data.float_data);
    } else {
      NEG(memory_ptr, arguments.operand1, arguments.operand2);
    }
    continue;
  op_SHL:
    SHL(memory_ptr, arguments.operand1, arguments.operand2, arguments.operand3);
    continue;
  op_SHR:
    SHR(memory_ptr, arguments.operand1, arguments.operand2, arguments.operand3);
    continue;
  op_REFER:
    REFER(memory, arguments.operand1, arguments.operand2);
    continue;
  op_IF:
    i = IF(memory_ptr, arguments.operand1, arguments.operand2,
           arguments.operand3);
    i--;
    continue;
  op_AND:
    AND(memory_ptr, arguments.operand1, arguments.operand2, arguments.operand3);
    continue;
  op_OR:
    OR(memory_ptr, arguments.operand1, arguments.operand2, arguments.operand3);
    continue;
  op_XOR:
    XOR(memory_ptr, arguments.operand1, arguments.operand2, arguments.operand3);
    continue;
  op_CMP:
    CMP(memory_ptr, arguments.operand1, arguments.operand2, arguments.operand3,
        arguments.operand4);
    continue;
  op_EQUAL:
    if (memory_ptr[arguments.operand2].type == 0x02) {
      SetLong(memory_ptr + arguments.operand1,
              memory_ptr[arguments.operand2].data.int_data);
    } else if (memory_ptr[arguments.operand2].type == 0x03) {
      SetDouble(memory_ptr + arguments.operand1,
                memory_ptr[arguments.operand2].data.float_data);
    } else {
      EQUAL(memory_ptr, arguments.operand1, arguments.operand2);
    }
    continue;
  op_GOTO:
    i = GOTO(memory_ptr, arguments.operand1);
    i--;
    continue;
  op_INVOKE_METHOD: {
    auto origin_class_index = current_class_index;
    current_class_index = arguments.operand1;
    INVOKE_METHOD(memory, classes, builtin_functions,
                  instructions_ptr[i].arguments);
    current_class_index = origin_class_index;
    continue;
  }
  op_LOAD_MEMBER:
    if (arguments.operand2 == 0) {
      LOAD_MEMBER(memory, classes, arguments.operand1, current_class_index,
                  arguments.operand3);
    } else {
      LOAD_MEMBER(memory, classes, arguments.operand1, arguments.operand2,
                  arguments.operand3);
    }
    continue;

  op_ADDI: {
    int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
    const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
    const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
    *result = op1 + op2;
    continue;
  }
  op_SUBI: {
    int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
    const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
    const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
    *result = op1 - op2;
    continue;
  }
  op_MULI: {
    int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
    const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
    const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
    *result = op1 * op2;
    continue;
  }
  op_DIVI: {
    int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
    const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
    const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
    *result = op1 / op2;
    continue;
  }
  op_REMI: {
    int64_t* result = &memory_ptr[arguments.operand1].data.int_data;
    const int64_t op1 = memory_ptr[arguments.operand2].data.int_data;
    const int64_t op2 = memory_ptr[arguments.operand3].data.int_data;
    *result = op1 % op2;
    continue;
  }
  op_ADDF: {
    double* result = &memory_ptr[arguments.operand1].data.float_data;
    const double op1 = memory_ptr[arguments.operand2].data.float_data;
    const double op2 = memory_ptr[arguments.operand3].data.float_data;
    *result = op1 + op2;
    continue;
  }
  op_SUBF: {
    double* result = &memory_ptr[arguments.operand1].data.float_data;
    const double op1 = memory_ptr[arguments.operand2].data.float_data;
    const double op2 = memory_ptr[arguments.operand3].data.float_data;
    *result = op1 - op2;
    continue;
  }
  op_MULF: {
    double* result = &memory_ptr[arguments.operand1].data.float_data;
    const double op1 = memory_ptr[arguments.operand2].data.float_data;
    const double op2 = memory_ptr[arguments.operand3].data.float_data;
    *result = op1 * op2;
    continue;
  }
  op_DIVF: {
    double* result = &memory_ptr[arguments.operand1].data.float_data;
    const double op1 = memory_ptr[arguments.operand2].data.float_data;
    const double op2 = memory_ptr[arguments.operand3].data.float_data;
    *result = op1 / op2;
    continue;
  }
#else
    // LOGGING_INFO("Executing instruction: " +
    // std::to_string(instruction.oper));
    switch (instruction.oper) {
      case _AQVM_OPERATOR_NOP:
        NOP();
        break;
      case _AQVM_OPERATOR_NEW:
        NEW(memory_ptr, classes, arguments.operand1, arguments.operand2,
            arguments.operand3, builtin_functions);
        break;
      case _AQVM_OPERATOR_ARRAY:
        ARRAY(memory_ptr, arguments.operand1, arguments.operand2,
              arguments.operand3, classes, builtin_functions);
        break;
      case _AQVM_OPERATOR_ADD:
        if (memory_ptr[arguments.operand1].type == 0x02 &&
            memory_ptr[arguments.operand2].type == 0x02 &&
            memory_ptr[arguments.operand3].type == 0x02) {
          memory_ptr[arguments.operand1].data.int_data =
              memory_ptr[arguments.operand2].data.int_data +
              memory_ptr[arguments.operand3].data.int_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_ADDI;
          }
        } else if (memory_ptr[arguments.operand1].type == 0x03 &&
                   memory_ptr[arguments.operand2].type == 0x03 &&
                   memory_ptr[arguments.operand3].type == 0x03) {
          memory_ptr[arguments.operand1].data.float_data =
              memory_ptr[arguments.operand2].data.float_data +
              memory_ptr[arguments.operand3].data.float_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_ADDF;
          }
        } else {
          ADD(memory_ptr, arguments.operand1, arguments.operand2,
              arguments.operand3);
        }
        break;
      case _AQVM_OPERATOR_SUB:
        if (memory_ptr[arguments.operand1].type == 0x02 &&
            memory_ptr[arguments.operand2].type == 0x02 &&
            memory_ptr[arguments.operand3].type == 0x02) {
          memory_ptr[arguments.operand1].data.int_data =
              memory_ptr[arguments.operand2].data.int_data -
              memory_ptr[arguments.operand3].data.int_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_SUBI;
          }
        } else if (memory_ptr[arguments.operand1].type == 0x03 &&
                   memory_ptr[arguments.operand2].type == 0x03 &&
                   memory_ptr[arguments.operand3].type == 0x03) {
          memory_ptr[arguments.operand1].data.float_data =
              memory_ptr[arguments.operand2].data.float_data -
              memory_ptr[arguments.operand3].data.float_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_SUBF;
          }
        } else {
          SUB(memory_ptr, arguments.operand1, arguments.operand2,
              arguments.operand3);
        }
        break;
      case _AQVM_OPERATOR_MUL:
        if (memory_ptr[arguments.operand1].type == 0x02 &&
            memory_ptr[arguments.operand2].type == 0x02 &&
            memory_ptr[arguments.operand3].type == 0x02) {
          memory_ptr[arguments.operand1].data.int_data =
              memory_ptr[arguments.operand2].data.int_data *
              memory_ptr[arguments.operand3].data.int_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_MULI;
          }
        } else if (memory_ptr[arguments.operand1].type == 0x03 &&
                   memory_ptr[arguments.operand2].type == 0x03 &&
                   memory_ptr[arguments.operand3].type == 0x03) {
          memory_ptr[arguments.operand1].data.float_data =
              memory_ptr[arguments.operand2].data.float_data *
              memory_ptr[arguments.operand3].data.float_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_MULF;
          }
        } else {
          MUL(memory_ptr, arguments.operand1, arguments.operand2,
              arguments.operand3);
        }
        break;
      case _AQVM_OPERATOR_DIV:
        if (memory_ptr[arguments.operand1].type == 0x02 &&
            memory_ptr[arguments.operand2].type == 0x02 &&
            memory_ptr[arguments.operand3].type == 0x02) {
          memory_ptr[arguments.operand1].data.int_data =
              memory_ptr[arguments.operand2].data.int_data /
              memory_ptr[arguments.operand3].data.int_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_DIVI;
          }
        } else if (memory_ptr[arguments.operand1].type == 0x03 &&
                   memory_ptr[arguments.operand2].type == 0x03 &&
                   memory_ptr[arguments.operand3].type == 0x03) {
          memory_ptr[arguments.operand1].data.float_data =
              memory_ptr[arguments.operand2].data.float_data /
              memory_ptr[arguments.operand3].data.float_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_DIVF;
          }
        } else {
          DIV(memory_ptr, arguments.operand1, arguments.operand2,
              arguments.operand3);
        }
        break;
      case _AQVM_OPERATOR_REM:
        if (memory_ptr[arguments.operand1].type == 0x02 &&
            memory_ptr[arguments.operand2].type == 0x02 &&
            memory_ptr[arguments.operand3].type == 0x02) {
          memory_ptr[arguments.operand1].data.int_data =
              memory_ptr[arguments.operand2].data.int_data %
              memory_ptr[arguments.operand3].data.int_data;
          if (memory_ptr[arguments.operand1].constant_type &&
              memory_ptr[arguments.operand2].constant_type &&
              memory_ptr[arguments.operand3].constant_type) {
            method.GetCode()[i].oper = _AQVM_OPERATOR_REMI;
          }
        } else {
          REM(memory_ptr, arguments.operand1, arguments.operand2,
              arguments.operand3);
        }
        break;
      case _AQVM_OPERATOR_NEG:
        if (memory_ptr[arguments.operand2].type == 0x02) {
          SetLong(memory_ptr + arguments.operand1,
                  -memory_ptr[arguments.operand2].data.int_data);
        } else if (memory_ptr[arguments.operand2].type == 0x03) {
          SetDouble(memory_ptr + arguments.operand1,
                    -memory_ptr[arguments.operand2].data.float_data);
        } else {
          NEG(memory_ptr, arguments.operand1, arguments.operand2);
        }
        break;
      case _AQVM_OPERATOR_SHL:
        SHL(memory_ptr, arguments.operand1, arguments.operand2,
            arguments.operand3);
        break;
      case _AQVM_OPERATOR_SHR:
        SHR(memory_ptr, arguments.operand1, arguments.operand2,
            arguments.operand3);
        break;
      case _AQVM_OPERATOR_REFER:
        REFER(memory, arguments.operand1, arguments.operand2);
        break;
      case _AQVM_OPERATOR_IF:
        i = IF(memory_ptr, arguments.operand1, arguments.operand2,
               arguments.operand3);
        i--;
        break;
      case _AQVM_OPERATOR_AND:
        AND(memory_ptr, arguments.operand1, arguments.operand2,
            arguments.operand3);
        break;
      case _AQVM_OPERATOR_OR:
        OR(memory_ptr, arguments.operand1, arguments.operand2,
           arguments.operand3);
        break;
      case _AQVM_OPERATOR_XOR:
        XOR(memory_ptr, arguments.operand1, arguments.operand2,
            arguments.operand3);
        break;
      case _AQVM_OPERATOR_CMP:
        CMP(memory_ptr, arguments.operand1, arguments.operand2,
            arguments.operand3, arguments.operand4);
        break;
      case _AQVM_OPERATOR_EQUAL:
        if (memory_ptr[arguments.operand2].type == 0x02) {
          SetLong(memory_ptr + arguments.operand1,
                  memory_ptr[arguments.operand2].data.int_data);
        } else if (memory_ptr[arguments.operand2].type == 0x03) {
          SetDouble(memory_ptr + arguments.operand1,
                    memory_ptr[arguments.operand2].data.float_data);
        } else {
          EQUAL(memory_ptr, arguments.operand1, arguments.operand2);
        }
        break;
      case _AQVM_OPERATOR_GOTO:
        i = GOTO(memory_ptr, arguments.operand1);
        i--;
        break;
      case _AQVM_OPERATOR_INVOKE_METHOD: {
        auto origin_class_index = current_class_index;
        current_class_index = arguments[0];
        INVOKE_METHOD(memory, classes, builtin_functions,
                      instructions_ptr[i].arguments);
        current_class_index = origin_class_index;
        break;
      }
      case _AQVM_OPERATOR_LOAD_MEMBER:
        if (arguments[1] == 0) {
          LOAD_MEMBER(memory, classes, arguments.operand1, current_class_index,
                      arguments[2]);
        } else {
          LOAD_MEMBER(memory, classes, arguments.operand1, arguments.operand2,
                      arguments[2]);
        }
        break;

      case _AQVM_OPERATOR_ADDI:
        memory_ptr[arguments.operand1].data.int_data =
            memory_ptr[arguments.operand2].data.int_data +
            memory_ptr[arguments.operand3].data.int_data;
        break;
      case _AQVM_OPERATOR_SUBI:
        memory_ptr[arguments.operand1].data.int_data =
            memory_ptr[arguments.operand2].data.int_data -
            memory_ptr[arguments.operand3].data.int_data;
        break;
      case _AQVM_OPERATOR_MULI:
        memory_ptr[arguments.operand1].data.int_data =
            memory_ptr[arguments.operand2].data.int_data *
            memory_ptr[arguments.operand3].data.int_data;
        break;
      case _AQVM_OPERATOR_DIVI:
        memory_ptr[arguments.operand1].data.int_data =
            memory_ptr[arguments.operand2].data.int_data /
            memory_ptr[arguments.operand3].data.int_data;
        break;
      case _AQVM_OPERATOR_REMI:
        memory_ptr[arguments.operand1].data.int_data =
            memory_ptr[arguments.operand2].data.int_data %
            memory_ptr[arguments.operand3].data.int_data;
        break;
      case _AQVM_OPERATOR_ADDF:
        memory_ptr[arguments.operand1].data.float_data =
            memory_ptr[arguments.operand2].data.float_data +
            memory_ptr[arguments.operand3].data.float_data;
        break;
      case _AQVM_OPERATOR_SUBF:
        memory_ptr[arguments.operand1].data.float_data =
            memory_ptr[arguments.operand2].data.float_data -
            memory_ptr[arguments.operand3].data.float_data;
        break;
      case _AQVM_OPERATOR_MULF:
        memory_ptr[arguments.operand1].data.float_data =
            memory_ptr[arguments.operand2].data.float_data *
            memory_ptr[arguments.operand3].data.float_data;
        break;
      case _AQVM_OPERATOR_DIVF:
        memory_ptr[arguments.operand1].data.float_data =
            memory_ptr[arguments.operand2].data.float_data /
            memory_ptr[arguments.operand3].data.float_data;
        break;
      case _AQVM_OPERATOR_LOAD_MODULE_MEMBER:
        // Note: This requires special handling as it needs module interpreter pointers
        // which need to be passed through the execution context
        LOGGING_ERROR("LOAD_MODULE_MEMBER not yet implemented in bytecode execution loop");
        break;
      case _AQVM_OPERATOR_INVOKE_MODULE_METHOD:
        // Note: This requires special handling as it needs module interpreter pointers
        // which need to be passed through the execution context
        LOGGING_ERROR("INVOKE_MODULE_METHOD not yet implemented in bytecode execution loop");
        break;
      case _AQVM_OPERATOR_NEW_MODULE:
        // Note: This requires special handling as it needs module interpreter pointers
        // which need to be passed through the execution context
        LOGGING_ERROR("NEW_MODULE not yet implemented in bytecode execution loop");
        break;
      case _AQVM_OPERATOR_WIDE:
        break;
      default:
        LOGGING_ERROR("Unknown operator: " + std::to_string(instruction.oper));
        break;
    }
#endif
  }

  return 0;
}
Function SelectBestFunction(Object* memory, std::vector<Function>& functions,
                            std::vector<std::size_t>& arguments) {
  int64_t value = -1;
  Function best_function;
  bool has_same_value_function = false;
  for (auto& function : functions) {
    int64_t function_value =
        GetFunctionOverloadValue(memory, function, arguments);
    if (function_value > value) {
      best_function = function;
      value = function_value;
      has_same_value_function = false;
    } else if (function_value == value) {
      has_same_value_function = true;
    }
  }

  if (value == -1)
    LOGGING_ERROR("No function was found that matches the parameters.");
  if (has_same_value_function)
    LOGGING_WARNING(
        "There is a function overload conflict where multiple functions have "
        "parameters that meet the requirements and cannot be automatically "
        "implicitly determined.");

  return best_function;
}

int64_t GetFunctionOverloadValue(Object* memory, Function& function,
                                 std::vector<std::size_t>& arguments) {
  int64_t value = 0;
  if (function.IsVariadic()) {
    if (arguments.size() < function.GetParameters().size() - 1) return -1;
    return 0;

  } else {
    if (arguments.size() != function.GetParameters().size()) {
      return -1;
    }
  }

  for (std::size_t i = 1; i < function.GetParameters().size(); i++) {
    Object* argument = GetOrigin(memory + arguments[i]);
    Object* function_param = GetOrigin(memory + function.GetParameters()[i]);

    bool is_number = argument->type >= 0x01 && argument->type <= 0x04 &&
                     function_param->type >= 0x01 &&
                     function_param->type <= 0x04;

    // Conversion rules:
    // 1. The higher the return value, the more suitable it is for the
    // original type.
    // 2. 0x00 represents the worst value and is generally applicable to
    // automatic types.
    // 3. 0x01-0x04 indicates downgrade conversion, which may result in loss
    // of exact values.
    // 4. 0x05-0x08 indicates upgrade conversion, but for converting double to
    // uint64_t, the exact value may be lost.
    // 5. 0x09 represents the original same type.
    if (function_param->constant_type) {
      if (function_param->type == argument->type) {
        // Array type.
        if (function_param->type == 0x09) {
          if (*argument->data.class_data->GetMembers()["@name"]
                   .data.string_data !=
              *function_param->data.class_data->GetMembers()["@name"]
                   .data.string_data)
            return -1;
        }

        value += 0x09;
      } else if (function_param->type > argument->type && is_number) {
        value += 9 - (function_param->type - argument->type);
      } else if (function_param->type < argument->type && is_number) {
        value += 4 - (argument->type - function_param->type);
      } else {
        return -1;
      }
    }
  }

  // Used to avoid situations where the indefinite parameter function and the
  // objective function obtain the same value when all other parameters are
  // met.
  if (!function.IsVariadic()) value += 1;

  return value;
}

int LOAD_MEMBER(Memory* memory, std::unordered_map<std::string, Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand) {
  auto& result_reference = memory->GetMemory()[result];
  auto& class_object = memory->GetOriginData(class_index);

  if (class_object.type != 0x09)
    LOGGING_ERROR("class_index is not a class object.");

  auto& member_name_object = memory->GetMemory()[operand];
  if (member_name_object.type != 0x05)
    LOGGING_ERROR("operand is not a string.");

  ObjectReference* reference = new ObjectReference();
  reference->is_class = true;
  reference->memory.class_memory = class_object.data.class_data;
  reference->index.variable_name =
      new std::string(*member_name_object.data.string_data);

  result_reference.type = 0x07;
  result_reference.constant_type = true;
  result_reference.data.reference_data = reference;

  return 0;
}

// LOAD_MODULE_MEMBER loads a variable from another interpreter's memory by creating a reference
// This allows cross-module variable access without copying data
int LOAD_MODULE_MEMBER(Memory* local_memory, Memory* module_memory,
                       std::size_t result, std::size_t module_var_index) {
  if (local_memory == nullptr || module_memory == nullptr) {
    LOGGING_ERROR("Memory pointer is null.");
    return -1;
  }

  auto& result_object = local_memory->GetMemory()[result];
  
  // Create a reference to the module's variable
  ObjectReference* reference = new ObjectReference();
  reference->is_class = false;
  reference->memory.memory = module_memory;
  reference->index.index = module_var_index;

  result_object.type = 0x07;
  result_object.constant_type = false;
  result_object.data.reference_data = reference;

  return 0;
}

// INVOKE_MODULE_METHOD invokes a method from an imported module's interpreter
// All data is passed by reference to avoid errors when crossing interpreter boundaries
int INVOKE_MODULE_METHOD(
    Memory* local_memory, Memory* module_memory,
    std::unordered_map<std::string, Class>& module_classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        module_builtin_functions,
    std::vector<size_t> arguments) {
  if (arguments.size() < 3) {
    LOGGING_ERROR("Invalid arguments for INVOKE_MODULE_METHOD.");
    return -1;
  }

  // First argument is the class object in module memory
  std::size_t class_object_index = arguments[0];
  // Second argument is the method name in module memory
  std::size_t method_name_index = arguments[1];

  // Remove the first two arguments, keeping only the method parameters
  arguments.erase(arguments.begin(), arguments.begin() + 2);

  // Get the method name from module memory
  auto module_ptr = module_memory->GetMemory().data();
  std::string method_name = GetString(module_ptr + method_name_index);

  // Check if it's a builtin function
  auto builtin_it = module_builtin_functions.find(method_name);
  if (builtin_it != module_builtin_functions.end()) {
    // For builtin functions, arguments need to be in module memory
    // We need to create references in module memory to local memory
    std::vector<std::size_t> module_args;
    for (auto arg : arguments) {
      std::size_t ref_index = module_memory->AddReference(local_memory, arg);
      module_args.push_back(ref_index);
    }
    return builtin_it->second(module_memory, module_args);
  }

  // For class methods, invoke using the module's classes and memory
  return InvokeClassMethod(module_memory, class_object_index, method_name_index,
                          arguments, module_classes, module_builtin_functions);
}

// NEW_MODULE creates a new instance of a class from an imported module
// The instance is created in the module's memory space
int NEW_MODULE(Memory* local_memory, Memory* module_memory,
               std::unordered_map<std::string, Class>& module_classes,
               std::size_t result, std::size_t size, std::size_t type,
               std::unordered_map<
                   std::string,
                   std::function<int(Memory*, std::vector<std::size_t>)>>&
                   module_builtin_functions) {
  if (local_memory == nullptr || module_memory == nullptr) {
    LOGGING_ERROR("Memory pointer is null.");
    return -1;
  }

  // Get the type information from local memory
  auto local_ptr = local_memory->GetMemory().data();
  Object type_data = local_ptr[type];

  // The type should be a string naming the class
  if (type_data.type != 0x05 || type_data.data.string_data == nullptr) {
    LOGGING_ERROR("NEW_MODULE requires a string type parameter.");
    return -1;
  }

  std::string class_name = *type_data.data.string_data;
  
  // Get the size value
  std::size_t size_value = GetUint64(local_ptr + size);

  // Create the object in module memory
  std::size_t module_obj_index = module_memory->Add(1);
  
  // Use the module's NEW operator to create the instance
  std::size_t module_type_index = module_memory->AddString(class_name);
  std::size_t module_size_index = module_memory->AddUint64t(size_value);
  
  int result_code = NEW(module_memory->GetMemory().data(), module_classes,
                       module_obj_index, module_size_index, module_type_index,
                       module_builtin_functions);
  
  if (result_code != 0) {
    return result_code;
  }

  // Create a reference in local memory to the module object
  ObjectReference* reference = new ObjectReference();
  reference->is_class = false;
  reference->memory.memory = module_memory;
  reference->index.index = module_obj_index;

  auto& result_object = local_memory->GetMemory()[result];
  result_object.type = 0x07;
  result_object.constant_type = false;
  result_object.data.reference_data = reference;

  return 0;
}

}  // namespace Interpreter
}  // namespace Aq