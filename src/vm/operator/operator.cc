// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/operator/operator.h"

#include <string>

#include "vm/bytecode/bytecode.h"
#include "vm/logging/logging.h"
#include "vm/memory/memory.h"

namespace Aq {
namespace Vm {
namespace Operator {
int NOP() { return 0; }

int LOAD(std::vector<Memory::Object>& heap, size_t ptr, size_t operand) {
  LOGGING_WARNING("DEPRECATED OPERATOR.");
  return 0;
}

int STORE(std::vector<Memory::Object>& heap, size_t ptr, size_t operand) {
  LOGGING_WARNING("DEPRECATED OPERATOR.");
  return 0;
}

int InvokeClassFunction(size_t class_index, std::string name,
                        std::vector<std::size_t> arguments);

void AddBytecodeFile(std::string file);

int INVOKE_METHOD(std::vector<std::size_t> arguments);

int NEW(std::vector<Memory::Object>& heap,
        std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
        std::string& current_bytecode_file,
        std::unordered_map<std::string, Bytecode::Class> classes, size_t ptr,
        size_t size, size_t type) {
  if (ptr >= heap.size()) INTERNAL_ERROR("ptr is out of memory.");
  if (size >= heap.size()) INTERNAL_ERROR("size is out of memory.");

  Memory::Object type_data = GetOriginData(heap, type);

  bool is_bytecode_file_main_program = false;
  if (type_data.type[0] == 0x05 &&
      std::get<std::string>(type_data.data)[0] == '~') {
    std::string file_name = std::get<std::string>(type_data.data);
    std::string class_name;

    size_t start_pos = file_name.find('~');
    size_t end_pos = file_name.find('~', start_pos + 1);

    if (start_pos != std::string::npos && end_pos != std::string::npos &&
        start_pos < end_pos) {
      class_name = file_name.substr(end_pos + 1);
      file_name = file_name.substr(start_pos + 1, end_pos - start_pos - 1);
    } else {
      INTERNAL_ERROR("Bytecode file name is invalid.");
    }

    // TODO(bugs): May have bugs there.
    is_bytecode_file_main_program = true;

    if (class_name == ".!__start") {
      if (bytecode_files.find(class_name) != bytecode_files.end()) {
        heap[ptr].type.push_back(0x07);
        heap[ptr].const_type = false;
        heap[ptr].data = bytecode_files[class_name].object;
        return 0;
      }

      AddBytecodeFile(std::get<std::string>(type_data.data));
    }
  }
  std::size_t size_value = GetUint64tData(heap, size);

  if ((type == 0 || (type_data.type[0] != 0x05 ||
                     std::get<std::string>(type_data.data).empty())) &&
      size_value == 0)
    size_value = 1;

  std::vector<Memory::Object> data(size_value + 1);

  data[0].type.push_back(0x04);
  data[0].const_type = true;
  data[0].data = size_value;

  if (type == 0) {
    for (size_t i = 1; i < 2; i++) {
      data[i].type.push_back(0x00);
      data[i].const_type = false;
    }
  } else {
    if (type_data.type[0] == 0x05 &&
        std::get<std::string>(type_data.data).empty()) {
      if (size_value == 0) {
        size_t i = 0;
        data[i].type.push_back(0x09);
        data[i].const_type = true;

        std::string class_name = GetStringData(heap, type);

        if (current_bytecode_file == "" && class_name[0] != '~') {
          class_name = "~" + current_bytecode_file + "~" + class_name;
        }

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

          std::string class_name = GetStringData(heap, type);

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
    SetObjectData(heap, ptr,
                  std::get<std::vector<Memory::Object>>(type_data.data));

    if (is_bytecode_file_main_program) {
      std::string class_name = GetStringData(heap, type);

      if (classes.find(class_name) == classes.end())
        LOGGING_ERROR("class not found.");

      Bytecode::Class& class_data = classes[class_name];
      class_data.memory->heap[2].data = data;

      heap.push_back({{0x05}, true, std::string("@constructor")});
      INVOKE_METHOD({2, heap.size() - 1, 1, 0});
    }
  } else {
    SetArrayData(heap, ptr, data);
  }
  return 0;
}

int CrossMemoryNew(struct Memory* memory, size_t ptr, size_t size,
                   size_t type) {
  if (ptr >= memory->object_table_size)
    EXIT_VM("NEW(size_t, size_t)", "ptr is out of memory.");
  if (size >= object_table_size)
    EXIT_VM("NEW(size_t, size_t)", "size is out of memory.");

  struct Object* type_data = object_table + type;
  type_data = GetOriginData(type_data);

  size_t size_value = GetUint64tData(size);

  if ((type == 0 ||
       (type_data->type[0] != 0x05 || type_data->data.string_data == NULL)) &&
      size_value == 0)
    size_value = 1;

  struct Object* data = calloc(size_value + 1, sizeof(struct Object));
  AddFreePtr(data);

  uint8_t* type_ptr = calloc(1, sizeof(uint8_t));
  data[0].type = type_ptr;
  type_ptr[0] = 0x04;
  data[0].const_type = true;
  data[0].data.uint64t_data = size_value;

  AddFreePtr(type_ptr);

  if (type == 0) {
    for (size_t i = 1; i < 2; i++) {
      uint8_t* type_ptr = calloc(1, sizeof(uint8_t));
      data[i].type = type_ptr;
      data[i].const_type = false;
      AddFreePtr(type_ptr);
    }
  } else {
    if (type_data->type[0] == 0x05 && type_data->data.string_data != NULL) {
      if (size_value == 0) {
        size_t i = 0;
        uint8_t* type_ptr = calloc(1, sizeof(uint8_t));
        data[i].type = type_ptr;
        data[i].type[0] = 0x09;
        data[i].const_type = true;
        AddFreePtr(type_ptr);

        struct Class* class_data = NULL;
        const char* class = GetStringData(type);
        const unsigned int class_hash = hash(class);
        struct ClassList* current_class_table = &class_table[class_hash];
        while (current_class_table != NULL &&
               current_class_table->class.name != NULL) {
          if (strcmp(current_class_table->class.name, class) == 0) {
            class_data = &current_class_table->class;
            break;
          }
          current_class_table = current_class_table->next;
        }

        if (class_data == NULL) {
          EXIT_VM("NEW(size_t, size_t)", "Class not found.");
        }

        struct Object* class_object =
            calloc(class_data->members_size, sizeof(struct Object));
        AddFreePtr(class_object);
        for (size_t j = 0; j < class_data->members_size; j++) {
          uint8_t* location = class_data->members[j].type;
          size_t length = 1;
          bool is_type_end = false;
          while (!is_type_end) {
            switch (*location) {
              case 0x00:
              case 0x01:
              case 0x02:
              case 0x03:
              case 0x04:
              case 0x05:
              case 0x09:
                is_type_end = true;
                break;

              case 0x06:
              case 0x07:
              case 0x08:
                length++;
                location++;
                break;

              default:
                EXIT_VM("AddClass(void*)", "Unsupported type.");
                break;
            }
          }

          class_object[j].type = calloc(length, sizeof(uint8_t));
          AddFreePtr(class_object[j].type);
          memcpy(class_object[j].type, class_data->members[j].type, length);
          class_object[j].const_type = class_data->members[j].const_type;
        }
        class_object[0].const_type = true;
        class_object[0].type[0] = 0x05;
        class_object[0].data.string_data = class;
        class_object[1].const_type = true;
        class_object[1].type[0] = 0x04;
        class_object[1].data.uint64t_data = class_data->members_size;
        data[i].data.object_data = class_object;

      } else {
        for (size_t i = 1; i < size_value + 1; i++) {
          uint8_t* type_ptr = calloc(1, sizeof(uint8_t));
          data[i].type = type_ptr;
          data[i].type[0] = 0x09;
          data[i].const_type = true;
          AddFreePtr(type_ptr);

          struct Class* class_data = NULL;
          const char* class = GetStringData(type);
          const unsigned int class_hash = hash(class);
          struct ClassList* current_class_table = &class_table[class_hash];
          while (current_class_table != NULL &&
                 current_class_table->class.name != NULL) {
            if (strcmp(current_class_table->class.name, class) == 0) {
              class_data = &current_class_table->class;
              break;
            }
            current_class_table = current_class_table->next;
          }

          if (class_data == NULL) {
            EXIT_VM("NEW(size_t, size_t)", "Class not found.");
          }

          struct Object* class_object =
              calloc(class_data->members_size, sizeof(struct Object));
          AddFreePtr(class_object);
          for (size_t j = 0; j < class_data->members_size; j++) {
            uint8_t* location = class_data->members[j].type;
            size_t length = 1;
            bool is_type_end = false;
            while (!is_type_end) {
              switch (*location) {
                case 0x00:
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                case 0x09:
                  is_type_end = true;
                  break;

                case 0x06:
                case 0x07:
                case 0x08:
                  length++;
                  location++;
                  break;

                default:
                  EXIT_VM("AddClass(void*)", "Unsupported type.");
                  break;
              }
            }

            class_object[j].type = calloc(length, sizeof(uint8_t));
            AddFreePtr(class_object[j].type);
            memcpy(class_object[j].type, class_data->members[j].type, length);
            class_object[j].const_type = class_data->members[j].const_type;
          }
          class_object[0].const_type = true;
          class_object[0].type[0] = 0x05;
          class_object[0].data.string_data = class;
          class_object[1].const_type = true;
          class_object[1].type[0] = 0x04;
          class_object[1].data.uint64t_data = class_data->members_size;
          data[i].data.object_data = class_object;
        }
      }
    } else {
      for (size_t i = 1; i < 2; i++) {
        data[i].type = type_data->type;
        data[i].const_type = true;
      }
    }
  }

  if (size_value == 0 && type_data->type[0] == 0x05 &&
      type_data->data.string_data != NULL) {
    SetObjectObjectData(memory->object_table + ptr, data->data.object_data);
  } else {
    SetPtrObjectData(memory->object_table + ptr, data);
  }
  return 0;
}

int InvokeCustomFunction(const char* name, size_t args_size,
                         size_t return_value, size_t* args);

int ARRAY(size_t result, size_t ptr, size_t index) {
  struct Object* array_object = GetPtrData(ptr);

  index = GetUint64tData(index);

  size_t original_size = GetUint64tObjectData(array_object);
  if (index >= GetUint64tObjectData(array_object)) {
    size_t newsize = index + 1;
    size_t new_allocated =
        (size_t)newsize + (newsize >> 3) + (newsize < 9 ? 3 : 6) + 1;
    struct Object* new_array = calloc(new_allocated, sizeof(struct Object));
    AddFreePtr(new_array);
    memcpy(new_array, array_object,
           sizeof(struct Object) * (GetUint64tObjectData(array_object) + 1));
    SetPtrData(ptr, new_array);
    new_array[0].const_type = true;
    new_array[0].type[0] = 0x04;
    new_array[0].data.uint64t_data = newsize;
    array_object = new_array;
  }

  bool is_type_null = (array_object + 1 + index)->type == NULL;

  if ((array_object + 1 + index)->type == NULL) {
    if ((array_object + 1)->const_type) {
      (array_object + 1 + index)->const_type = true;
      (array_object + 1 + index)->type = (array_object + 1)->type;
    } else {
      (array_object + 1 + index)->type = calloc(1, sizeof(uint8_t));
    }
  }

  if ((array_object + 1)->const_type && (array_object + 1)->type[0] == 0x09 &&
      is_type_null) {
    SetReferenceData(result, array_object + 1 + index);
    InvokeCustomFunction((array_object + 1)->data.object_data->data.string_data,
                         1, result, NULL);
  }

  SetReferenceData(result, array_object + 1 + index);

  return 0;
}
int PTR(size_t index, size_t ptr) {
  SetPtrData(ptr, object_table + index);
  return 0;
}
int ADD(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("ADD(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("ADD(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("ADD(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if (GetByteData(operand1) + GetByteData(operand2) > INT8_MAX ||
            GetByteData(operand1) + GetByteData(operand2) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) + GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) + GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) + GetUint64tData(operand2));
        break;
      case 0x05: {
        char* new_str = (char*)calloc(strlen(GetStringData(operand1)) +
                                          strlen(GetStringData(operand2)) + 1,
                                      sizeof(char));
        AddFreePtr(new_str);
        strncpy(new_str, GetStringData(operand1),
                strlen(GetStringData(operand1)));
        strncat(new_str, GetStringData(operand2),
                strlen(GetStringData(operand2)));
        SetStringData(result, new_str);
        break;
      }
      default:
        EXIT_VM("ADD(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("ADD(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) + GetUint64tData(operand2));
        break;
      case 0x06:
        if (operand1_data->type[0] == 0x06) {
          SetPtrData(result, GetPtrData(operand1) + GetLongData(operand2));
        } else {
          SetPtrData(result, GetLongData(operand1) + GetPtrData(operand2));
        }
        break;
      default:
        EXIT_VM("ADD(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int SUB(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("SUB(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("SUB(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("SUB(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if (GetByteData(operand1) - GetByteData(operand2) > INT8_MAX ||
            GetByteData(operand1) - GetByteData(operand2) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) - GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) - GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) - GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) - GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("SUB(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("SUB(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) - GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) - GetUint64tData(operand2));
        break;
      case 0x06:
        if (operand1_data->type[0] == 0x06) {
          SetPtrData(result, GetPtrData(operand1) - GetLongData(operand2));
        } else {
          EXIT_VM("SUB(size_t,size_t,size_t)", "Unsupported operator.");
        }
        break;
      default:
        EXIT_VM("SUB(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int MUL(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("MUL(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("MUL(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("MUL(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if (GetByteData(operand1) * GetByteData(operand2) > INT8_MAX ||
            GetByteData(operand1) * GetByteData(operand2) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) * GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) * GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) * GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) * GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("MUL(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) * GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) * GetUint64tData(operand2));
        break;
      case 0x05:
        if (operand1_data->type[0] == 0x05) {
          char* new_str = (char*)calloc(
              GetUint64tData(operand2) * strlen(GetStringData(operand1)) + 1,
              sizeof(char));
          AddFreePtr(new_str);
          for (size_t i = 0; i < GetUint64tData(operand2); i++) {
            strncat(new_str, GetStringData(operand1),
                    strlen(GetStringData(operand1)));
          }
          SetStringData(result, new_str);
        } else {
          char* new_str = (char*)calloc(
              GetUint64tData(operand1) * strlen(GetStringData(operand2)) + 1,
              sizeof(char));
          AddFreePtr(new_str);
          for (size_t i = 0; i < GetUint64tData(operand1); i++) {
            strncat(new_str, GetStringData(operand2),
                    strlen(GetStringData(operand2)));
          }
          SetStringData(result, new_str);
        }
        break;
      default:
        EXIT_VM("MUL(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int DIV(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("DIV(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("DIV(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("DIV(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
    EXIT_VM("DIV(size_t,size_t,size_t)", "Unsupported type.");
  uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                            ? operand1_data->type[0]
                            : operand2_data->type[0];

  switch (result_type) {
    case 0x01:
      if (GetByteData(operand1) / GetByteData(operand2) > INT8_MAX ||
          GetByteData(operand1) / GetByteData(operand2) < INT8_MIN) {
        if (GetByteData(operand1) % GetByteData(operand2) == 0) {
          SetLongData(result, GetByteData(operand1) / GetByteData(operand2));
        } else {
          SetDoubleData(result, (double)GetByteData(operand1) /
                                    (double)GetByteData(operand2));
        }
      } else {
        if (GetByteData(operand1) % GetByteData(operand2) == 0) {
          SetByteData(result, GetByteData(operand1) / GetByteData(operand2));
        } else {
          SetDoubleData(result, (double)GetByteData(operand1) /
                                    (double)GetByteData(operand2));
        }
      }
      break;
    case 0x02:
      if (GetLongData(operand1) % GetLongData(operand2) == 0) {
        SetLongData(result, GetLongData(operand1) / GetLongData(operand2));
      } else {
        SetDoubleData(result, (double)GetLongData(operand1) /
                                  (double)GetLongData(operand2));
      }
      break;
    case 0x03:
      SetDoubleData(result, (double)GetDoubleData(operand1) /
                                (double)GetDoubleData(operand2));
      break;
    case 0x04:
      if (GetUint64tData(operand1) / GetUint64tData(operand2) > INT64_MAX) {
        SetUint64tData(result,
                       GetUint64tData(operand1) / GetUint64tData(operand2));
      } else {
        if (GetUint64tData(operand1) % GetUint64tData(operand2) == 0) {
          SetLongData(result,
                      GetUint64tData(operand1) / GetUint64tData(operand2));
        } else {
          SetDoubleData(result, (double)GetUint64tData(operand1) /
                                    (double)GetUint64tData(operand2));
        }
      }
      break;
    default:
      EXIT_VM("DIV(size_t,size_t,size_t)", "Unsupported type.");
      break;
  }

  return 0;
}
int REM(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("REM(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("REM(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("REM(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if (GetByteData(operand1) % GetByteData(operand2) > INT8_MAX ||
            GetByteData(operand1) % GetByteData(operand2) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) % GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) % GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) % GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) % GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("REM(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("REM(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) % GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) % GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("REM(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int NEG(size_t result, size_t operand1) {
  if (result >= object_table_size)
    EXIT_VM("SUB(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("SUB(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  operand1_data = GetOriginData(operand1_data);

  switch (operand1_data->type[0]) {
    case 0x01:
      SetByteData(result, -GetByteData(operand1));
      break;
    case 0x02:
      SetLongData(result, -GetLongData(operand1));
      break;
    case 0x03:
      SetDoubleData(result, -GetDoubleData(operand1));
      break;
    case 0x04:
      SetUint64tData(result, -GetUint64tData(operand1));
      break;
    default:
      EXIT_VM("NEG(size_t,size_t)", "Unsupported type.");
      break;
  }

  return 0;
}
int SHL(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("SHL(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("SHL(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("SHL(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if (GetByteData(operand1) << GetByteData(operand2) > INT8_MAX ||
            GetByteData(operand1) << GetByteData(operand2) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) << GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) << GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) << GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result, GetUint64tData(operand1)
                                   << GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("SHL(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("SHL(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) << GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result, GetUint64tData(operand1)
                                   << GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("SHL(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int SHR(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("SHR(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("SHR(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("SHR(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if (GetByteData(operand1) >> GetByteData(operand2) > INT8_MAX ||
            GetByteData(operand1) >> GetByteData(operand2) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) >> GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) >> GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("SHR(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("SHR(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("SHR(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int REFER(size_t result, size_t operand1) {
  if (result >= object_table_size)
    EXIT_VM("REFER(size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("REFER(size_t,size_t)", "Out of object_table_size.");

  struct Object* data = GetOriginData(object_table + operand1);

  SetReferenceData(result, data);

  return 0;
}
size_t IF(size_t condition, size_t true_branche, size_t false_branche) {
  if (GetByteData(condition) != 0) {
    return true_branche;
  } else {
    return false_branche;
  }
}
int AND(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("AND(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("AND(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("AND(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if ((GetByteData(operand1) & GetByteData(operand2)) > INT8_MAX ||
            (GetByteData(operand1) & GetByteData(operand2)) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) & GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) & GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) & GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) & GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("AND(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("AND(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) & GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) & GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("AND(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int OR(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("OR(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("OR(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("OR(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if ((GetByteData(operand1) | GetByteData(operand2)) > INT8_MAX ||
            (GetByteData(operand1) | GetByteData(operand2)) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) | GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) | GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) | GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) | GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("OR(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("OR(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) | GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) | GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("OR(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int XOR(size_t result, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("XOR(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("XOR(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("XOR(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if ((GetByteData(operand1) ^ GetByteData(operand2)) > INT8_MAX ||
            (GetByteData(operand1) ^ GetByteData(operand2)) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) ^ GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) ^ GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) ^ GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) ^ GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("XOR(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("XOR(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) ^ GetLongData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) ^ GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("XOR(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }
  return 0;
}
int CMP(size_t result, size_t opcode, size_t operand1, size_t operand2) {
  if (result >= object_table_size)
    EXIT_VM("CMP(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("CMP(size_t,size_t,size_t)", "Out of object_table_size.");
  if (operand2 >= object_table_size)
    EXIT_VM("CMP(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* operand1_data = object_table + operand1;
  struct Object* operand2_data = object_table + operand2;
  operand1_data = GetOriginData(operand1_data);
  operand2_data = GetOriginData(operand2_data);

  switch (opcode) {
    case 0x00:
      if (operand1_data->type[0] == operand2_data->type[0]) {
        switch (operand1_data->type[0]) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) == GetByteData(operand2));
            break;
          case 0x02:
            SetByteData(result, GetLongData(operand1) == GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) == GetUint64tData(operand2));
            break;
          case 0x05:
            SetByteData(result, strcmp(GetStringData(operand1),
                                       GetStringData(operand2)) == 0);
            break;
          case 0x06:
            SetByteData(result, GetPtrData(operand1) == GetPtrData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      } else {
        if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
          EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
        uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                                  ? operand1_data->type[0]
                                  : operand2_data->type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(result, GetLongData(operand1) == GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) == GetUint64tData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      }
      break;
    case 0x01:
      if (operand1_data->type[0] == operand2_data->type[0]) {
        switch (operand1_data->type[0]) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) != GetByteData(operand2));
            break;
          case 0x02:
            SetByteData(result, GetLongData(operand1) != GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) != GetUint64tData(operand2));
            break;
          case 0x05:
            SetByteData(result, strcmp(GetStringData(operand1),
                                       GetStringData(operand2)) != 0);
            break;
          case 0x06:
            SetByteData(result, GetPtrData(operand1) != GetPtrData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      } else {
        if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
          EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
        uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                                  ? operand1_data->type[0]
                                  : operand2_data->type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(result, GetLongData(operand1) != GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) != GetUint64tData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      }
      break;
    case 0x02:
      if (operand1_data->type[0] == operand2_data->type[0]) {
        switch (operand1_data->type[0]) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) > GetByteData(operand2));
            break;
          case 0x02:
            SetByteData(result, GetLongData(operand1) > GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) > GetUint64tData(operand2));
            break;
          case 0x05:
            SetByteData(result, strcmp(GetStringData(operand1),
                                       GetStringData(operand2)) > 0);
            break;
          case 0x06:
            SetByteData(result, GetPtrData(operand1) > GetPtrData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      } else {
        if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
          EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
        uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                                  ? operand1_data->type[0]
                                  : operand2_data->type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(result, GetLongData(operand1) > GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) > GetUint64tData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      }
      break;
    case 0x03:
      if (operand1_data->type[0] == operand2_data->type[0]) {
        switch (operand1_data->type[0]) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) >= GetByteData(operand2));
            break;
          case 0x02:
            SetByteData(result, GetLongData(operand1) >= GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) >= GetUint64tData(operand2));
            break;
          case 0x05:
            SetByteData(result, strcmp(GetStringData(operand1),
                                       GetStringData(operand2)) >= 0);
            break;
          case 0x06:
            SetByteData(result, GetPtrData(operand1) >= GetPtrData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      } else {
        if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
          EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
        uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                                  ? operand1_data->type[0]
                                  : operand2_data->type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(result, GetLongData(operand1) >= GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) >= GetUint64tData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      }
      break;
    case 0x04:
      if (operand1_data->type[0] == operand2_data->type[0]) {
        switch (operand1_data->type[0]) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) < GetByteData(operand2));
            break;
          case 0x02:
            SetByteData(result, GetLongData(operand1) < GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) < GetUint64tData(operand2));
            break;
          case 0x05:
            SetByteData(result, strcmp(GetStringData(operand1),
                                       GetStringData(operand2)) < 0);
            break;
          case 0x06:
            SetByteData(result, GetPtrData(operand1) < GetPtrData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      } else {
        if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
          EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
        uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                                  ? operand1_data->type[0]
                                  : operand2_data->type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(result, GetLongData(operand1) < GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) < GetUint64tData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      }
      break;
    case 0x05:
      if (operand1_data->type[0] == operand2_data->type[0]) {
        switch (operand1_data->type[0]) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) <= GetByteData(operand2));
            break;
          case 0x02:
            SetByteData(result, GetLongData(operand1) <= GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) <= GetUint64tData(operand2));
            break;
          case 0x05:
            SetByteData(result, strcmp(GetStringData(operand1),
                                       GetStringData(operand2)) <= 0);
            break;
          case 0x06:
            SetByteData(result, GetPtrData(operand1) <= GetPtrData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      } else {
        if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
          EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
        uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                                  ? operand1_data->type[0]
                                  : operand2_data->type[0];
        switch (result_type) {
          case 0x02:
            SetByteData(result, GetLongData(operand1) <= GetLongData(operand2));
            break;
          case 0x03:
            SetByteData(result,
                        GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x04:
            SetByteData(result,
                        GetUint64tData(operand1) <= GetUint64tData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t,size_t,size_t)", "Unsupported type.");
            break;
        }
      }
      break;
    default:
      EXIT_VM("CMP(size_t,size_t,size_t,size_t)", "Invalid opcode.");
  }
  return 0;
}
int INVOKE(size_t* args) {
  if (args == NULL) EXIT_VM("INVOKE(size_t*)", "Invalid args.");
  size_t func = args[0];
  size_t arg_count = args[1];
  size_t return_value = args[2];
  size_t* invoke_args = NULL;
  if (arg_count > 0) {
    invoke_args = args + 3;
  }
  InternalObject args_obj = {arg_count - 1, invoke_args};
  func_ptr invoke_func = GetFunction(GetStringData(func));
  if (invoke_func != NULL) {
    invoke_func(args_obj, return_value);
    return 0;
  }

  return InvokeCustomFunction(GetStringData(func), arg_count, return_value,
                              invoke_args);
}
int EQUAL(size_t result, size_t value) {
  if (result >= object_table_size)
    EXIT_VM("EQUAL(size_t,size_t)", "Out of object_table_size.");
  if (value >= object_table_size)
    EXIT_VM("EQUAL(size_t,size_t)", "Out of object_table_size.");

  struct Object* value_data = object_table + value;
  value_data = GetOriginData(value_data);

  switch (value_data->type[0]) {
    case 0x00:
      break;
    case 0x01:
      SetByteData(result, GetByteData(value));
      break;
    case 0x02:
      SetLongData(result, GetLongData(value));
      break;
    case 0x03:
      SetDoubleData(result, GetDoubleData(value));
      break;
    case 0x04:
      SetUint64tData(result, GetUint64tData(value));
      break;
    case 0x05:
      SetStringData(result, GetStringData(value));
      break;
    case 0x06:
      SetPtrData(result, GetPtrData(value));
      break;
    case 0x09:
      SetObjectData(result, GetObjectData(value));
      break;
    case 0x0A:
      SetOriginData(result, GetOriginData(object_table + value));
      break;
    default:
      EXIT_VM("EQUAL(size_t,size_t)", "Unsupported type.");
  }
  return 0;
}

int CrossMemoryEqual(struct Memory* result_memory, size_t result,
                     struct Memory* value_memory, size_t value) {
  if (result >= result_memory->object_table_size)
    EXIT_VM("CrossMemoryEqual(struct Memory*,size_t,struct Memory*,size_t)",
            "Out of object_table_size.");
  if (value >= value_memory->object_table_size)
    EXIT_VM("CrossMemoryEqual(struct Memory*,size_t,struct Memory*,size_t)",
            "Out of object_table_size.");

  struct Object* value_data = value_memory->object_table + value;
  value_data = GetOriginData(value_data);

  switch (value_data->type[0]) {
    case 0x00:
      break;
    case 0x01:
      SetByteObjectData(result_memory->object_table + result,
                        GetByteObjectData(value_memory->object_table + value));
      break;
    case 0x02:
      SetLongObjectData(result_memory->object_table + result,
                        GetLongObjectData(value_memory->object_table + value));
      break;
    case 0x03:
      SetDoubleObjectData(
          result_memory->object_table + result,
          GetDoubleObjectData(value_memory->object_table + value));
      break;
    case 0x04:
      SetUint64tObjectData(
          result_memory->object_table + result,
          GetUint64tObjectData(value_memory->object_table + value));
      break;
    case 0x05:
      SetStringObjectData(
          result_memory->object_table + result,
          GetStringObjectData(value_memory->object_table + value));
      break;
    case 0x06:
      SetPtrObjectData(result_memory->object_table + result,
                       GetPtrObjectData(value_memory->object_table + value));
      break;
    case 0x09:
      SetObjectObjectData(
          result_memory->object_table + result,
          GetObjectObjectData(value_memory->object_table + value));
      break;
    case 0x0A:
      SetOriginObjectData(result_memory->object_table + result,
                          GetOriginData(value_memory->object_table + value));
      break;
    default:
      EXIT_VM("CrossMemoryEqual(struct Memory*,size_t,struct Memory*,size_t)",
              "Unsupported type.");
  }
  return 0;
}

size_t GOTO(size_t location) { return GetUint64tData(location); }
int LOAD_CONST(size_t object, size_t const_object) {
  if (object >= object_table_size)
    EXIT_VM("LOAD_CONST(size_t,size_t)", "Out of object_table_size.");
  if (const_object >= const_object_table_size)
    EXIT_VM("LOAD_CONST(size_t,size_t)", "Out of const_object_table_size.");

  switch (const_object_table[const_object].type[0]) {
    case 0x01:
      SetByteData(object, const_object_table[const_object].data.byte_data);
      break;
    case 0x02:
      SetLongData(object, const_object_table[const_object].data.long_data);
      break;
    case 0x03:
      SetDoubleData(object, const_object_table[const_object].data.double_data);
      break;
    case 0x04:
      SetUint64tData(object,
                     const_object_table[const_object].data.uint64t_data);
      break;
    case 0x05:
      SetStringData(object, const_object_table[const_object].data.string_data);
      break;
    case 0x06:
      SetPtrData(object, const_object_table[const_object].data.ptr_data);
      break;
    default:
      EXIT_VM("LOAD_CONST(size_t,size_t)", "Unsupported type.");
  }

  return 0;
}
int CONVERT(size_t result, size_t operand1) {
  if (result >= object_table_size)
    EXIT_VM("CONVERT(size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("CONVERT(size_t,size_t)", "Out of object_table_size.");

  struct Object* result_data = object_table + result;
  result_data = GetOriginData(result_data);

  switch (result_data->type[0]) {
    case 0x01:
      SetByteData(result, GetByteData(operand1));
      break;

    case 0x02:
      SetLongData(result, GetLongData(operand1));
      break;

    case 0x03:
      SetDoubleData(result, GetDoubleData(operand1));
      break;

    case 0x04:
      SetUint64tData(result, GetUint64tData(operand1));
      break;

    case 0x05:
      SetStringData(result, GetStringData(operand1));
      break;

    case 0x06:
      SetPtrData(result, GetPtrData(operand1));
      break;

    default:
      EXIT_VM("CONVERT(size_t,size_t)", "Unsupported type.");
      break;
  }
  return 0;
}
int _CONST(size_t result, size_t operand1) {
  if (result >= object_table_size)
    EXIT_VM("CONST(size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("CONST(size_t,size_t)", "Out of object_table_size.");

  SetConstData(result, object_table + operand1);
  return 0;
}

int INVOKE_METHOD(size_t* args) {
  if (args == NULL) EXIT_VM("INVOKE_METHOD(size_t*)", "Invalid args.");
  size_t func = args[1];
  size_t arg_count = args[2];
  size_t return_value = args[3];
  size_t* invoke_args = NULL;
  if (arg_count > 0) {
    invoke_args = args + 4;
  }
  InternalObject args_obj = {arg_count - 1, invoke_args};

  func_ptr invoke_func = GetFunction(GetStringData(func));
  if (invoke_func != NULL) {
    invoke_func(args_obj, return_value);
    return 0;
  }

  return InvokeClassFunction(args[0], GetStringData(func), arg_count,
                             return_value, invoke_args);
}

int LOAD_MEMBER(size_t result, size_t class, size_t operand) {
  if (result >= object_table_size)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)",
            "Result out of object_table_size.");
  if (class >= object_table_size)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)",
            "Class Out of object_table_size.");

  struct Object* class_data = object_table + class;
  class_data = GetOriginData(class_data);
  // printf("\nType: %i\n", class_data->type[0]);
  if (class_data == NULL || class_data->type[0] != 0x09)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Error class data.");

  if (class_data->data.object_data == NULL ||
      class_data->data.object_data->type == NULL ||
      class_data->data.object_data->type[0] != 0x05)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)",
            "Unsupported class name type.");
  const char* class_name = class_data->data.object_data->data.string_data;

  struct Object* name_data = object_table + operand;
  name_data = GetOriginData(name_data);
  if (name_data == NULL || name_data->type[0] != 0x05)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Error class name data.");

  const char* var_name = name_data->data.string_data;

  size_t offset = 0;
  bool is_find = false;
  const unsigned int class_hash = hash(class_name);
  struct ClassList* current_class_table = &class_table[class_hash];
  while (current_class_table != NULL &&
         current_class_table->class.name != NULL) {
    if (strcmp(current_class_table->class.name, class_name) == 0) {
      bool is_var_find = false;
      const unsigned int member_hash = hash(var_name);
      struct ClassVarInfoList* current_var_table =
          &(current_class_table->class.var_info_table[member_hash]);
      while (current_var_table != NULL && current_var_table->name != NULL) {
        if (strcmp(current_var_table->name, var_name) == 0) {
          offset = current_var_table->index;
          is_var_find = true;
          break;
        }
        current_var_table = current_var_table->next;
      }
      if (!is_var_find)
        EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Class Var not found.");
      is_find = true;
      break;
    }
    current_class_table = current_class_table->next;
  }

  if (!is_find)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Class not found.");

  struct Object* object_data = class_data->data.object_data + offset;

  SetReferenceData(result, object_data);

  return 0;
}

int WIDE() { return 0; }
}  // namespace Operator
}  // namespace Vm
}  // namespace Aq