// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/operator/operator.h"

#include <functional>
#include <string>

#include "vm/bytecode/bytecode.h"
#include "vm/logging/logging.h"
#include "vm/memory/memory.h"

namespace Aq {
namespace Vm {
namespace Operator {
int NOP() { return 0; }

int LOAD(std::vector<Memory::Object>& heap, std::size_t ptr,
         std::size_t operand) {
  LOGGING_WARNING("DEPRECATED OPERATOR.");
  return 0;
}

int STORE(std::vector<Memory::Object>& heap, std::size_t ptr,
          std::size_t operand) {
  LOGGING_WARNING("DEPRECATED OPERATOR.");
  return 0;
}

int NEW(std::vector<Memory::Object>& heap,
        std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
        std::string& current_bytecode_file,
        std::unordered_map<std::string, Bytecode::Class> classes,
        bool is_big_endian, std::size_t ptr, std::size_t size, std::size_t type,
        std::shared_ptr<Memory::Memory>& memory,
        std::unordered_map<std::string,
                           std::function<int(std::vector<Memory::Object>&,
                                             std::vector<std::size_t>)>>&
            builtin_functions) {
  if (ptr >= heap.size()) INTERNAL_ERROR("ptr is out of memory.");
  if (size >= heap.size()) INTERNAL_ERROR("size is out of memory.");

  // LOGGING_INFO("DEUBG");
  Memory::Object type_data = GetOriginData(heap, type);

  bool is_bytecode_file_main_program = false;
  // LOGGING_INFO("DEUBG");
  if (type_data.type[0] == 0x05 &&
      std::get<std::string>(type_data.data)[0] == '~') {
    std::string file_name = std::get<std::string>(type_data.data);
    std::string class_name;

    std::size_t start_pos = file_name.find('~');
    std::size_t end_pos = file_name.find('~', start_pos + 1);

    if (start_pos != std::string::npos && end_pos != std::string::npos &&
        start_pos < end_pos) {
      class_name = file_name.substr(end_pos + 1);
      file_name = file_name.substr(start_pos + 1, end_pos - start_pos - 1);
    } else {
      INTERNAL_ERROR("Bytecode file name is invalid.");
    }

    // TODO(bugs): May have bugs there.
    is_bytecode_file_main_program = true;

    // TODO(bugs): May have bugs there.
    if (class_name == ".!__start") {
      if (bytecode_files.find(class_name) != bytecode_files.end()) {
        heap[ptr].type.push_back(0x07);
        heap[ptr].const_type = false;
        // heap[ptr].data = Memory::ObjectReference(class_heap, 2);
        return 0;
      }

      Bytecode::AddBytecodeFile(std::get<std::string>(type_data.data).c_str(),
                                bytecode_files, is_big_endian, classes);
      if (bytecode_files.find(class_name) != bytecode_files.end()) {
        heap[ptr].data = Memory::ObjectReference(
            classes[std::get<std::string>(type_data.data)].memory->heap, 2);
        return 0;
      }
    }
  }
  // LOGGING_INFO("DEUBG");
  std::size_t size_value = GetUint64tData(heap, size);

  // LOGGING_INFO("DEUBG");
  if ((type == 0 || (type_data.type[0] != 0x05 ||
                     std::get<std::string>(type_data.data).empty())) &&
      size_value == 0)
    size_value = 1;

  // LOGGING_INFO("DEUBG");
  std::vector<Memory::Object> data(size_value);

  // LOGGING_INFO("DEUBG");

  auto class_object_ptr = new std::vector<Memory::Object>();
  auto& class_object = *class_object_ptr;
  if (type == 0) {
    data[0].type.push_back(0x00);
    data[0].const_type = false;
  } else {
    // LOGGING_INFO("1");
    if (type_data.type[0] == 0x05 &&
        !std::get<std::string>(type_data.data).empty()) {
      // LOGGING_INFO("2");
      if (size_value == 0) {
        data.resize(1);
        std::size_t i = 0;
        data[i].type.push_back(0x09);
        data[i].const_type = true;

        // LOGGING_INFO("3");
        std::string class_name = GetStringData(heap, type);

        // if (current_bytecode_file == "" && class_name[0] != '~')
        //   class_name = "~~" + class_name;

        // LOGGING_INFO("class_name: " + class_name);

        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");

        // LOGGING_INFO("4");
        Bytecode::Class& class_data = classes[class_name];

        class_object.resize(class_data.members.size());
        for (size_t j = 0; j < class_data.members.size(); j++) {
          class_object[j].type = class_data.members[j].type;
          class_object[j].const_type = class_data.members[j].const_type;
        }

        // LOGGING_INFO("5");
        class_object.insert(class_object.begin(), {{0x05}, true, class_name});
      } else {
        for (size_t i = 1; i < size_value + 1; i++) {
          data[i].type.push_back(0x09);
          data[i].const_type = true;

          // LOGGING_INFO("6");
          std::string class_name = GetStringData(heap, type);

          if (classes.find(class_name) == classes.end())
            LOGGING_ERROR("class not found.");

          Bytecode::Class& class_data = classes[class_name];

          for (size_t j = 0; j < class_data.members.size(); j++) {
            class_object[j].type = class_data.members[j].type;
            class_object[j].const_type = class_data.members[j].const_type;
          }

          class_object.insert(class_object.begin(), {{0x05}, true, class_name});
        }
      }
    } else {
      for (size_t i = 1; i < 2; i++) {
        data[i].type = type_data.type;
        data[i].const_type = true;
      }
    }
  }

  // LOGGING_INFO("8");
  if (size_value == 0 && type_data.type[0] == 0x05 &&
      !std::get<std::string>(type_data.data).empty()) {
    LOGGING_INFO("Name: " + GetStringData(heap, type));
    SetObjectData(heap, ptr, Memory::ObjectReference(class_object, 0));

    if (is_bytecode_file_main_program) {
      std::string class_name = GetStringData(heap, type);

      if (classes.find(class_name) == classes.end())
        LOGGING_ERROR("class not found.");

      Bytecode::Class& class_data = classes[class_name];
      class_data.memory->heap[2].data = Memory::ObjectReference(class_object, 0);

      heap.push_back({{0x05}, true, std::string("@constructor")});
      INVOKE_METHOD(heap, current_bytecode_file, classes, memory,
                    bytecode_files, builtin_functions, is_big_endian,
                    {2, heap.size() - 1, 1, 0});
      // heap.pop_back();
    }
  } else {
    SetArrayData(heap, ptr, Memory::ObjectReference(data, 0));
  }

  // LOGGING_INFO("DEUBG 9");
  return 0;
}

int CrossMemoryNew(std::shared_ptr<Memory::Memory> memory,
                   std::unordered_map<std::string, Bytecode::Class> classes,
                   std::size_t ptr, std::size_t size, std::size_t type) {
  if (ptr >= memory->heap.size()) INTERNAL_ERROR("ptr is out of memory.");
  if (size >= memory->heap.size()) INTERNAL_ERROR("size is out of memory.");

  Memory::Object type_data = GetOriginData(memory->heap, type);

  std::size_t size_value = GetUint64tData(memory->heap, size);

  if ((type == 0 || (type_data.type[0] != 0x05 ||
                     std::get<std::string>(type_data.data).empty())) &&
      size_value == 0)
    size_value = 1;

  std::vector<Memory::Object> data(size_value);

  if (type == 0) {
    for (size_t i = 1; i < 2; i++) {
      data[i].type.push_back(0x00);
      data[i].const_type = false;
    }
  } else {
    if (type_data.type[0] == 0x05 &&
        std::get<std::string>(type_data.data).empty()) {
      if (size_value == 0) {
        std::size_t i = 0;
        data[i].type.push_back(0x09);
        data[i].const_type = true;

        std::string class_name = GetStringData(memory->heap, type);

        if (classes.find(class_name) == classes.end())
          LOGGING_ERROR("class not found.");

        Bytecode::Class& class_data = classes[class_name];

        std::vector<Memory::Object> class_object(class_data.members.size());
        for (size_t j = 0; j < class_data.members.size(); j++) {
          class_object[j].type = class_data.members[j].type;
          class_object[j].const_type = class_data.members[j].const_type;
        }

        class_object.insert(class_object.begin(), {{0x05}, true, class_name});
      } else {
        for (size_t i = 1; i < size_value + 1; i++) {
          data[i].type.push_back(0x09);
          data[i].const_type = true;

          std::string class_name = GetStringData(memory->heap, type);

          if (classes.find(class_name) == classes.end())
            LOGGING_ERROR("class not found.");

          Bytecode::Class& class_data = classes[class_name];

          std::vector<Memory::Object> class_object(class_data.members.size());
          for (size_t j = 0; j < class_data.members.size(); j++) {
            class_object[j].type = class_data.members[j].type;
            class_object[j].const_type = class_data.members[j].const_type;
          }

          class_object.insert(class_object.begin(), {{0x05}, true, class_name});
        }
      }
    } else {
      for (size_t i = 1; i < 2; i++) {
        data[i].type = type_data.type;
        data[i].const_type = true;
      }
    }
  }

  if (size_value == 0 && type_data.type[0] == 0x05 &&
      !std::get<std::string>(type_data.data).empty()) {
    SetObjectData(memory->heap, ptr,
                  std::get<Memory::ObjectReference>(type_data.data));
  } else {
    SetArrayData(memory->heap, ptr, Memory::ObjectReference(data, 0));
  }
  return 0;
}

int ARRAY(
    std::vector<Memory::Object>& heap, std::size_t result, std::size_t ptr,
    std::size_t index,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<Memory::Object>&,
                                         std::vector<std::size_t>)>>&
        builtin_functions,
    std::string& current_bytecode_file, bool is_big_endian,
    std::shared_ptr<Memory::Memory>& memory) {
  // LOGGING_INFO("ARRAY operator called with result: " +
  //      std::to_string(result) + ", ptr: " + std::to_string(ptr) +
  //  ", index: " + std::to_string(index));
  auto& array = GetArrayData(heap, ptr);

  // LOGGING_INFO("Type: "+std::to_string(array[0].type[0]) );

  index = GetUint64tData(heap, index);
  // LOGGING_INFO("Index: " + std::to_string(index));

  if (index >= array.size()) {
    array.resize(index + 1);
  }

  bool is_type_null = array[index].type.size() == 0;

  if (is_type_null) {
    if (array[0].const_type) {
      array[index].const_type = true;
      array[index].type = array[0].type;
    } else {
      array[index].const_type = false;
      array[index].type.push_back(0x00);
    }
  }

  if (array[0].const_type && array[0].type[0] == 0x09 && is_type_null) {
    SetReferenceData(heap, result, array, index);
    Bytecode::InvokeCustomFunction(
        heap,
        std::get<std::string>(
            std::get<std::shared_ptr<Memory::Object>>(array[index].data)->data),
        {result}, classes, bytecode_files, builtin_functions,
        current_bytecode_file, is_big_endian, memory);
  }

  SetReferenceData(heap, result, array, index);

  // LOGGING_INFO("Set reference data at index " + std::to_string(result) +
  //     " with type " + std::to_string(heap[result].type[0]) + ".");

  // SetLongData(heap, result,0);
  // SetLongData(heap, result,0);

  return 0;
}

int PTR(std::vector<Memory::Object>& heap, std::size_t index, std::size_t ptr) {
  LOGGING_WARNING("DEPRECATED OPERATOR.");
  return 0;
}

int ADD(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if (GetByteData(heap, operand1) + GetByteData(heap, operand2) >
                INT8_MAX ||
            GetByteData(heap, operand1) + GetByteData(heap, operand2) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) + GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) + GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) + GetLongData(heap, operand2));
        break;
      case 0x03:
        SetDoubleData(
            heap, result,
            GetDoubleData(heap, operand1) + GetDoubleData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) + GetUint64tData(heap, operand2));
        break;
      case 0x05: {
        SetStringData(heap, result,
                      std::get<std::string>(operand1_data.data) +
                          std::get<std::string>(operand2_data.data));
        break;
      }
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR(
          "It is not supported to add strings directly to other types. Other "
          "types should be converted to strings through conversion functions.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) + GetLongData(heap, operand2));
        break;

      case 0x03:
        SetDoubleData(
            heap, result,
            GetDoubleData(heap, operand1) + GetDoubleData(heap, operand2));
        break;

      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) + GetUint64tData(heap, operand2));
        break;

      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int SUB(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if (GetByteData(heap, operand1) - GetByteData(heap, operand2) >
                INT8_MAX ||
            GetByteData(heap, operand1) - GetByteData(heap, operand2) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) - GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) - GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) - GetLongData(heap, operand2));
        break;
      case 0x03:
        SetDoubleData(
            heap, result,
            GetDoubleData(heap, operand1) - GetDoubleData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) - GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR(
          "It is not supported to add strings directly to other types. Other "
          "types should be converted to strings through conversion functions.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) - GetLongData(heap, operand2));
        break;

      case 0x03:
        SetDoubleData(
            heap, result,
            GetDoubleData(heap, operand1) - GetDoubleData(heap, operand2));
        break;

      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) - GetUint64tData(heap, operand2));
        break;

      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int MUL(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if (GetByteData(heap, operand1) * GetByteData(heap, operand2) >
                INT8_MAX ||
            GetByteData(heap, operand1) * GetByteData(heap, operand2) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) * GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) * GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) * GetLongData(heap, operand2));
        break;
      case 0x03:
        SetDoubleData(
            heap, result,
            GetDoubleData(heap, operand1) * GetDoubleData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) * GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) * GetLongData(heap, operand2));
        break;
      case 0x03:
        SetDoubleData(
            heap, result,
            GetDoubleData(heap, operand1) * GetDoubleData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) * GetUint64tData(heap, operand2));
        break;
      case 0x05:
        if (operand1_data.type[0] == 0x05) {
          std::string new_string;
          for (std::size_t i = 0; i < Memory::GetUint64tData(heap, operand2);
               i++)
            new_string += std::get<std::string>(operand1_data.data);
          SetStringData(heap, result, new_string);

        } else {
          std::string new_string;
          for (std::size_t i = 0; i < Memory::GetUint64tData(heap, operand1);
               i++)
            new_string += std::get<std::string>(operand2_data.data);
          SetStringData(heap, result, new_string);
        }
        break;

      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int DIV(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
    LOGGING_ERROR("Unsupported string type.");
  uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                            ? operand1_data.type[0]
                            : operand2_data.type[0];

  switch (result_type) {
    case 0x01:
      if (GetByteData(heap, operand1) / GetByteData(heap, operand2) >
              INT8_MAX ||
          GetByteData(heap, operand1) / GetByteData(heap, operand2) <
              INT8_MIN) {
        if (GetByteData(heap, operand1) % GetByteData(heap, operand2) == 0) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) / GetByteData(heap, operand2));
        } else {
          SetDoubleData(heap, result,
                        (double)GetByteData(heap, operand1) /
                            (double)GetByteData(heap, operand2));
        }
      } else {
        if (GetByteData(heap, operand1) % GetByteData(heap, operand2) == 0) {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) / GetByteData(heap, operand2));
        } else {
          SetDoubleData(heap, result,
                        (double)GetByteData(heap, operand1) /
                            (double)GetByteData(heap, operand2));
        }
      }
      break;
    case 0x02:
      if (GetLongData(heap, operand1) % GetLongData(heap, operand2) == 0) {
        SetLongData(heap, result,
                    GetLongData(heap, operand1) / GetLongData(heap, operand2));
      } else {
        SetDoubleData(heap, result,
                      (double)GetLongData(heap, operand1) /
                          (double)GetLongData(heap, operand2));
      }
      break;
    case 0x03:
      SetDoubleData(
          heap, result,
          GetDoubleData(heap, operand1) / GetDoubleData(heap, operand2));
      break;
    case 0x04:
      if (GetUint64tData(heap, operand1) / GetUint64tData(heap, operand2) >
          INT64_MAX) {
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) / GetUint64tData(heap, operand2));
      } else {
        if (GetUint64tData(heap, operand1) % GetUint64tData(heap, operand2) ==
            0) {
          SetLongData(
              heap, result,
              GetUint64tData(heap, operand1) / GetUint64tData(heap, operand2));
        } else {
          SetDoubleData(heap, result,
                        (double)GetUint64tData(heap, operand1) /
                            (double)GetUint64tData(heap, operand2));
        }
      }
      break;

    default:
      LOGGING_ERROR("Unsupported type.");
      break;
  }

  return 0;
}

int REM(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if (GetByteData(heap, operand1) % GetByteData(heap, operand2) >
                INT8_MAX ||
            GetByteData(heap, operand1) % GetByteData(heap, operand2) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) % GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) % GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) % GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) % GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR("Unsupported type.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) % GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) % GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int NEG(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);

  switch (operand1_data.type[0]) {
    case 0x01:
      SetByteData(heap, result, -GetByteData(heap, operand1));
      break;
    case 0x02:
      SetLongData(heap, result, -GetLongData(heap, operand1));
      break;
    case 0x03:
      SetDoubleData(heap, result, -GetDoubleData(heap, operand1));
      break;
    case 0x04:
      SetUint64tData(heap, result, -GetUint64tData(heap, operand1));
      break;
    default:
      LOGGING_ERROR("Unsupported type.");
      break;
  }

  return 0;
}

int SHL(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if (GetByteData(heap, operand1) << GetByteData(heap, operand2) >
                INT8_MAX ||
            GetByteData(heap, operand1) << GetByteData(heap, operand2) <
                INT8_MIN) {
          SetLongData(heap, result,
                      GetByteData(heap, operand1)
                          << GetByteData(heap, operand2));
        } else {
          SetByteData(heap, result,
                      GetByteData(heap, operand1)
                          << GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) << GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(heap, result,
                       GetUint64tData(heap, operand1)
                           << GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR("Unsupported type.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) << GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(heap, result,
                       GetUint64tData(heap, operand1)
                           << GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int SHR(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if (GetByteData(heap, operand1) >> GetByteData(heap, operand2) >
                INT8_MAX ||
            GetByteData(heap, operand1) >> GetByteData(heap, operand2) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) >> GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) >> GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) >> GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) >> GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR("Unsupported type.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) >> GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) >> GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int REFER(std::vector<Memory::Object>& heap, std::size_t result,
          std::size_t operand1) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  auto data = GetOriginDataReference(heap, operand1);

  SetReferenceData(heap, result, data.GetMemory(), data.GetIndex());

  return 0;
}

size_t IF(std::vector<Memory::Object>& heap, std::size_t condition,
          std::size_t true_branche, std::size_t false_branche) {
  if (GetByteData(heap, condition) != 0) {
    return true_branche;
  } else {
    return false_branche;
  }
}

int AND(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if ((GetByteData(heap, operand1) & GetByteData(heap, operand2)) >
                INT8_MAX ||
            (GetByteData(heap, operand1) & GetByteData(heap, operand2)) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) & GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) & GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) & GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) & GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR("Unsupported type.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) & GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) & GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int OR(std::vector<Memory::Object>& heap, std::size_t result,
       std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if ((GetByteData(heap, operand1) | GetByteData(heap, operand2)) >
                INT8_MAX ||
            (GetByteData(heap, operand1) | GetByteData(heap, operand2)) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) | GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) | GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) | GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) | GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR("Unsupported type.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) | GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) | GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int XOR(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  if (operand1_data.type[0] == operand2_data.type[0]) {
    switch (operand1_data.type[0]) {
      case 0x01:
        if ((GetByteData(heap, operand1) ^ GetByteData(heap, operand2)) >
                INT8_MAX ||
            (GetByteData(heap, operand1) ^ GetByteData(heap, operand2)) <
                INT8_MIN) {
          SetLongData(
              heap, result,
              GetByteData(heap, operand1) ^ GetByteData(heap, operand2));
        } else {
          SetByteData(
              heap, result,
              GetByteData(heap, operand1) ^ GetByteData(heap, operand2));
        }
        break;
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) ^ GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) ^ GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  } else {
    if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
      LOGGING_ERROR("Unsupported type.");
    uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                              ? operand1_data.type[0]
                              : operand2_data.type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(heap, result,
                    GetLongData(heap, operand1) ^ GetLongData(heap, operand2));
        break;
      case 0x04:
        SetUint64tData(
            heap, result,
            GetUint64tData(heap, operand1) ^ GetUint64tData(heap, operand2));
        break;
      default:
        LOGGING_ERROR("Unsupported type.");
        break;
    }
  }
  return 0;
}

int CMP(std::vector<Memory::Object>& heap, std::size_t result,
        std::size_t opcode, std::size_t operand1, std::size_t operand2) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand1 >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (operand2 >= heap.size()) INTERNAL_ERROR("Out of memory.");

  Memory::Object operand1_data = GetOriginData(heap, operand1);
  Memory::Object operand2_data = GetOriginData(heap, operand2);

  switch (opcode) {
    case 0x00:
      if (operand1_data.type[0] == operand2_data.type[0]) {
        switch (operand1_data.type[0]) {
          case 0x01:
            SetByteData(
                heap, result,
                GetByteData(heap, operand1) == GetByteData(heap, operand2));
            break;
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) == GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) == GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) ==
                            GetUint64tData(heap, operand2));
            break;
          case 0x05:
            SetByteData(
                heap, result,
                GetStringData(heap, operand1) == GetStringData(heap, operand2));
            break;
          case 0x06:
            SetByteData(
                heap, result,
                GetArrayData(heap, operand1) == GetArrayData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      } else {
        if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
          LOGGING_ERROR("Unsupported type.");
        uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                                  ? operand1_data.type[0]
                                  : operand2_data.type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) == GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) == GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) ==
                            GetUint64tData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      }
      break;
    case 0x01:
      if (operand1_data.type[0] == operand2_data.type[0]) {
        switch (operand1_data.type[0]) {
          case 0x01:
            SetByteData(
                heap, result,
                GetByteData(heap, operand1) != GetByteData(heap, operand2));
            break;
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) != GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) != GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) !=
                            GetUint64tData(heap, operand2));
            break;
          case 0x05:
            SetByteData(
                heap, result,
                GetStringData(heap, operand1) != GetStringData(heap, operand2));
            break;
          case 0x06:
            SetByteData(
                heap, result,
                GetArrayData(heap, operand1) != GetArrayData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      } else {
        if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
          LOGGING_ERROR("Unsupported type.");
        uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                                  ? operand1_data.type[0]
                                  : operand2_data.type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) != GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) != GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) !=
                            GetUint64tData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      }
      break;
    case 0x02:
      if (operand1_data.type[0] == operand2_data.type[0]) {
        switch (operand1_data.type[0]) {
          case 0x01:
            SetByteData(
                heap, result,
                GetByteData(heap, operand1) > GetByteData(heap, operand2));
            break;
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) > GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) > GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) >
                            GetUint64tData(heap, operand2));
            break;
          case 0x05:
            SetByteData(
                heap, result,
                GetStringData(heap, operand1) > GetStringData(heap, operand2));
            break;
          case 0x06:
            LOGGING_WARNING("Not supported array type.");
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      } else {
        if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
          LOGGING_ERROR("Unsupported type.");
        uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                                  ? operand1_data.type[0]
                                  : operand2_data.type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) > GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) > GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) >
                            GetUint64tData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      }
      break;
    case 0x03:
      if (operand1_data.type[0] == operand2_data.type[0]) {
        switch (operand1_data.type[0]) {
          case 0x01:
            SetByteData(
                heap, result,
                GetByteData(heap, operand1) >= GetByteData(heap, operand2));
            break;
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) >= GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) >= GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) >=
                            GetUint64tData(heap, operand2));
            break;
          case 0x05:
            SetByteData(
                heap, result,
                GetStringData(heap, operand1) >= GetStringData(heap, operand2));
            break;
          case 0x06:
            LOGGING_WARNING("Not supported array type.");
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      } else {
        if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
          LOGGING_ERROR("Unsupported type.");
        uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                                  ? operand1_data.type[0]
                                  : operand2_data.type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) >= GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) >= GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) >=
                            GetUint64tData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      }
      break;
    case 0x04:
      if (operand1_data.type[0] == operand2_data.type[0]) {
        switch (operand1_data.type[0]) {
          case 0x01:
            SetByteData(
                heap, result,
                GetByteData(heap, operand1) < GetByteData(heap, operand2));
            break;
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) < GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) < GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) <
                            GetUint64tData(heap, operand2));
            break;
          case 0x05:
            SetByteData(
                heap, result,
                GetStringData(heap, operand1) < GetStringData(heap, operand2));
            break;
          case 0x06:
            LOGGING_WARNING("Not supported array type.");
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      } else {
        if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
          LOGGING_ERROR("Unsupported type.");
        uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                                  ? operand1_data.type[0]
                                  : operand2_data.type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) < GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) < GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) <
                            GetUint64tData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      }
      break;
    case 0x05:
      if (operand1_data.type[0] == operand2_data.type[0]) {
        switch (operand1_data.type[0]) {
          case 0x01:
            SetByteData(
                heap, result,
                GetByteData(heap, operand1) <= GetByteData(heap, operand2));
            break;
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) <= GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) <= GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) <=
                            GetUint64tData(heap, operand2));
            break;
          case 0x05:
            SetByteData(
                heap, result,
                GetStringData(heap, operand1) <= GetStringData(heap, operand2));
            break;
          case 0x06:
            LOGGING_WARNING("Not supported array type.");
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      } else {
        if (operand1_data.type[0] == 0x05 || operand2_data.type[0] == 0x05)
          LOGGING_ERROR("Unsupported type.");
        uint8_t result_type = operand1_data.type[0] > operand2_data.type[0]
                                  ? operand1_data.type[0]
                                  : operand2_data.type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(
                heap, result,
                GetLongData(heap, operand1) <= GetLongData(heap, operand2));
            break;
          case 0x03:
            SetByteData(
                heap, result,
                GetDoubleData(heap, operand1) <= GetDoubleData(heap, operand2));
            break;
          case 0x04:
            SetByteData(heap, result,
                        GetUint64tData(heap, operand1) <=
                            GetUint64tData(heap, operand2));
            break;
          default:
            LOGGING_ERROR("Unsupported type.");
            break;
        }
      }
      break;
    default:
      LOGGING_ERROR("Invalid opcode.");
  }
  return 0;
}

int INVOKE(
    std::vector<Memory::Object>& heap,
    std::unordered_map<std::string,
                       std::function<int(std::vector<Memory::Object>&,
                                         std::vector<std::size_t>)>>&
        builtin_functions,
    std::vector<std::size_t> arguments,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::string& current_bytecode_file, bool is_big_endian,
    std::shared_ptr<Memory::Memory>& memory) {
  if (arguments.size() < 2) LOGGING_ERROR("Invalid arguments.");
  std::size_t function = arguments[0];
  arguments.erase(arguments.begin(), arguments.begin() + 1);
  auto invoke_function = builtin_functions.find(GetStringData(heap, function));
  if (invoke_function != builtin_functions.end()) {
    invoke_function->second(heap, arguments);
    return 0;
  }

  return Bytecode::InvokeCustomFunction(
      heap, GetStringData(heap, function), arguments, classes, bytecode_files,
      builtin_functions, current_bytecode_file, is_big_endian, memory);
}

int EQUAL(std::vector<Memory::Object>& heap, std::size_t result,
          std::size_t value) {
  if (result >= heap.size()) INTERNAL_ERROR("Out of memory.");
  if (value >= heap.size()) INTERNAL_ERROR("Out of memory.");

  if (result == 10) {
    SetLongData(heap, result, 0);
    return 0;
  }

  // LOGGING_INFO("EQUAL: result = {}, value = {}");

  Memory::Object value_data = GetOriginData(heap, value);

  // LOGGING_INFO("EQUAL: result = {}, value = {}");
  // LOGGING_INFO("EQUAL: value_data.type = {}");

  switch (value_data.type[0]) {
    case 0x00:
      break;
    case 0x01:
      // LOGGING_INFO("EQUAL: value_data.type = 0x01");
      SetByteData(heap, result, GetByteData(heap, value));
      break;
    case 0x02:
      // LOGGING_INFO("EQUAL: value_data.type = 0x02");

      SetLongData(heap, result, 0);
      SetLongData(heap, result, GetLongData(heap, value));
      break;
    case 0x03:
      // LOGGING_INFO("EQUAL: value_data.type = 0x03");
      SetDoubleData(heap, result, GetDoubleData(heap, value));
      break;
    case 0x04:
      // LOGGING_INFO("EQUAL: value_data.type = 0x04");
      SetUint64tData(heap, result, GetUint64tData(heap, value));
      break;
    case 0x05:
      // LOGGING_INFO("EQUAL: value_data.type = 0x05");
      SetStringData(heap, result, GetStringData(heap, value));
      break;
    case 0x06:
      // LOGGING_INFO("EQUAL: value_data.type = 0x06");
      SetArrayData(heap, result,
                   Memory::ObjectReference(GetArrayData(heap, value), 0));
      break;
    case 0x09:
      // LOGGING_INFO("EQUAL: value_data.type = 0x09");
      SetObjectData(heap, result,
                    Memory::ObjectReference(GetObjectData(heap, value), 0));
      break;
    case 0x0A:
      // SetOriginData(heap, result, GetPointerData(heap, value));
      LOGGING_WARNING("Not supported origin type yet.");
      break;
    default:
      LOGGING_ERROR("Unsupported type.");
  }

  // LOGGING_INFO("EQUAL: result = {}, value = {}");
  return 0;
}

int CrossMemoryEqual(std::vector<Memory::Object>& result_heap,
                     std::size_t result,
                     std::vector<Memory::Object>& value_heap,
                     std::size_t value) {
  if (result >= result_heap.size()) INTERNAL_ERROR("Out of object_table_size.");
  if (value >= value_heap.size()) INTERNAL_ERROR("Out of object_table_size.");

  Memory::Object value_data = GetOriginData(value_heap, value);

  switch (value_data.type[0]) {
    case 0x00:
      break;
    case 0x01:
      SetByteData(result_heap, result, GetByteData(value_heap, value));
      break;
    case 0x02:
      SetLongData(result_heap, result, GetLongData(value_heap, value));
      break;
    case 0x03:
      SetDoubleData(result_heap, result, GetDoubleData(value_heap, value));
      break;
    case 0x04:
      SetUint64tData(result_heap, result, GetUint64tData(value_heap, value));
      break;
    case 0x05:
      SetStringData(result_heap, result, GetStringData(value_heap, value));
      break;
    case 0x06:
      SetArrayData(result_heap, result,
                   Memory::ObjectReference(GetArrayData(value_heap, value), 0));
      break;
    case 0x09:
      SetObjectData(
          result_heap, result,
          Memory::ObjectReference(GetObjectData(value_heap, value), 0));
      break;
    case 0x0A:
      // SetOriginData(result_heap, result,
      //               GetOriginData(value_memory->object_table + value));
      LOGGING_WARNING("Not supported origin type yet.");
      break;
    default:
      LOGGING_ERROR("Unsupported type.");
  }
  return 0;
}

size_t GOTO(std::vector<Memory::Object>& heap, std::size_t location) {
  return GetUint64tData(heap, location);
}

int LOAD_CONST(std::vector<Memory::Object>& heap,
               std::vector<Memory::Object>& constant_pool, std::size_t object,
               std::size_t const_object) {
  if (object >= heap.size()) INTERNAL_ERROR("Out of object_table_size.");
  if (const_object >= constant_pool.size())
    INTERNAL_ERROR("Out of constant_pool_size.");

  switch (constant_pool[const_object].type[0]) {
    case 0x01:
      SetByteData(heap, object,
                  std::get<int8_t>(constant_pool[const_object].data));
      break;
    case 0x02:
      SetLongData(heap, object,
                  std::get<int64_t>(constant_pool[const_object].data));
      break;
    case 0x03:
      SetDoubleData(heap, object,
                    std::get<double>(constant_pool[const_object].data));
      break;
    case 0x04:
      SetUint64tData(heap, object,
                     std::get<uint64_t>(constant_pool[const_object].data));
      break;
    case 0x05:
      SetStringData(heap, object,
                    std::get<std::string>(constant_pool[const_object].data));
      break;
    case 0x06:
      SetArrayData(
          heap, object,
          std::get<Memory::ObjectReference>(constant_pool[const_object].data));
      break;
    default:
      LOGGING_ERROR("Unsupported type.");
  }

  return 0;
}

int CONVERT(std::vector<Memory::Object>& heap, std::size_t result,
            std::size_t operand1) {
  LOGGING_WARNING("DEPRECATED OPERATOR.");
  return 0;
}

int CONST(std::vector<Memory::Object>& heap, std::size_t result,
          std::size_t operand1) {
  if (result >= heap.size()) LOGGING_ERROR("Out of object_table_size.");
  if (operand1 >= heap.size()) LOGGING_ERROR("Out of object_table_size.");

  SetConstData(heap, result, heap, operand1);
  return 0;
}

int INVOKE_METHOD(
    std::vector<Memory::Object>& heap, std::string& current_bytecode_file,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::shared_ptr<Memory::Memory>& memory,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<Memory::Object>&,
                                         std::vector<std::size_t>)>>&
        builtin_functions,
    bool is_big_endian, std::vector<size_t> arguments) {
  LOGGING_INFO("INVOKE_METHOD: arguments size = " +
               std::to_string(arguments.size()));
  if (arguments.size() < 3) INTERNAL_ERROR("Invalid arguments.");

  LOGGING_INFO("DEBUG1");
  std::size_t class_index = arguments[0];
  std::size_t function = arguments[1];

  LOGGING_INFO(GetStringData(heap, function));

  LOGGING_INFO("DEBUG2");

  arguments.erase(arguments.begin(), arguments.begin() + 2);

  LOGGING_INFO("DEBUG3");

  auto invoke_function = builtin_functions.find(GetStringData(heap, function));

  LOGGING_INFO("DEBUG4");

  if (invoke_function != builtin_functions.end()) {
    LOGGING_INFO("DEBUG IN1");
    invoke_function->second(heap, arguments);
    LOGGING_INFO("DEBUG IN2");
    return 0;
  }

  LOGGING_INFO("DEBUG5");

  return Bytecode::InvokeClassFunction(
      heap, class_index, GetStringData(heap, function), arguments,
      current_bytecode_file, classes, memory, bytecode_files, builtin_functions,
      is_big_endian);
}

int LOAD_MEMBER(std::vector<Memory::Object>& heap,
                std::unordered_map<std::string, Bytecode::Class>& classes,
                std::size_t result, std::size_t class_index,
                std::size_t operand) {
  if (result >= heap.size()) INTERNAL_ERROR("Result out of object_table_size.");
  if (class_index >= heap.size())
    INTERNAL_ERROR("Class Out of object_table_size.");

  auto class_data = GetOriginData(heap, class_index);
  if (class_data.type[0] != 0x09) LOGGING_ERROR("Error class data.");

  if (std::get<Memory::ObjectReference>(class_data.data)
          .GetMemory()
          .get()
          .empty() ||
      std::get<Memory::ObjectReference>(class_data.data)
          .GetMemory()
          .get()[0]
          .type.empty() ||
      std::get<Memory::ObjectReference>(class_data.data)
              .GetMemory()
              .get()[0]
              .type[0] != 0x05)
    LOGGING_ERROR("Unsupported class name type.");
  std::string class_name =
      std::get<std::string>(std::get<Memory::ObjectReference>(class_data.data)
                                .GetMemory()
                                .get()[0]
                                .data);

  auto name_data = GetOriginData(heap, operand);
  if (name_data.type[0] != 0x05) LOGGING_ERROR("Error class name data.");

  std::string variable_name = std::get<std::string>(name_data.data);

  auto class_declaration = classes.find(class_name);

  if (class_declaration == classes.end())
    LOGGING_ERROR("Class not found: " + class_name);

  if (class_declaration->second.variables.find(variable_name) ==
      class_declaration->second.variables.end())
    LOGGING_ERROR("Variable not found: " + variable_name);

  std::size_t offset = class_declaration->second.variables[variable_name];

  SetReferenceData(
      heap, result,
      std::get<Memory::ObjectReference>(class_data.data).GetMemory(), offset);

  return 0;
}

int WIDE() { return 0; }
}  // namespace Operator
}  // namespace Vm
}  // namespace Aq