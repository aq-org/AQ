// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/operator.h"

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

int NEW(std::shared_ptr<Memory> memory,
        std::unordered_map<std::string, Class> classes, std::size_t ptr,
        std::size_t size, std::size_t type,
        std::unordered_map<
            std::string, std::function<int(Memory, std::vector<std::size_t>)>>&
            builtin_functions) {
 /* Object type_data = memory->GetOriginData(type);

  std::size_t size_value = memory->GetUint64tData(size);

  if ((type == 0 || (type_data.type[0] != 0x05 ||
                     std::get<std::string>(type_data.data).empty())) &&
      size_value == 0)
    size_value = 1;

  std::shared_ptr<Memory> array_memory = std::make_shared<Memory>();

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
        std::shared_ptr<ClassMemory> class_memory =
            std::make_shared<ClassMemory>();
        data.push_back({{0x09}, class_memory, true, false});

        std::string class_name = memory->GetStringData(type);
        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");
        Class& class_data = classes[class_name];
        *class_memory = *class_data.GetMembers();

      } else {
        // Class array type.
        for (size_t i = 1; i < size_value + 1; i++) {
          data[i].type.push_back(0x09);
          data[i].constant_type = true;

          std::string class_name = memory->GetStringData(type);
          if (classes.find(class_name) == classes.end())
            LOGGING_ERROR("class not found.");
          Class& class_data = classes[class_name];

          for (size_t j = 0; j < class_data.members.size(); j++) {
            class_object[j].type = class_data.members[j].type;
            class_object[j].constant_type = class_data.members[j].constant_type;
          }

          class_object.insert(class_object.begin(), {{0x05}, true, class_name});
        }
      }
    } else {
      // Array type.
      for (size_t i = 1; i < 2; i++) {
        data[i].type = type_data.type;
        data[i].constant_type = true;
      }
    }
  }

  if (size_value == 0 && type_data.type[0] == 0x05 &&
      !std::get<std::string>(type_data.data).empty()) {
    LOGGING_INFO("Name: " + GetStringData(heap, type));
    SetObjectData(heap, ptr, Memory::ObjectReference(class_object, 0));
  } else {
    SetArrayData(heap, ptr, Memory::ObjectReference(data, 0));
  }

  return 0;*/
}

int CrossMemoryNew(std::shared_ptr<Memory> memory,
                   std::unordered_map<std::string, Class> classes,
                   std::size_t ptr, std::size_t size, std::size_t type) {}

int ARRAY(
    std::shared_ptr<Memory> memory, std::size_t result, std::size_t ptr,
    std::size_t index, std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory, std::vector<std::size_t>)>>&
        builtin_functions,
    std::string& current_bytecode_file) {}

[[deprecated]] int PTR(std::shared_ptr<Memory> memory, std::size_t index,
                       std::size_t ptr) {
  return 0;
}

int ADD(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int SUB(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int MUL(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int DIV(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int REM(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}
int NEG(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1) {}

int SHL(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int SHR(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int REFER(std::shared_ptr<Memory> memory, std::size_t result,
          std::size_t operand1) {}

size_t IF(std::shared_ptr<Memory> memory, std::size_t condition,
          std::size_t true_branche, std::size_t false_branche) {}

int AND(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int OR(std::shared_ptr<Memory> memory, std::size_t result, std::size_t operand1,
       std::size_t operand2) {}

int XOR(std::shared_ptr<Memory> memory, std::size_t result,
        std::size_t operand1, std::size_t operand2) {}

int CMP(std::shared_ptr<Memory> memory, std::size_t result, std::size_t opcode,
        std::size_t operand1, std::size_t operand2) {}

int INVOKE(
    std::shared_ptr<Memory> memory,
    std::unordered_map<std::string,
                       std::function<int(Memory, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Class>& classes,
    std::string& current_bytecode_file) {}
int EQUAL(std::shared_ptr<Memory> memory, std::size_t result,
          std::size_t value) {}

int CrossMemoryEqual(std::shared_ptr<Memory> result_heap, std::size_t result,
                     std::shared_ptr<Memory> value_heap, std::size_t value) {}

size_t GOTO(std::shared_ptr<Memory> memory, std::size_t location) {}
int LOAD_CONST(std::shared_ptr<Memory> memory,
               std::shared_ptr<Memory> constant_pool, std::size_t object,
               std::size_t const_object) {}

[[deprecated]] int CONVERT(std::shared_ptr<Memory> memory, std::size_t result,
                           std::size_t operand1) {
  return 0;
}
[[deprecated]] int CONST(std::shared_ptr<Memory> memory, std::size_t result,
                         std::size_t operand1) {
  return 0;
}

int INVOKE_METHOD(
    std::shared_ptr<Memory> memory, std::string& current_bytecode_file,
    std::unordered_map<std::string, Class>& classes,
    std::unordered_map<std::string,
                       std::function<int(Memory, std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<size_t> arguments) {}

int LOAD_MEMBER(std::shared_ptr<Memory> memory,
                std::unordered_map<std::string, Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand) {}

int WIDE() {}
}  // namespace Interpreter
}  // namespace Aq