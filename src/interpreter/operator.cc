// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/operator.h"

#include "logging/logging.h"
#include "memory.h"

namespace Aq {
namespace Interpreter {
std::size_t current_class_index = 2;
Object* global_memory_ptr = nullptr;

int NOP() { return 0; }

int NEW(Object* memory, std::unordered_map<std::string, Class> classes,
        std::size_t ptr, std::size_t size, std::size_t type,
        std::unordered_map<
            std::string, std::function<int(Memory*, std::vector<std::size_t>)>>&
            builtin_functions) {
  Object type_data = memory[type];

  std::size_t size_value = GetUint64(memory + size);

  if ((type == 0 ||
       (type_data.type != 0x05 || type_data.data.string_data == nullptr)) &&
      size_value == 0)
    size_value = 1;

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
        Object object;
        object.type = 0x09;
        object.data.class_data = new ClassMemory();
        object.constant_type = true;
        data.push_back(object);

        std::string class_name = GetString(memory + type);
        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");
        Class& class_data = classes[class_name];
        class_memory->GetMembers() = class_data.GetMembers()->GetMembers();

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

int EQUAL(Object* memory, std::size_t result, std::size_t value) {
  Object* value_object = GetOrigin(memory + value);

  std::size_t type = value_object->type;

  switch (type) {
    case 0x01:
      SetByte(memory + result, GetByte(value_object));
      break;
    case 0x02:
      SetLong(memory + result, GetLong(value_object));
      break;
    case 0x03:
      SetDouble(memory + result, GetDouble(value_object));
      break;
    case 0x04:
      SetUint64(memory + result, GetUint64(value_object));
      break;
    case 0x05:
      SetString(memory + result, GetString(value_object));
      break;
    case 0x06:
      SetArrayContent(memory + result, GetArray(value_object)->GetMemory());
      break;
    case 0x09:
      SetObject(memory + result, GetObject(value_object));
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

  std::string class_name = *GetObject(memory_ptr + class_object)
                                ->GetMembers()["@name"]
                                .data.string_data;
  std::string method_name = GetString(memory_ptr + method_name_object);

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
    for (std::size_t i = function_arguments.size(); i < arguments.size(); i++) {
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

}  // namespace Interpreter
}  // namespace Aq