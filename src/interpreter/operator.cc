// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/operator.h"

#include <memory>
#include <variant>

#include "logging/logging.h"
#include "memory.h"

namespace Aq {
namespace Interpreter {
std::size_t current_class_index = 2;

int NOP() { return 0; }

int NEW(Memory* memory, std::unordered_map<std::string, Class> classes,
        std::size_t ptr, std::size_t size, std::size_t type,
        std::unordered_map<
            std::string, std::function<int(Memory*, std::vector<std::size_t>)>>&
            builtin_functions) {
  Object type_data = memory->GetMemory()[type];

  std::size_t size_value = memory->GetUint64tData(size);

  if ((type == 0 || (type_data.type[0] != 0x05 ||
                     std::get<std::string>(type_data.data).empty())) &&
      size_value == 0)
    size_value = 1;

  Memory* array_memory = std::make_shared<Memory>();
  ClassMemory* class_memory = std::make_shared<ClassMemory>();

  auto& data = array_memory->GetMemory();

  // Auto/Dynamic type.
  if (type == 0) {
    data.push_back({{0x00}, type_data.data, false, false});

  } else {
    // Class type.
    if (type_data.type[0] == 0x05 &&
        !std::get<std::string>(type_data.data).empty()) {
      // Class only.
      if (size_value == 0) {
        data.push_back({{0x09}, class_memory, true, false});

        std::string class_name = memory->GetStringData(type);
        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");
        Class& class_data = classes[class_name];
        *class_memory = *class_data.GetMembers();

      } else {
        // Class array type.
        std::string class_name = memory->GetStringData(type);
        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");
        Class& class_data = classes[class_name];

        auto class_memory = std::make_shared<ClassMemory>();
        *class_memory = *class_data.GetMembers();

        data.push_back({{0x09}, class_memory, true, false});
      }

    } else {
      // Array type.
      for (size_t i = 0; i < size_value; i++) {
        data.push_back(
            {type_data.type, type_data.data, type_data.type[0] != 0x00, false});
      }
    }
  }

  if (size_value == 0 && type_data.type[0] == 0x05 &&
      !std::get<std::string>(type_data.data).empty()) {
    memory->SetObjectData(ptr, class_memory);
  } else {
    memory->SetArrayData(ptr, array_memory);
  }

  return 0;
}

int ARRAY(
    Memory* memory, std::size_t result, std::size_t ptr, std::size_t index,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions) {
  auto array_object = memory->GetOriginData(ptr);
  auto array = array_object.data.array_data;

  auto index_object = memory->GetOriginData(index);
  index = GetUint64(index_object);

  if (index >= array->GetMemory().size()) array->GetMemory().resize(index + 1);

  memory->GetMemory()[result].type = 0x07;
  memory->GetMemory()[result].data.reference_data =
      new ObjectReference{false, array, index};

  return 0;
}

int ADD(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);

  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);

  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;

  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) + GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) + GetLong(operand2_data));
      break;
    case 0x03:
      SetDouble(result_reference,
                GetDouble(operand1_data) + GetDouble(operand2_data));
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) + GetUint64(operand2_data));
      break;
    case 0x05:
      SetString(result_reference,
                GetString(operand1_data) + GetString(operand2_data));
      break;
    case 0x06: {
      auto& array1 = GetArray(operand1_data)->GetMemory();
      auto& array2 = GetArray(operand2_data)->GetMemory();
      std::vector<Object> new_array;
      new_array.reserve(array1.size() + array2.size());
      new_array.insert(new_array.end(), array1.begin(), array1.end());
      new_array.insert(new_array.end(), array2.begin(), array2.end());
      SetArray(result_reference, new_array);
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

int SUB(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);

  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);

  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) - GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) - GetLong(operand2_data));
      break;
    case 0x03:
      SetDouble(result_reference,
                GetDouble(operand1_data) - GetDouble(operand2_data));
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) - GetUint64(operand2_data));
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

int MUL(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);
  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) * GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) * GetLong(operand2_data));
      break;
    case 0x03:
      SetDouble(result_reference,
                GetDouble(operand1_data) * GetDouble(operand2_data));
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) * GetUint64(operand2_data));
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

int DIV(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);
  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) / GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) / GetLong(operand2_data));
      break;
    case 0x03:
      SetDouble(result_reference,
                GetDouble(operand1_data) / GetDouble(operand2_data));
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) / GetUint64(operand2_data));
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

int REM(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);
  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) % GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) % GetLong(operand2_data));
      break;
    case 0x03:
      LOGGING_ERROR("Cannot calculate remainder for doubles.");
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) % GetUint64(operand2_data));
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
int NEG(Memory* memory, std::size_t result, std::size_t operand1) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  std::size_t type = operand1_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference, -GetByte(operand1_data));
      break;
    case 0x02:
      SetLong(result_reference, -GetLong(operand1_data));
      break;
    case 0x03:
      SetDouble(result_reference, -GetDouble(operand1_data));
      break;
    case 0x04:
      SetUint64(result_reference, -GetUint64(operand1_data));
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

int SHL(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);
  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference, GetByte(operand1_data)
                                    << GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference, GetLong(operand1_data)
                                    << GetLong(operand2_data));
      break;
    case 0x03:
      LOGGING_ERROR("Cannot shift doubles. Use multiplication for shifting.");
      break;
    case 0x04:
      SetUint64(result_reference, GetUint64(operand1_data)
                                      << GetUint64(operand2_data));
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

int SHR(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);
  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) >> GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) >> GetLong(operand2_data));
      break;
    case 0x03:
      LOGGING_ERROR("Cannot shift doubles. Use multiplication for shifting.");
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) >> GetUint64(operand2_data));
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

  LOGGING_INFO("result: " + std::to_string(result) +
               " operand1: " + std::to_string(operand1));
  Object& object = memory->GetMemory()[result];
  LOGGING_INFO("object.guard_tag: " + std::to_string(object.guard_tag));
  if (object.constant_type) {
    if (object.type.empty() || object.type[0] != 0x07)
      LOGGING_ERROR(
          "Cannot change constant data type memory and unexpected type.");

    for (std::size_t i = 0; i < object.type.size(); i++) {
      if (object.type[i + 1] == 0x00) break;

      if (object.type[i + 1] != memory->GetOriginData(operand1).type[i])
        LOGGING_ERROR(
            "Cannot change constant data type memory and unexpected type.");
    }

    object.data = ObjectReference{memory, operand1};
  } else {
    object.type = {0x07, 0x00};
    object.data = ObjectReference{memory, operand1};
  }

  object.guard_tag = 0x07;
  object.guard_ptr = nullptr;

  return 0;
}

std::size_t IF(Memory* memory, std::size_t condition, std::size_t true_branche,
               std::size_t false_branche) {
  auto condition_data = GetByte(memory->GetOriginData(condition));

  if (condition_data != 0) return true_branche;

  return false_branche;
}

int AND(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);
  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) & GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) & GetLong(operand2_data));
      break;
    case 0x03:
      SetDouble(result_reference, static_cast<double>(GetByte(operand1_data) &
                                                      GetByte(operand2_data)));
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) & GetUint64(operand2_data));
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

int OR(Memory* memory, std::size_t result, std::size_t operand1,
       std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);
  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) | GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) | GetLong(operand2_data));
      break;
    case 0x03:
      SetDouble(result_reference, static_cast<double>(GetByte(operand1_data) |
                                                      GetByte(operand2_data)));
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) | GetUint64(operand2_data));
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

int XOR(Memory* memory, std::size_t result, std::size_t operand1,
        std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);

  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;
  switch (type) {
    case 0x01:
      SetLong(result_reference,
              GetByte(operand1_data) ^ GetByte(operand2_data));
      break;
    case 0x02:
      SetLong(result_reference,
              GetLong(operand1_data) ^ GetLong(operand2_data));
      break;
    case 0x03:
      SetDouble(result_reference, static_cast<double>(GetByte(operand1_data) ^
                                                      GetByte(operand2_data)));
      break;
    case 0x04:
      SetUint64(result_reference,
                GetUint64(operand1_data) ^ GetUint64(operand2_data));
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

int CMP(Memory* memory, std::size_t result, std::size_t opcode,
        std::size_t operand1, std::size_t operand2) {
  auto& result_reference = memory->GetOriginData(result);
  auto& operand1_data = memory->GetOriginData(operand1);
  auto& operand2_data = memory->GetOriginData(operand2);

  std::size_t type = operand1_data.guard_tag > operand2_data.guard_tag
                         ? operand1_data.guard_tag
                         : operand2_data.guard_tag;

  switch (opcode) {
    case 0x00:  // ==
      switch (type) {
        case 0x01:
          SetByte(result_reference,
                  GetByte(operand1_data) == GetByte(operand2_data));
          return 0;
        case 0x02:
          SetByte(result_reference,
                  GetLong(operand1_data) == GetLong(operand2_data));
          return 0;
        case 0x03:
          SetByte(result_reference,
                  GetDouble(operand1_data) == GetDouble(operand2_data));
          return 0;
        case 0x04:
          SetByte(result_reference,
                  GetUint64(operand1_data) == GetUint64(operand2_data));
          return 0;
        case 0x05:
          if (operand1_data.guard_tag != operand2_data.guard_tag)
            LOGGING_ERROR("Cannot compare strings with different types.");
          SetByte(result_reference,
                  GetString(operand1_data) == GetString(operand2_data));
          return 0;
        case 0x06:
          if (operand1_data.guard_tag != operand2_data.guard_tag)
            LOGGING_ERROR("Cannot compare arrays with different types.");
          SetByte(result_reference,
                  GetArray(operand1_data) == GetArray(operand2_data));
          return 0;
        case 0x09:
          if (operand1_data.guard_tag != operand2_data.guard_tag)
            LOGGING_ERROR("Cannot compare classes with different types.");
          SetByte(result_reference,
                  GetObject(operand1_data) == GetObject(operand2_data));
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    case 0x01:  // !=
      switch (type) {
        case 0x01:
          SetByte(result_reference,
                  GetByte(operand1_data) != GetByte(operand2_data));
          return 0;
        case 0x02:
          SetByte(result_reference,
                  GetLong(operand1_data) != GetLong(operand2_data));
          return 0;
        case 0x03:
          SetByte(result_reference,
                  GetDouble(operand1_data) != GetDouble(operand2_data));
          return 0;
        case 0x04:
          SetByte(result_reference,
                  GetUint64(operand1_data) != GetUint64(operand2_data));
          return 0;
        case 0x05:
          if (operand1_data.guard_tag != operand2_data.guard_tag)
            LOGGING_ERROR("Cannot compare strings with different types.");
          SetByte(result_reference,
                  GetString(operand1_data) != GetString(operand2_data));
          return 0;
        case 0x06:
          if (operand1_data.guard_tag != operand2_data.guard_tag)
            LOGGING_ERROR("Cannot compare arrays with different types.");
          SetByte(result_reference,
                  GetArray(operand1_data) != GetArray(operand2_data));
          return 0;
        case 0x09:
          if (operand1_data.guard_tag != operand2_data.guard_tag)
            LOGGING_ERROR("Cannot compare classes with different types.");
          SetByte(result_reference,
                  GetObject(operand1_data) != GetObject(operand2_data));
          return 0;
        default:
          LOGGING_ERROR("Unsupported data type for comparison: " +
                        std::to_string(type));
          return 0;
      }
    case 0x02:  // >
      switch (type) {
        case 0x01:
          SetByte(result_reference,
                  GetByte(operand1_data) > GetByte(operand2_data));
          return 0;
        case 0x02:
          SetByte(result_reference,
                  GetLong(operand1_data) > GetLong(operand2_data));
          return 0;
        case 0x03:
          SetByte(result_reference,
                  GetDouble(operand1_data) > GetDouble(operand2_data));
          return 0;
        case 0x04:
          SetByte(result_reference,
                  GetUint64(operand1_data) > GetUint64(operand2_data));
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
          SetByte(result_reference,
                  GetByte(operand1_data) >= GetByte(operand2_data));
          return 0;
        case 0x02:
          SetByte(result_reference,
                  GetLong(operand1_data) >= GetLong(operand2_data));
          return 0;
        case 0x03:
          SetByte(result_reference,
                  GetDouble(operand1_data) >= GetDouble(operand2_data));
          return 0;
        case 0x04:
          SetByte(result_reference,
                  GetUint64(operand1_data) >= GetUint64(operand2_data));
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
          SetByte(result_reference,
                  GetByte(operand1_data) < GetByte(operand2_data));
          return 0;
        case 0x02:
          SetByte(result_reference,
                  GetLong(operand1_data) < GetLong(operand2_data));
          return 0;
        case 0x03:
          SetByte(result_reference,
                  GetDouble(operand1_data) < GetDouble(operand2_data));
          return 0;
        case 0x04:
          SetByte(result_reference,
                  GetUint64(operand1_data) < GetUint64(operand2_data));
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
          SetByte(result_reference,
                  GetByte(operand1_data) <= GetByte(operand2_data));
          return 0;
        case 0x02:
          SetByte(result_reference,
                  GetLong(operand1_data) <= GetLong(operand2_data));
          return 0;
        case 0x03:
          SetByte(result_reference,
                  GetDouble(operand1_data) <= GetDouble(operand2_data));
          return 0;
        case 0x04:
          SetByte(result_reference,
                  GetUint64(operand1_data) <= GetUint64(operand2_data));
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

int EQUAL(Memory* memory, std::size_t result, std::size_t value) {
  LOGGING_INFO("EQUAL operation on memory at index: " + std::to_string(result) +
               " with value: " + std::to_string(value));
  if (memory->GetMemory()[result].type[0] == 0x07 &&
      !std::holds_alternative<Memory*>(memory->GetMemory()[result].data))
    return REFER(memory, result, value);

  auto& result_reference = memory->GetOriginData(result);
  LOGGING_INFO("a1");
  auto& value_data = memory->GetOriginData(value);
  LOGGING_INFO("a2");

  LOGGING_INFO("value_data.guard_tag: " + std::to_string(value_data.guard_tag));
  switch (value_data.guard_tag) {
    case 0x01:
      SetByte(result_reference, GetByte(value_data));
      break;
    case 0x02:
      SetLong(result_reference, GetLong(value_data));
      break;
    case 0x03:
      SetDouble(result_reference, GetDouble(value_data));
      break;
    case 0x04:
      SetUint64(result_reference, GetUint64(value_data));
      break;
    case 0x05:
      SetString(result_reference, GetString(value_data));
      break;
    case 0x06:
      SetArray(result_reference, GetArray(value_data)->GetMemory());
      break;
    case 0x09:
      SetObject(result_reference, GetObject(value_data));
      break;
    default:
      LOGGING_ERROR("Unsupported data type for EQUAL operation: " +
                    std::to_string(value_data.guard_tag));
      break;
  }

  return 0;
}

std::size_t GOTO(Memory* memory, std::size_t location) {
  Object& object = memory->GetOriginData(location);
  return GetUint64(object);
}

int INVOKE_METHOD(
    Memory* memory, std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<size_t> arguments) {
  if (arguments.size() < 3) INTERNAL_ERROR("Invalid arguments.");

  auto& class_object = memory->GetOriginData(arguments[0]);
  if (class_object.guard_tag != 0x09)
    LOGGING_ERROR("First argument must be a class object.");

  auto& method_name_object = memory->GetOriginData(arguments[1]);
  if (method_name_object.guard_tag != 0x05)
    LOGGING_ERROR("Second argument must be a string.");

  arguments.erase(arguments.begin(), arguments.begin() + 2);

  auto invoke_function = builtin_functions.find(GetString(method_name_object));
  if (invoke_function != builtin_functions.end()) {
    LOGGING_INFO("INVOKE BUILTIN FUNCTION.");
    return invoke_function->second(memory, arguments);
  }

  return InvokeClassMethod(memory, class_object, method_name_object, arguments,
                           classes, builtin_functions);
}

int InvokeClassMethod(
    Memory* memory, Object& class_object, Object& method_name_object,
    std::vector<size_t> arguments,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory*, std::vector<std::size_t>)>>&
        builtin_functions) {
  std::string class_name =
      GetString(GetObject(class_object)->GetMembers()["@name"]);
  std::string method_name = GetString(method_name_object);
  LOGGING_INFO("Invoking method: " + method_name + " on class: " + class_name);

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

  Function method = SelectBestFunction(memory, method_it->second, arguments);

  auto function_arguments = method.GetParameters();

  // Return value.
  memory->GetMemory()[function_arguments[0]].type = {0x07, 0x00};
  memory->GetMemory()[function_arguments[0]].guard_tag = 0x07;
  memory->GetMemory()[function_arguments[0]].guard_ptr = nullptr;
  memory->GetMemory()[function_arguments[0]].constant_type = true;
  memory->GetMemory()[function_arguments[0]].constant_data = false;
  memory->GetMemory()[function_arguments[0]].data =
      ObjectReference{memory, arguments[0]};
  LOGGING_INFO("Function arg index:" + std::to_string(function_arguments[0]) +
               " data: " + std::to_string(arguments[0]));

  for (std::size_t i = 1;
       i < (method.IsVariadic() ? function_arguments.size() - 1
                                : function_arguments.size());
       i++) {
    LOGGING_INFO("Function arg index:" + std::to_string(function_arguments[i]) +
                 " data: " + std::to_string(arguments[i]));

    auto& argument_object = memory->GetMemory()[function_arguments[i]];

    if (argument_object.type[0] == 0x07) {
      argument_object.data = ObjectReference{memory, arguments[i]};

    } else {
      if (argument_object.constant_type) {
        switch (argument_object.type[0]) {
          case 0x01:
            SetByte(argument_object,
                    GetByte(memory->GetOriginData(arguments[i])));
            break;
          case 0x02:
            SetLong(argument_object,
                    GetLong(memory->GetOriginData(arguments[i])));
            break;
          case 0x03:
            SetDouble(argument_object,
                      GetDouble(memory->GetOriginData(arguments[i])));
            break;
          case 0x04:
            SetUint64(argument_object,
                      GetUint64(memory->GetOriginData(arguments[i])));
            break;
          case 0x05:
            SetString(argument_object,
                      GetString(memory->GetOriginData(arguments[i])));
            break;
          case 0x06:
            SetArray(
                argument_object,
                GetArray(memory->GetOriginData(arguments[i]))->GetMemory());
            break;
          case 0x09:
            SetObject(argument_object,
                      GetObject(memory->GetOriginData(arguments[i])));
            break;
          default:
            LOGGING_ERROR("Unsupported data type for function argument: " +
                          std::to_string(argument_object.type[0]));
            return -1;
        }
      } else {
        auto& reference = memory->GetOriginData(arguments[i]);
        switch (reference.type[0]) {
          case 0x01:
            SetByte(argument_object, GetByte(reference));
            break;
          case 0x02:
            SetLong(argument_object, GetLong(reference));
            break;
          case 0x03:
            SetDouble(argument_object, GetDouble(reference));
            break;
          case 0x04:
            SetUint64(argument_object, GetUint64(reference));
            break;
          case 0x05:
            SetString(argument_object, GetString(reference));
            break;
          case 0x06:
            SetArray(argument_object, GetArray(reference)->GetMemory());
            break;
          case 0x09:
            SetObject(argument_object, GetObject(reference));
            break;
          default:
            LOGGING_ERROR("Unsupported data type for function argument: " +
                          std::to_string(reference.guard_tag));
            return -1;
        }
      }
    }
  }

  if (method.IsVariadic()) {
    auto array = std::make_shared<Memory>();
    memory->GetMemory()[function_arguments.back()].type = {0x06, 0x00};
    memory->GetMemory()[function_arguments.back()].guard_tag = 0x06;
    memory->GetMemory()[function_arguments.back()].guard_ptr = nullptr;
    memory->GetMemory()[function_arguments.back()].constant_type = true;
    memory->GetMemory()[function_arguments.back()].constant_data = false;
    memory->GetMemory()[function_arguments.back()].data = array;
    for (std::size_t i = function_arguments.size(); i < arguments.size(); i++) {
      LOGGING_INFO("Adding variadic argument: " + std::to_string(arguments[i]));
      array->GetMemory().push_back(memory->GetOriginData(arguments[i]));
    }
  }

  auto instructions = method.GetCode();

  for (int64_t i = 0; i < instructions.size(); i++) {
    LOGGING_INFO("Executing instruction " + std::to_string(i) + "/" +
                 std::to_string(instructions.size() - 1));
    LOGGING_INFO(
        "Instruction: " + std::to_string(instructions[i].GetOper()) +
        " | Memory size: " + std::to_string(memory->GetMemory().size()));
    auto instruction = instructions[i];
    auto arguments = instruction.GetArgs();
    switch (instruction.GetOper()) {
      case _AQVM_OPERATOR_NOP:
        NOP();
        break;
      case _AQVM_OPERATOR_LOAD:
        LOAD(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_STORE:
        STORE(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_NEW:
        NEW(memory, classes, arguments[0], arguments[1], arguments[2],
            builtin_functions);
        break;
      case _AQVM_OPERATOR_ARRAY:
        ARRAY(memory, arguments[0], arguments[1], arguments[2], classes,
              builtin_functions);
        break;
      case _AQVM_OPERATOR_PTR:
        PTR(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_ADD:
        ADD(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_SUB:
        SUB(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_MUL:
        MUL(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_DIV:
        DIV(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_REM:
        REM(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_NEG:
        NEG(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_SHL:
        SHL(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_SHR:
        SHR(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_REFER:
        REFER(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_IF:
        i = IF(memory, arguments[0], arguments[1], arguments[2]);
        i--;
        break;
      case _AQVM_OPERATOR_AND:
        AND(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_OR:
        OR(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_XOR:
        XOR(memory, arguments[0], arguments[1], arguments[2]);
        break;
      case _AQVM_OPERATOR_CMP:
        CMP(memory, arguments[0], arguments[1], arguments[2], arguments[3]);
        break;
      case _AQVM_OPERATOR_INVOKE:
        INVOKE(memory, builtin_functions, arguments, classes);
        break;
      case _AQVM_OPERATOR_EQUAL:
        EQUAL(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_GOTO:
        i = GOTO(memory, arguments[0]);
        i--;
        break;
      case _AQVM_OPERATOR_LOAD_CONST:
        LOAD_CONST(memory, memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_CONVERT:
        CONVERT(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_CONST:
        CONST(memory, arguments[0], arguments[1]);
        break;
      case _AQVM_OPERATOR_INVOKE_METHOD: {
        auto origin_class_index = current_class_index;
        current_class_index = arguments[0];
        INVOKE_METHOD(memory, classes, builtin_functions, arguments);
        current_class_index = origin_class_index;
        break;
      }
      case _AQVM_OPERATOR_LOAD_MEMBER:
        if (arguments[1] == 0) {
          LOAD_MEMBER(memory, classes, arguments[0], current_class_index,
                      arguments[2]);
        } else {
          LOAD_MEMBER(memory, classes, arguments[0], arguments[1],
                      arguments[2]);
        }
        break;
      case _AQVM_OPERATOR_WIDE:
        WIDE();
        break;
      default:
        LOGGING_ERROR("Unknown operator: " +
                      std::to_string(instruction.GetOper()));
        break;
    }
  }

  return 0;
}
Function SelectBestFunction(Memory* memory, std::vector<Function>& functions,
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

int64_t GetFunctionOverloadValue(Memory* memory, Function& function,
                                 std::vector<std::size_t>& arguments) {
  int64_t value = 0;
  if (function.IsVariadic()) {
    if (arguments.size() < function.GetParameters().size() - 1) return -1;
    return 0;

  } else {
    if (arguments.size() != function.GetParameters().size()) {
      LOGGING_INFO("Arg size: " + std::to_string(arguments.size()) +
                   " Func param size: " +
                   std::to_string(function.GetParameters().size()));
      return -1;
    }
  }

  for (std::size_t i = 1; i < function.GetParameters().size(); i++) {
    Object& argument = memory->GetOriginData(arguments[i]);
    Object& function_param = memory->GetOriginData(function.GetParameters()[i]);

    bool is_number = argument.guard_tag >= 0x01 && argument.guard_tag <= 0x04 &&
                     function_param.guard_tag >= 0x01 &&
                     function_param.guard_tag <= 0x04;

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
    if (function_param.constant_type) {
      if (function_param.guard_tag == argument.guard_tag) {
        // Array type.
        if (function_param.guard_tag == 0x06) {
          if (argument.type != argument.type) return -1;

          // Class Type.
        } else if (function_param.guard_tag == 0x09) {
          if (GetString(std::get<ClassMemory*>(argument.data)
                            ->GetMembers()["@name"]) !=
              GetString(std::get<ClassMemory*>(function_param.data)
                            ->GetMembers()["@name"]))
            return -1;
        }

        value += 0x09;
      } else if (function_param.guard_tag > argument.guard_tag && is_number) {
        value += 9 - (function_param.guard_tag - argument.guard_tag);
      } else if (function_param.guard_tag < argument.guard_tag && is_number) {
        value += 4 - (argument.guard_tag - function_param.guard_tag);
      } else {
        return -1;
      }
    } else if (memory->GetMemory()[function.GetParameters()[i]].type[0] ==
               0x07) {
      if (memory->GetMemory()[function.GetParameters()[i]].type.back() !=
          0x00) {
        if (memory->GetMemory()[function.GetParameters()[i]].type.size() - 1 !=
            argument.type.size())
          return -1;

        for (std::size_t j = 0; j < argument.type.size(); j++) {
          if (argument.type[j] !=
              memory->GetMemory()[function.GetParameters()[i]].type[j + 1]) {
            return -1;
          }
        }

        value += 0x07;
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

  if (class_object.guard_tag != 0x09)
    LOGGING_ERROR("LOAD_MEMBER: class_index is not a class object.");

  auto& member_name_object = memory->GetOriginData(operand);
  if (member_name_object.guard_tag != 0x05)
    LOGGING_ERROR("LOAD_MEMBER: operand is not a string.");

  result_reference.type = {0x07, 0x00};
  result_reference.constant_type = true;
  result_reference.constant_data = false;
  result_reference.guard_tag = 0x07;
  result_reference.guard_ptr = nullptr;
  result_reference.data = ObjectReference{
      std::get<ClassMemory*>(class_object.data), GetString(member_name_object)};

  return 0;
}

}  // namespace Interpreter
}  // namespace Aq