// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/builtin/builtin.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "vm/memory/memory.h"
#include "vm/logging/logging.h"

namespace Aq {
namespace Vm {
namespace Builtin {
void InitializeBuiltinFunction(
    std::unordered_map<std::string,
                       std::function<int(std::vector<Memory::Object>&,std::vector<std::size_t>)>>&
        builtin_functions) {
  // TODO: Implement the initialization of built-in functions.
  builtin_functions["__builtin_print"] = print;
}

int print(std::vector<Memory::Object>& heap,std::vector<std::size_t> arguments) {
  if (arguments.size() != 2)
    LOGGING_ERROR("Invalid arguments size.");
  Memory::Object object = GetOriginData(heap, arguments[1]);

  switch (object.type[0]) {
    case 0x01:
      SetLongData(heap,arguments[0], printf("%d", GetByteData(heap, arguments[1])));
      break;
    case 0x02:
      SetLongData(heap,arguments[0], printf("%lld", GetLongData(heap, arguments[1])));
      break;
    case 0x03:
      SetLongData(heap,arguments[0], printf("%.15f", GetDoubleData(heap, arguments[1])));
      break;
    case 0x04:
      SetLongData(heap,arguments[0], printf("%zu", GetUint64tData(heap, arguments[1])));
      break;
    case 0x05:
      SetLongData(heap,arguments[0], printf("%s", GetStringData(heap, arguments[1]).c_str()));
      break;
    case 0x06:
    LOGGING_WARNING("Not support print array yet.");
      // SetLongData(heap,arguments[0], printf("%p", GetArrayData(heap, arguments[1])));
      break;
    default:
      LOGGING_ERROR("Unsupported type.");
      break;
  }

  return 0;
}
/*
void vaprint(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (arguments.size() != 2)
    LOGGING_ERROR("Invalid arguments size.");
  Memory::Object object = GetOriginData(heap, arguments[1]);

  if ( object->type[0] != 0x06)
    LOGGING_ERROR( "Invalid object.");

  for (size_t i = 1; i < GetUint64tObjectData(object->data.ptr_data) + 1; i++) {
    switch (GetOriginData(object->data.ptr_data + i)->type[0]) {
      case 0x01:
        SetLongData(heap,arguments[0],
                    printf("%d", GetByteObjectData(object->data.ptr_data + i)));
        break;
      case 0x02:
        SetLongData(
            heap,arguments[0],
            printf("%lld", GetLongObjectData(object->data.ptr_data + i)));
        break;
      case 0x03:
        SetLongData(
            heap,arguments[0],
            printf("%.15f", GetDoubleObjectData(object->data.ptr_data + i)));
        break;
      case 0x04:
        SetLongData(
            heap,arguments[0],
            printf("%zu", GetUint64tObjectData(object->data.ptr_data + i)));
        break;
      case 0x05:
        SetLongData(
            heap,arguments[0],
            printf("%s", GetStringObjectData(object->data.ptr_data + i)));
        break;
      case 0x06:
        SetLongData(heap,arguments[0],
                    printf("%p", GetPtrObjectData(object->data.ptr_data + i)));
        break;
      default:
        printf("Type: %u\n", (object->data.ptr_data + i)->type[0]);
        EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Unsupported type.");
        break;
    }
  }
}

void aqstl_abs(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_abs(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_abs(InternalObject,size_t)", "Invalid return value.");

  struct Object* object = object_table + heap, arguments[1];
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_abs(InternalObject,size_t)", "Invalid object.");

  switch (object->type[0]) {
    case 0x01:
      SetByteData(heap,arguments[0], abs(GetByteData(heap, arguments[1])));
      break;
    case 0x02:
      SetLongData(heap,arguments[0], llabs(GetLongData(heap, arguments[1])));
      break;
    case 0x03:
      SetDoubleData(heap,arguments[0], fabs(GetDoubleData(heap, arguments[1])));
      break;
    case 0x04:
      SetUint64tData(heap,arguments[0], GetUint64tData(heap, arguments[1]));
      break;
    default:
      EXIT_VM("aqstl_abs(InternalObject,size_t)", "Unsupported type.");
  }
}

void aqstl_open(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 1 || args.size > 2)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid return value.");
  struct Object* object = object_table + heap, arguments[1];
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid object.");

  if (object->type[0] != 0x05)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid object.");
  const char* mode = "r";
  if (args.size == 2) {
    struct Object* mode_object = object_table + args.index[1];
    mode_object = GetOriginData(mode_object);
    if (mode_object == NULL)
      EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid mode object.");
    if (mode_object->type[0] != 0x05)
      EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid mode object.");
    mode = GetStringData(args.index[1]);
  }
  FILE* file = fopen(GetStringObjectData(object), mode);
  if (file == NULL) {
    // perror("[ERROR] aqstl_open(InternalObject,size_t)");
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Failed to open file.");
  }

  struct Object* return_object =
      (struct Object*)calloc(1, sizeof(struct Object));
  if (return_object == NULL) {
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Failed to allocate memory.");
  }
  return_object->const_type = false;
  return_object->type = (uint8_t*)calloc(1, sizeof(uint8_t));
  if (return_object->type == NULL) {
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Failed to allocate memory.");
  }
  *(return_object->type) = 0x0A;
  return_object->data.origin_data = file;

  SetOriginData(heap,arguments[0], return_object);
}

void aqstl_write(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2)
    EXIT_VM("aqstl_write(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_write(InternalObject,size_t)", "Invalid return value.");

  struct Object* object = object_table + heap, arguments[1];
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_write(InternalObject,size_t)", "Invalid object.");

  if (object->type[0] != 0x0A) {
    EXIT_VM("aqstl_write(InternalObject,size_t)", "Invalid object.");
  }

  struct Object* data_object = object_table + args.index[1];
  data_object = GetOriginData(data_object);

  if (data_object == NULL)
    EXIT_VM("aqstl_write(InternalObject,size_t)", "Invalid data object.");

  if (data_object->type[0] != 0x05) {
    EXIT_VM("aqstl_write(InternalObject,size_t)", "Invalid data object.");
  }

  size_t size = strlen(GetStringData(args.index[1]));
  size_t written = fwrite(GetStringData(args.index[1]), 1, size,
                          (FILE*)object->data.origin_data);
  if (written != size) {
    EXIT_VM("aqstl_write(InternalObject,size_t)", "Failed to write to file.");
  }

  SetLongData(heap,arguments[0], written);
}

void aqstl_read(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2)
    EXIT_VM("aqstl_read(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_read(InternalObject,size_t)", "Invalid return value.");

  struct Object* object = object_table + heap, arguments[1];
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_read(InternalObject,size_t)", "Invalid object.");

  if (object->type[0] != 0x0A) {
    EXIT_VM("aqstl_read(InternalObject,size_t)", "Invalid object.");
  }

  struct Object* data_object = object_table + args.index[1];
  data_object = GetOriginData(data_object);

  if (data_object == NULL)
    EXIT_VM("aqstl_read(InternalObject,size_t)", "Invalid data object.");

  size_t size = GetUint64tData(args.index[1]);
  void* data = calloc(size + 1, sizeof(char));
  size_t read = fread(data, 1, size, (FILE*)object->data.origin_data);
  if (read != size) {
    //EXIT_VM("aqstl_read(InternalObject,size_t)", "Failed to read from file.");
  }

  SetStringData(heap,arguments[0], (const char*)data);
}

void aqstl_close(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_close(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_close(InternalObject,size_t)", "Invalid return value.");

  struct Object* object = object_table + heap, arguments[1];
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_close(InternalObject,size_t)", "Invalid object.");

  if (object->type[0] != 0x0A) {
    // printf("Type: %u\n", object->type[0]);
    EXIT_VM("aqstl_close(InternalObject,size_t)", "Invalid object.");
  }

  if (fclose((FILE*)object->data.origin_data) != 0) {
    EXIT_VM("aqstl_close(InternalObject,size_t)", "Failed to close file.");
  }

  SetLongData(heap,arguments[0], 0);
}

void aqstl_input(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size!= 1)
    EXIT_VM("aqstl_input(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_input(InternalObject,size_t)", "Invalid return value.");
  
  struct Object* object = object_table + heap, arguments[1];
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_input(InternalObject,size_t)", "Invalid object.");
  if (object->type[0]!= 0x05) {
    EXIT_VM("aqstl_input(InternalObject,size_t)", "Invalid object.");    
  }

  printf("%s", GetStringObjectData(object));

  size_t size = 0;
  size_t buffer_size = 128;
  char* buffer = (char*)malloc(buffer_size * sizeof(char));
  if (!buffer) {
    EXIT_VM("aqstl_input(InternalObject,size_t)", "Memory allocation failed."); 
  }

  int c;
  size_t length = 0;
  while ((c = fgetc(stdin)) != EOF && c != '\n') {
      if (length >= buffer_size - 1) {
          buffer_size += 128;
          char* new_buffer = (char*)realloc(buffer, buffer_size * sizeof(char));
          if (!new_buffer) {
              free(buffer);
              EXIT_VM("aqstl_input(InternalObject,size_t)", "Memory reallocation failed.");
          }
          buffer = new_buffer;
      }
      buffer[length++] = (char)c;
  }
  buffer[length] = '\0';

  AddFreePtr(buffer);
  SetStringData(heap,arguments[0], buffer);
}*/
}  // namespace Builtin
}  // namespace Vm
}  // namespace Aq