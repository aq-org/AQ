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

int NOP() { return 0; }

[[deprecated]] int LOAD(std::shared_ptr<Memory> memory, std::size_t ptr,
                        std::size_t operand) {
  return 0;
}

[[deprecated]] int STORE(std::shared_ptr<Memory> memory, std::size_t ptr,
                         std::size_t operand) {
  return 0;
}

int NEW(
    std::shared_ptr<Memory> memory,
    std::unordered_map<std::string, Class> classes, std::size_t ptr,
    std::size_t size, std::size_t type,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions) {
  Object type_data = memory->GetOriginData(type);

  std::size_t size_value = memory->GetUint64tData(size);

  if ((type == 0 || (type_data.type[0] != 0x05 ||
                     std::get<std::string>(type_data.data).empty())) &&
      size_value == 0)
    size_value = 1;

  std::shared_ptr<Memory> array_memory = std::make_shared<Memory>();
  std::shared_ptr<ClassMemory> class_memory = std::make_shared<ClassMemory>();

  auto& data = array_memory->GetMemory();

  // Auto/Dynamic type.
  if (type == 0) {
    data[0].type.push_back(0x00);
    data[0].constant_type = false;

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
        for (size_t i = 0; i < size_value; i++) {
          data[i].type.push_back(0x09);
          data[i].constant_type = true;

          std::string class_name = memory->GetStringData(type);
          if (classes.find(class_name) == classes.end())
            LOGGING_ERROR("class not found.");
          Class& class_data = classes[class_name];

          auto class_memory = std::make_shared<ClassMemory>();
          *class_memory = *class_data.GetMembers();
          data[i].data = class_memory;
        }
      }

    } else {
      // Array type.
      for (size_t i = 0; i < 1; i++) {
        data[i].type = type_data.type;
        data[i].constant_type = true;
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

[[deprecated]] int CrossMemoryNew(
    std::shared_ptr<Memory> memory,
    std::unordered_map<std::string, Class> classes, std::size_t ptr,
    std::size_t size, std::size_t type) {
  return 0;
}

int ARRAY(
    std::shared_ptr<Memory> memory, std::size_t result, std::size_t ptr,
    std::size_t index, std::unordered_map<std::string, Class>& classes,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions) {
  ObjectReference origin_data{memory, result};
  memory->GetLastReference(origin_data);
  Object& origin_reference =
      std::get<std::shared_ptr<Memory>>(origin_data.memory)
          ->GetMemory()[std::get<std::size_t>(origin_data.index)];

  auto array_object = memory->GetOriginData(ptr);
  auto array = std::get<std::shared_ptr<Memory>>(array_object.data);

  if (index >= array->GetMemory().size()) array->GetMemory().resize(index + 1);
  if (array->GetMemory()[index].type.empty()) {
    if (array->GetMemory()[0].constant_type) {
      array->GetMemory()[index].constant_type = true;
      array->GetMemory()[index].type = array->GetMemory()[0].type;

      // Class Type.
      if (array->GetMemory()[0].type[0] == 0x09) {
        std::string class_name = GetString(
            std::get<std::shared_ptr<ClassMemory>>(array->GetMemory()[0].data)
                ->GetMembers()["@name"]);
        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");

        Class& class_data = classes[class_name];
        array->GetMemory()[index].data = std::make_shared<ClassMemory>();
        *std::get<std::shared_ptr<ClassMemory>>(
            array->GetMemory()[index].data) = *class_data.GetMembers();
      }
    } else {
      array->GetMemory()[index].type.push_back(0x00);
      array->GetMemory()[index].constant_type = false;
      array->GetMemory()[index].constant_data = false;
    }
  }

  if (origin_reference.constant_type) {
    if (array->GetMemory()[index].constant_type)
      LOGGING_ERROR("Cannot change constant data type memory.");

    if (origin_reference.type.empty() || origin_reference.type[0] != 0x07)
      LOGGING_ERROR(
          "Cannot change constant data type memory and unexpected type.");

    if (origin_reference.type.size() - 1 !=
            array->GetMemory()[index].type.size() &&
        origin_reference.type.back() != 0x07)
      LOGGING_ERROR(
          "Cannot change constant data type memory and unexpected type.");

    for (std::size_t i = 0; i < origin_reference.type.size(); i++) {
      if (origin_reference.type[i + 1] == 0x00) break;

      if (origin_reference.type[i + 1] != array->GetMemory()[index].type[i])
        LOGGING_ERROR(
            "Cannot change constant data type memory and unexpected type.");
    }

    origin_reference.data = ObjectReference{array, index};
  } else {
    origin_reference.type = {0x07, 0x00};
    origin_reference.data = ObjectReference{array, index};
  }

  origin_reference.guard_tag = 0x07;
  origin_reference.guard_ptr = nullptr;

  return 0;
}

[[deprecated]] int PTR(std::shared_ptr<Memory> memory, std::size_t index,
                       std::size_t ptr) {
  return 0;
}

int ADD(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int SUB(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int MUL(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int DIV(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int REM(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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
int NEG(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1) {
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

int SHL(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int SHR(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int REFER(std::shared_ptr<Memory> memory, std::size_t result,
          std::size_t operand1) {
  if (result >= memory->GetMemory().size()) INTERNAL_ERROR("Out of memory.");

  Object& object = memory->GetOriginData(result);

  auto reference = ObjectReference{memory, operand1};
  memory->GetLastReference(reference);

  std::reference_wrapper<Object> operand1_reference =
      memory->GetOriginData(operand1);

  if (std::holds_alternative<std::shared_ptr<Memory>>(
          operand1_reference.get().data)) {
    operand1_reference =
        std::ref(std::get<std::shared_ptr<Memory>>(reference.memory)
                     ->GetMemory()[std::get<std::size_t>(reference.index)]);
  } else {
    operand1_reference =
        std::ref(std::get<std::shared_ptr<ClassMemory>>(reference.memory)
                     ->GetMembers()[std::get<std::string>(reference.index)]);
  }

  if (object.constant_type) {
    bool is_type_ok = false;
    if (!operand1_reference.get().constant_type)
      LOGGING_ERROR(
          "Cannot change constant data type memory and unexpected type.");

    if (object.type.empty() || object.type[0] != 0x07)
      LOGGING_ERROR(
          "Cannot change constant data type memory and unexpected type.");

    for (std::size_t i = 1, j = 0;
         i < object.type.size() && j < operand1_reference.get().type.size();
         i++, j++) {
      while (operand1_reference.get().type[j] == 0x07) j++;
      while (object.type[i] == 0x07) i++;
      if (object.type[i] == 0x00) {
        is_type_ok = true;
        break;
      }
      if (object.type[i] != operand1_reference.get().type[j])
        LOGGING_ERROR(
            "Cannot change constant data type memory and unexpected type.");
      if (i == object.type.size() - 1 &&
          j == operand1_reference.get().type.size() - 1)
        is_type_ok = true;
    }

    if (!is_type_ok)
      LOGGING_ERROR(
          "Cannot change constant data type memory and unexpected "
          "type.");

    object.data = reference;
  } else {
    object.type = {0x07, 0x00};
    object.data = reference;
  }

  object.guard_tag = 0x07;
  object.guard_ptr = nullptr;

  return 0;
}

std::size_t IF(std::shared_ptr<Memory> memory, std::size_t condition,
               std::size_t true_branche, std::size_t false_branche) {
  auto condition_data = GetByte(memory->GetOriginData(condition));

  if (condition_data != 0) return true_branche;

  return false_branche;
}

int AND(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int OR(std::shared_ptr<Memory> memory, std::size_t result, std::size_t operand1,
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

int XOR(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
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

int CMP(std::shared_ptr<Memory> memory, std::size_t result, std::size_t opcode,
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

[[deprecated]] int INVOKE(
    std::shared_ptr<Memory> memory,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Class>& classes) {
  return 0;
}
int EQUAL(std::shared_ptr<Memory> memory, std::size_t result,
          std::size_t value) {
  auto& result_reference = memory->GetOriginData(result);
  auto& value_data = memory->GetOriginData(value);
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

[[deprecated]] int CrossMemoryEqual(std::shared_ptr<Memory> result_heap,
                                    std::size_t result,
                                    std::shared_ptr<Memory> value_heap,
                                    std::size_t value) {
  return 0;
}

std::size_t GOTO(std::shared_ptr<Memory> memory, std::size_t location) {
  Object& object = memory->GetOriginData(location);
  return GetUint64(object);
}

[[deprecated]] int LOAD_CONST(std::shared_ptr<Memory> memory,
                              std::shared_ptr<Memory> constant_pool,
                              std::size_t object, std::size_t const_object) {
  return 0;
}

[[deprecated]] int CONVERT(std::shared_ptr<Memory> memory, std::size_t result,
                           std::size_t operand1) {
  return 0;
}
[[deprecated]] int CONST(std::shared_ptr<Memory> memory, std::size_t result,
                         std::size_t operand1) {
  return 0;
}

int INVOKE_METHOD(
    std::shared_ptr<Memory> memory,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
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
    return invoke_function->second(memory, arguments);
  }

  return InvokeClassMethod(memory, class_object, method_name_object, arguments,
                           classes, builtin_functions);
}

int InvokeClassMethod(
    std::shared_ptr<Memory> memory, Object& class_object,
    Object& method_name_object, std::vector<size_t> arguments,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<
        std::string,
        std::function<int(std::shared_ptr<Memory>, std::vector<std::size_t>)>>&
        builtin_functions) {
  std::string class_name =
      GetString(GetObject(class_object)->GetMembers()["@name"]);
  std::string method_name = GetString(method_name_object);

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

  for (std::size_t i = 0; i < function_arguments.size(); i++) {
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
        switch (reference.guard_tag) {
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
    memory->GetMemory()[function_arguments.back()].data = array;
    for (std::size_t i = function_arguments.size() - 2; i < arguments.size();
         i++) {
      array->GetMemory().push_back(memory->GetOriginData(arguments[i]));
    }
  }

  auto origin_class_data = memory->GetMemory()[2].data;
  memory->GetMemory()[2].data = class_object.data;

  auto instructions = method.GetCode();

  for (int64_t i = 0; i < instructions.size(); i++) {
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
      case _AQVM_OPERATOR_INVOKE_METHOD:
        INVOKE_METHOD(memory, classes, builtin_functions, arguments);
        break;
      case _AQVM_OPERATOR_LOAD_MEMBER:
        if (arguments[1] == 0) {
          LOAD_MEMBER(memory, classes, arguments[0], 2, arguments[2]);
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

  memory->GetMemory()[2].data = origin_class_data;

  return 0;
}
Function SelectBestFunction(std::shared_ptr<Memory> memory,
                            std::vector<Function>& functions,
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

int64_t GetFunctionOverloadValue(std::shared_ptr<Memory> memory,
                                 Function& function,
                                 std::vector<std::size_t>& arguments) {
  int64_t value = 0;
  if (function.IsVariadic()) {
    if (arguments.size() < function.GetParameters().size() - 1) {
      LOGGING_ERROR("Not enough arguments for variadic function.");
      return -1;
    }
  } else {
    if (arguments.size() != function.GetParameters().size()) {
      LOGGING_ERROR("Incorrect number of arguments for function.");
      return -1;
    }
  }

  for (std::size_t i = 0; i < function.GetParameters().size(); i++) {
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
          if (GetString(std::get<std::shared_ptr<ClassMemory>>(argument.data)
                            ->GetMembers()["@name"]) !=
              GetString(
                  std::get<std::shared_ptr<ClassMemory>>(function_param.data)
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

int LOAD_MEMBER(std::shared_ptr<Memory> memory,
                std::unordered_map<std::string, Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand) {
  auto& result_reference = memory->GetOriginData(result);
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
  result_reference.data =
      ObjectReference{std::get<std::shared_ptr<ClassMemory>>(class_object.data),
                      GetString(member_name_object)};

  return 0;
}

[[deprecated]] int WIDE() { return 0; }

int8_t GetByte(Object& object) {
  switch (object.guard_tag) {
    case 0x01:
      return *static_cast<int8_t*>(object.guard_ptr);
    case 0x02:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(*static_cast<int64_t*>(object.guard_ptr));
    case 0x03:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(*static_cast<double*>(object.guard_ptr));
    case 0x04:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int8_t>(*static_cast<uint64_t*>(object.guard_ptr));
    case 0x05:
      LOGGING_ERROR("Cannot get byte from string.");
    case 0x06:
      LOGGING_ERROR("Cannot get byte from array.");
    case 0x07:
      LOGGING_ERROR("Cannot get byte from reference.");
    case 0x09:
      LOGGING_ERROR("Cannot get byte from class.");
    case 0x0A:
      LOGGING_ERROR("Cannot get byte from pointer.");
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(object.guard_tag));
      break;
  }

  return 0;
}

void SetByte(Object& object, int8_t data) {
  if (object.guard_tag == 0x01) {
    *static_cast<int8_t*>(object.guard_ptr) = data;
    return;
  }

  if (object.constant_type) {
    switch (object.guard_tag) {
      case 0x01:
        *static_cast<int8_t*>(object.guard_ptr) = data;
        break;
      case 0x02:
        *static_cast<int64_t*>(object.guard_ptr) = data;
        break;
      case 0x03:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<double*>(object.guard_ptr) = data;
        break;
      case 0x04:
        *static_cast<uint64_t*>(object.guard_ptr) = data;
        break;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(object.guard_tag));
    }
  } else {
    object.type = {0x01};
    object.data = data;
    object.guard_tag = 0x01;
    object.guard_ptr = static_cast<void*>(&object.data);
  }
}

int64_t GetLong(Object& object) {
  switch (object.guard_tag) {
    case 0x01:
      return static_cast<int64_t>(*static_cast<int8_t*>(object.guard_ptr));
    case 0x02:
      return *static_cast<int64_t*>(object.guard_ptr);
    case 0x03:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int64_t>(*static_cast<double*>(object.guard_ptr));
    case 0x04:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<int64_t>(*static_cast<uint64_t*>(object.guard_ptr));
    case 0x05:
      LOGGING_ERROR("Cannot get long from string.");
    case 0x06:
      LOGGING_ERROR("Cannot get long from array.");
    case 0x07:
      LOGGING_ERROR("Cannot get long from reference.");
    case 0x09:
      LOGGING_ERROR("Cannot get long from class.");
    case 0x0A:
      LOGGING_ERROR("Cannot get long from pointer.");
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(object.guard_tag));
      break;
  }

  return 0;
}

void SetLong(Object& object, int64_t data) {
  if (object.guard_tag == 0x02) {
    *static_cast<int64_t*>(object.guard_ptr) = data;
    return;
  }

  if (object.constant_type) {
    switch (object.guard_tag) {
      case 0x01:
        *static_cast<int8_t*>(object.guard_ptr) = data;
        break;
      case 0x02:
        *static_cast<int64_t*>(object.guard_ptr) = data;
        break;
      case 0x03:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<double*>(object.guard_ptr) = data;
        break;
      case 0x04:
        *static_cast<uint64_t*>(object.guard_ptr) = data;
        break;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(object.guard_tag));
    }
  } else {
    object.type = {0x02};
    object.data = data;
    object.guard_tag = 0x02;
    object.guard_ptr = static_cast<void*>(&object.data);
  }
}

double GetDouble(Object& object) {
  switch (object.guard_tag) {
    case 0x01:
      return static_cast<double>(*static_cast<int8_t*>(object.guard_ptr));
    case 0x02:
      return static_cast<double>(*static_cast<int64_t*>(object.guard_ptr));
    case 0x03:
      return *static_cast<double*>(object.guard_ptr);
    case 0x04:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<double>(*static_cast<uint64_t*>(object.guard_ptr));
    case 0x05:
      LOGGING_ERROR("Cannot get double from string.");
    case 0x06:
      LOGGING_ERROR("Cannot get double from array.");
    case 0x07:
      LOGGING_ERROR("Cannot get double from reference.");
    case 0x09:
      LOGGING_ERROR("Cannot get double from class.");
    case 0x0A:
      LOGGING_ERROR("Cannot get double from pointer.");
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(object.guard_tag));
      break;
  }

  return 0.0;
}

void SetDouble(Object& object, double data) {
  if (object.guard_tag == 0x03) {
    *static_cast<double*>(object.guard_ptr) = data;
    return;
  }

  if (object.constant_type) {
    switch (object.guard_tag) {
      case 0x01:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<int8_t*>(object.guard_ptr) = data;
        break;
      case 0x02:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<int64_t*>(object.guard_ptr) = data;
        break;
      case 0x03:
        *static_cast<double*>(object.guard_ptr) = data;
        break;
      case 0x04:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<uint64_t*>(object.guard_ptr) = data;
        break;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(object.guard_tag));
    }
  } else {
    object.type = {0x03};
    object.data = data;
    object.guard_tag = 0x03;
    object.guard_ptr = static_cast<void*>(&object.data);
  }
}

uint64_t GetUint64(Object& object) {
  switch (object.guard_tag) {
    case 0x01:
      return static_cast<uint64_t>(*static_cast<int8_t*>(object.guard_ptr));
    case 0x02:
      return static_cast<uint64_t>(*static_cast<int64_t*>(object.guard_ptr));
    case 0x03:
      LOGGING_WARNING("Implicit conversion may changes value.");
      return static_cast<uint64_t>(*static_cast<double*>(object.guard_ptr));
    case 0x04:
      return *static_cast<uint64_t*>(object.guard_ptr);
    case 0x05:
      LOGGING_ERROR("Cannot get uint64 from string.");
    case 0x06:
      LOGGING_ERROR("Cannot get uint64 from array.");
    case 0x07:
      LOGGING_ERROR("Cannot get uint64 from reference.");
    case 0x09:
      LOGGING_ERROR("Cannot get uint64 from class.");
    case 0x0A:
      LOGGING_ERROR("Cannot get uint64 from pointer.");
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(object.guard_tag));
      break;
  }

  return 0;
}

void SetUint64(Object& object, uint64_t data) {
  if (object.guard_tag == 0x04) {
    *static_cast<uint64_t*>(object.guard_ptr) = data;
    return;
  }

  if (object.constant_type) {
    switch (object.guard_tag) {
      case 0x01:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<int8_t*>(object.guard_ptr) = data;
        break;
      case 0x02:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<int64_t*>(object.guard_ptr) = data;
        break;
      case 0x03:
        LOGGING_WARNING("Implicit conversion may changes value.");
        *static_cast<double*>(object.guard_ptr) = data;
        break;
      case 0x04:
        *static_cast<uint64_t*>(object.guard_ptr) = data;
        break;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(object.guard_tag));
    }
  } else {
    object.type = {0x04};
    object.data = data;
    object.guard_tag = 0x04;
    object.guard_ptr = static_cast<void*>(&object.data);
  }
}

std::string GetString(Object& object) {
  switch (object.guard_tag) {
    case 0x01:
      LOGGING_ERROR("Cannot get string from byte.");
    case 0x02:
      LOGGING_ERROR("Cannot get string from long.");
    case 0x03:
      LOGGING_ERROR("Cannot get string from double.");
    case 0x04:
      LOGGING_ERROR("Cannot get string from uint64.");
    case 0x05:
      return std::get<std::string>(object.data);
    case 0x06:
      LOGGING_ERROR("Cannot get string from array.");
    case 0x07:
      LOGGING_ERROR("Cannot get string from reference.");
    case 0x09:
      LOGGING_ERROR("Cannot get string from class.");
    case 0x0A:
      LOGGING_ERROR("Cannot get string from pointer.");
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(object.guard_tag));
      break;
  }

  return "";
}

void SetString(Object& object, const std::string& data) {
  if (object.guard_tag == 0x05) {
    std::get<std::string>(object.data) = data;
    return;
  }

  if (object.constant_type) {
    switch (object.guard_tag) {
      case 0x05:
        std::get<std::string>(object.data) = data;
        break;
      default:
        LOGGING_ERROR("Unsupported data type: " +
                      std::to_string(object.guard_tag));
    }
  } else {
    object.type = {0x05};
    object.data = data;
    object.guard_tag = 0x05;
    object.guard_ptr = static_cast<void*>(&object.data);
  }
}

std::shared_ptr<Memory> GetArray(Object& object) {
  switch (object.guard_tag) {
    case 0x01:
      LOGGING_ERROR("Cannot get array from byte.");
    case 0x02:
      LOGGING_ERROR("Cannot get array from long.");
    case 0x03:
      LOGGING_ERROR("Cannot get array from double.");
    case 0x04:
      LOGGING_ERROR("Cannot get array from uint64.");
    case 0x05:
      LOGGING_ERROR("Cannot get array from string.");
    case 0x06:
      return std::get<std::shared_ptr<Memory>>(object.data);
    case 0x07:
      LOGGING_ERROR("Cannot get array from reference.");
    case 0x09:
      LOGGING_ERROR("Cannot get array from class.");
    case 0x0A:
      LOGGING_ERROR("Cannot get array from pointer.");
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(object.guard_tag));
      break;
  }

  return nullptr;
}

void SetArray(Object& object, std::vector<Object>& data) {
  if (object.guard_tag == 0x06) {
    std::get<std::shared_ptr<Memory>>(object.data)->SetMemory(data);
    return;
  }

  if (object.constant_type) {
    LOGGING_ERROR("Cannot set array to constant type memory.");
  } else {
    object.type = {0x06};
    object.data = std::make_shared<Memory>();
    std::get<std::shared_ptr<Memory>>(object.data)->SetMemory(data);
    object.guard_tag = 0x06;
    object.guard_ptr = static_cast<void*>(&object.data);
  }
}

std::shared_ptr<ClassMemory> GetObject(Object& object) {
  switch (object.guard_tag) {
    case 0x01:
      LOGGING_ERROR("Cannot get object from byte.");
    case 0x02:
      LOGGING_ERROR("Cannot get object from long.");
    case 0x03:
      LOGGING_ERROR("Cannot get object from double.");
    case 0x04:
      LOGGING_ERROR("Cannot get object from uint64.");
    case 0x05:
      LOGGING_ERROR("Cannot get object from string.");
    case 0x06:
      LOGGING_ERROR("Cannot get object from array.");
    case 0x07:
      return std::get<std::shared_ptr<ClassMemory>>(object.data);
    case 0x09:
      return std::get<std::shared_ptr<ClassMemory>>(object.data);
    case 0x0A:
      LOGGING_ERROR("Cannot get object from pointer.");
    default:
      LOGGING_ERROR("Unsupported data type: " +
                    std::to_string(object.guard_tag));
      break;
  }

  return nullptr;
}

void SetObject(Object& object, std::shared_ptr<ClassMemory> data) {
  if (object.guard_tag == 0x09) {
    std::get<std::shared_ptr<ClassMemory>>(object.data) = data;
    return;
  }

  if (object.constant_type) {
    LOGGING_ERROR("Cannot set object to constant type memory.");
  } else {
    object.type = {0x09};
    object.data = data;
    object.guard_tag = 0x09;
    object.guard_ptr = static_cast<void*>(&object.data);
  }
}
}  // namespace Interpreter
}  // namespace Aq