// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "aqstl.h"
#include "aqvm_init.h"

// #define TRACE_FUNCTION Trace trace(__FUNCTION__)

/*typedef struct StackNode {
  char* function_name;
  struct StackNode* next;
} StackNode;

StackNode* call_stack = NULL;

void PushStack(const char* function_name) {
  StackNode* new_node = (StackNode*)malloc(sizeof(StackNode));
  new_node->function_name = strdup(function_name);
  new_node->next = call_stack;
  call_stack = new_node;
}

void PopStack() {
  if (call_stack != NULL) {
    StackNode* temp = call_stack;
    call_stack = call_stack->next;
    free(temp->function_name);
    free(temp);
  }
}

void PrintStackRecursive(StackNode* node) {
  if (node == NULL) {
    printf("[INFO] Run: ");
    return;
  }
  PrintStackRecursive(node->next);
  printf("%s -> ", node->function_name);
}

void PrintStack() {
  PrintStackRecursive(call_stack);
  printf("Success\n");
}

typedef struct Trace {
  const char* function_name;
} Trace;

Trace TraceCreate(const char* function_name) {
  Trace trace;
  trace.function_name = function_name;
  PushStack(function_name);
  PrintStack();
  return trace;
}

void TraceDestroy(Trace* trace) {
  if (trace) {
    PopStack();
    PrintStack();
  }
}

#define Trace(trace)                       \
  Trace trace = TraceCreate(__FUNCTION__); \
  __attribute__((cleanup(TraceDestroy))) Trace* trace_ptr = &trace;

#define TRACE_FUNCTION                                  \
  Trace _trace __attribute__((cleanup(TraceDestroy))) = \
      TraceCreate(__FUNCTION__)*/

#define TRACE_FUNCTION

/*union Data {
  int8_t byte_data;
  int64_t long_data;
  double double_data;
  uint64_t uint64t_data;
  const char* string_data;
  struct Object* ptr_data;
  struct Object* reference_data;
  struct Object* const_data;
  struct Object* object_data;
};

struct Object {
  uint8_t* type;
  bool const_type;
  union Data data;
};*/

struct Object* const_object_table;

size_t const_object_table_size;

// struct Object* object_table;

// size_t object_table_size;

/*typedef struct {
  size_t size;
  size_t* index;
} InternalObject;

typedef void (*func_ptr)(InternalObject, size_t);*/

enum Operator {
  OPERATOR_NOP = 0x00,
  OPERATOR_LOAD,
  OPERATOR_STORE,
  OPERATOR_NEW,
  OPERATOR_ARRAY,
  OPERATOR_PTR,
  OPERATOR_ADD,
  OPERATOR_SUB,
  OPERATOR_MUL,
  OPERATOR_DIV,
  OPERATOR_REM,
  OPERATOR_NEG,
  OPERATOR_SHL,
  OPERATOR_SHR,
  OPERATOR_REFER,
  OPERATOR_IF,
  OPERATOR_AND,
  OPERATOR_OR,
  OPERATOR_XOR,
  OPERATOR_CMP,
  OPERATOR_INVOKE,
  OPERATOR_EQUAL,
  OPERATOR_GOTO,
  OPERATOR_LOAD_CONST,
  OPERATOR_CONVERT,
  OPERATOR_CONST,
  OPERATOR_INVOKE_METHOD,
  OPERATOR_LOAD_MEMBER,
  OPERATOR_WIDE = 0xFF
};

struct Bytecode {
  enum Operator operator;
  size_t* args;
};

typedef struct {
  const char* name;
  void* location;
  size_t commands_size;
  bool va_flag;
  struct Bytecode* commands;
  size_t args_size;
  size_t* args;
} FuncInfo;

struct FuncPair {
  const char* first;
  FuncInfo second;
};

struct FreeList {
  struct FreeList* next;
  void* ptr;
};

struct FuncList {
  struct FuncPair pair;
  struct FuncList* next;
};

struct Memory {
  struct Object* object_table;
  size_t object_table_size;
  struct Object* const_object_table;
  size_t const_object_table_size;
};

struct ClassVarInfoList {
  const char* name;
  size_t index;
  struct ClassVarInfoList* next;
};

struct BytecodeFileList {
  const char* name;
  struct Object* object;
  int index;
  struct BytecodeFileList* next;
};

struct BytecodeFileList bytecode_file_table[256];

const char* current_bytecode_file;

struct Class {
  const char* name;
  struct Object* members;
  struct ClassVarInfoList var_info_table[256];
  size_t members_size;
  struct FuncList methods[256];
  struct BytecodeFileList bytecode_file[256];
  struct Memory* memory;
};

struct ClassList {
  struct Class class;
  struct ClassList* next;
};

struct FileIndexList {
  const char* name;
  int index;
  struct FileIndexList* next;
};

struct FileIndexList file_index_table[256];

int current_file_count;

struct Object* current_running_object;

/*struct BytecodeFileList {
  const char* name;
  struct BytecodeFile* file;
  struct BytecodeFileList* next;
};

struct BytecodeFile {
  const char* location;
  struct Memory* memory;
  struct FuncList func_table[256];
  struct ClassList class_table[256];
  struct Object* object_table;
  size_t object_table_size;
  struct Object* const_object_table;
  size_t const_object_table_size;
  struct BytecodeFile* bytecode_file;
  struct BytecodeFileList bytecode_file_table[256];
};

struct BytecodeFileList bytecode_file_table[256];

struct BytecodeFileList global_bytecode_file_table[256];*/

func_ptr GetFunction(const char* name);
FuncInfo GetCustomFunction(const char* name, size_t* args, size_t args_size);

struct Memory* global_memory;

struct FuncList func_table[256];

struct ClassList class_table[256];

struct FreeList* free_list;

bool is_big_endian;

inline void EXIT_VM(const char* func_name, const char* message) {
  fprintf(stderr, "[ERROR] %s: %s\n", func_name, message);
  exit(1);
}

void AddFreePtr(void* ptr) {
  TRACE_FUNCTION;
  struct FreeList* new_free_list =
      (struct FreeList*)malloc(sizeof(struct FreeList));
  if (new_free_list == NULL) EXIT_VM("AddFreePtr(void*)", "Out of memory.");
  new_free_list->ptr = ptr;
  new_free_list->next = free_list;
  free_list = new_free_list;
}

void FreeAllPtr() {
  TRACE_FUNCTION;
  struct FreeList* current = free_list;
  while (current != NULL) {
    struct FreeList* next = current->next;
    free(current->ptr);
    free(current);
    current = next;
  }
}

void IsBigEndian() {
  TRACE_FUNCTION;
  uint16_t test_data = 0x0011;
  is_big_endian = (*(uint8_t*)&test_data == 0x00);
}

/*int SwapInt(int x) {
  TRACE_FUNCTION;
  uint32_t ux = (uint32_t)x;
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  return (int)ux;
}*/

int64_t SwapLong(int64_t x) {
  TRACE_FUNCTION;
  uint64_t ux = (uint64_t)x;
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  return (int64_t)ux;
}

/*float SwapFloat(float x) {
  TRACE_FUNCTION;
  uint32_t ux;
  memcpy(&ux, &x, sizeof(uint32_t));
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  float result;
  memcpy(&result, &ux, sizeof(float));
  return result;
}*/

double SwapDouble(double x) {
  TRACE_FUNCTION;
  uint64_t ux;
  memcpy(&ux, &x, sizeof(uint64_t));
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  double result;
  memcpy(&result, &ux, sizeof(double));
  return result;
}

uint64_t SwapUint64t(uint64_t x) {
  TRACE_FUNCTION;
  x = ((x << 56) & 0xFF00000000000000ULL) |
      ((x << 40) & 0x00FF000000000000ULL) |
      ((x << 24) & 0x0000FF0000000000ULL) | ((x << 8) & 0x000000FF00000000ULL) |
      ((x >> 8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
      ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
  return x;
}

/*void* SwapPtr(void* ptr) {
  TRACE_FUNCTION;
  uint64_t x = (uintptr_t)ptr;
  x = ((x << 56) & 0xFF00000000000000ULL) |
      ((x << 40) & 0x00FF000000000000ULL) |
      ((x << 24) & 0x0000FF0000000000ULL) | ((x << 8) & 0x000000FF00000000ULL) |
      ((x >> 8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
      ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
  return (void*)x;
}

struct Memory* InitializeMemory(void* data, void* type, size_t size) {
  TRACE_FUNCTION;
  struct Memory* memory_ptr = (struct Memory*)malloc(sizeof(struct Memory));
  if (memory_ptr == NULL)
    EXIT_VM("InitializeMemory(void*, void*, size_t)", "Out of memory.");
  memory_ptr->data = data;
  memory_ptr->type[0] = type;
  memory_ptr->size = size;

  return memory_ptr;
}

void FreeMemory(struct Memory* memory_ptr) {
  TRACE_FUNCTION;
  free(memory_ptr);
}

int SetType(const struct Memory* memory, size_t index, uint8_t type) {
  if (index % 2 != 0) {
    return memory->type[0][index / 2] & 0x0F;
  } else {
    return (memory->type[0][index / 2] & 0xF0) >> 4;
  }
}

int WriteData(const struct Memory* memory, const size_t index,
              const void* data_ptr, const size_t size) {
  TRACE_FUNCTION;
  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);

  return 0;
}

uint8_t GetType(const struct Memory* memory, size_t index) {
  TRACE_FUNCTION;
  if (index >= object_table_size) EXIT_VM("GetType(size_t)", "Out of memory.");
  if (index % 2 != 0) {
    printf("GetType: %zu, Type: %d\n", index,
           (*(memory->type[0] + (index / 2)) & 0x0F));
    return (*(memory->type[0] + (index / 2)) & 0x0F);
  } else {
    printf("GetType: %zu, Type: %d\n", index,
           (*(memory->type[0] + (index / 2)) & 0xF0) >> 4);
    return (*(memory->type[0] + (index / 2)) & 0xF0) >> 4;
  }
}*/

struct Object* GetPtrData(size_t index) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("GetPtrData(size_t)", "Out of memory.");

  // printf("GetPtrData: %p\n", *(void**)((uintptr_t)memory->data + index));
  if (object_table[index].type[0] == 0x06) {
    return object_table[index].data.ptr_data;
  } else if (object_table[index].type[0] == 0x07) {
    struct Object reference_data = *object_table[index].data.reference_data;
    while (true) {
      switch (reference_data.type[0]) {
        case 0x06:
          return object_table[index].data.ptr_data;
        case 0x07:
          reference_data = *reference_data.data.reference_data;
          break;
        case 0x08:
          reference_data = *reference_data.data.const_data;
          break;
        default:
          // printf("Type: %d\n", reference_data.type[0]);
          EXIT_VM("GetPtrData(size_t)", "Unsupported type.");
          break;
      }
    }
  } else if (object_table[index].type[0] == 0x08) {
    struct Object const_data = *object_table[index].data.const_data;
    while (true) {
      switch (const_data.type[0]) {
        case 0x06:
          return object_table[index].data.ptr_data;
        case 0x07:
          const_data = *const_data.data.reference_data;
          break;
        case 0x08:
          const_data = *const_data.data.const_data;
          break;
        default:
          // printf("Type: %d\n", const_data.type[0]);
          EXIT_VM("GetPtrData(size_t)", "Unsupported type.");
          break;
      }
    }
  } else {
    // printf("Type: %d\n", object_table[index].type[0]);
    // printf("Type: %i", object_table[index].type[0]);
    EXIT_VM("GetPtrData(size_t)", "Unsupported Type.");
  }
  return NULL;
}

struct Object* GetPtrObjectData(struct Object* object) {
  TRACE_FUNCTION;

  if (object->type[0] == 0x06) {
    return object->data.ptr_data;
  } else if (object->type[0] == 0x07) {
    struct Object reference_data = *object->data.reference_data;
    while (true) {
      switch (reference_data.type[0]) {
        case 0x06:
          return object->data.ptr_data;
        case 0x07:
          reference_data = *reference_data.data.reference_data;
          break;
        case 0x08:
          reference_data = *reference_data.data.const_data;
          break;
        default:
          EXIT_VM("GetPtrObjectData(struct Object*)", "Unsupported type.");
          break;
      }
    }
  } else if (object->type[0] == 0x08) {
    struct Object const_data = *object->data.const_data;
    while (true) {
      switch (const_data.type[0]) {
        case 0x06:
          return object->data.ptr_data;
        case 0x07:
          const_data = *const_data.data.reference_data;
          break;
        case 0x08:
          const_data = *const_data.data.const_data;
          break;
        default:
          EXIT_VM("GetPtrObjectData(struct Object*)", "Unsupported type.");
          break;
      }
    }
  } else {
    EXIT_VM("GetPtrObjectData(struct Object*)", "Unsupported Type.");
  }
  return NULL;
}

int8_t GetByteData(size_t index) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("GetByteData(size_t)", "Out of memory.");
  switch (object_table[index].type[0]) {
    case 0x01:
      return object_table[index].data.byte_data;
    case 0x02:
      return object_table[index].data.long_data;
    case 0x03:
      return object_table[index].data.double_data;
    case 0x04:
      return object_table[index].data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *object_table[index].data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetByteData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *object_table[index].data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetByteData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetByteData(size_t)", "Invalid type.");
      break;
  }
  return -1;
}

int8_t GetByteObjectData(struct Object* data) {
  TRACE_FUNCTION;
  if (data == NULL)
    EXIT_VM("GetByteObjectData(struct Object*)", "data is NULL.");
  switch (data->type[0]) {
    case 0x01:
      return data->data.byte_data;
    case 0x02:
      return data->data.long_data;
    case 0x03:
      return data->data.double_data;
    case 0x04:
      return data->data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *data->data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetByteObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *data->data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetByteObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetByteObjectData(struct Object*)", "Invalid type.");
      break;
  }
  return -1;
}

/*int GetIntData(size_t index) {
  TRACE_FUNCTION;
  switch (object_table[index].type[0]) {
    case 0x01:
      if (index >= object_table_size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      return object_table[index].data.byte_data;
    case 0x02:
      if (index >= object_table_size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      return object_table[index].data.long_data;
    case 0x03:
      if (index >= object_table_size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      return object_table[index].data.double_data;
    case 0x04:
      if (index >= object_table_size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      return object_table[index].data.uint64t_data;
    default:
      EXIT_VM("GetIntData(size_t)", "Invalid type.");
      break;
  }
  return -1;
}*/

int64_t GetLongData(size_t index) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("GetLongData(size_t)", "Out of memory.");
  switch (object_table[index].type[0]) {
    case 0x01:
      return object_table[index].data.byte_data;
    case 0x02:
      // printf("GetLongData: %lld\n", object_table[index].data.long_data);
      return object_table[index].data.long_data;
    case 0x03:
      return object_table[index].data.double_data;
    case 0x04:
      return object_table[index].data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *object_table[index].data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            // printf("type: %i", reference_data.type[0]);
            EXIT_VM("GetLongData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *object_table[index].data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetLongData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetLongData(size_t)", "Invalid type.");
      break;
  }
  return -1;
}

int64_t GetLongObjectData(struct Object* object) {
  TRACE_FUNCTION;
  switch (object->type[0]) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      return object->data.long_data;
    case 0x03:
      return object->data.double_data;
    case 0x04:
      return object->data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *object->data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            // printf("type: %i", reference_data.type[0]);
            EXIT_VM("GetLongObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *object->data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetLongObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetLongObjectData(struct Object*)", "Invalid type.");
      break;
  }
  return -1;
}

/*float GetFloatData(size_t index) {
  TRACE_FUNCTION;
  switch (object_table[index].type[0]) {
    case 0x01:
      if (index >= object_table_size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return object_table[index].data.byte_data;
    case 0x02:
      if (index >= object_table_size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return object_table[index].data.long_data;
    case 0x03:
      if (index >= object_table_size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return object_table[index].data.double_data;
    case 0x04:
      if (index >= object_table_size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return object_table[index].data.uint64t_data;
    default:
      EXIT_VM("GetFloatData(size_t)", "Invalid type.");
      break;
  }
  return -1;
}*/

double GetDoubleData(size_t index) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("GetDoubleData(size_t)", "Out of memory.");
  switch (object_table[index].type[0]) {
    case 0x01:
      return object_table[index].data.byte_data;
    case 0x02:
      return object_table[index].data.long_data;
    case 0x03:
      return object_table[index].data.double_data;
    case 0x04:
      return object_table[index].data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *object_table[index].data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetDoubleData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *object_table[index].data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetDoubleData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetDoubleData(size_t)", "Invalid type.");
      break;
  }
  return -1;
}

double GetDoubleObjectData(struct Object* object) {
  TRACE_FUNCTION;
  switch (object->type[0]) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      return object->data.long_data;
    case 0x03:
      return object->data.double_data;
    case 0x04:
      return object->data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *object->data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetDoubleObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *object->data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetDoubleObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetDoubleObjectData(struct Object*)", "Invalid type.");
      break;
  }
  return -1;
}

uint64_t GetUint64tData(size_t index) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("GetUint64tData(size_t)", "Out of memory.");
  switch (object_table[index].type[0]) {
    case 0x01:
      return object_table[index].data.byte_data;
    case 0x02:
      return object_table[index].data.long_data;
    case 0x03:
      return object_table[index].data.double_data;
    case 0x04:
      return object_table[index].data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *object_table[index].data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetUint64tData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *object_table[index].data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetUint64tData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetUint64tData(size_t)", "Invalid type.");
      break;
  }
  return 0;
}

uint64_t GetUint64tObjectData(struct Object* object) {
  TRACE_FUNCTION;
  if (object == NULL)
    EXIT_VM("GetUint64tObjectData(struct Object*)", "object is NULL.");
  switch (object->type[0]) {
    case 0x01:
      return object->data.byte_data;
    case 0x02:
      return object->data.long_data;
    case 0x03:
      return object->data.double_data;
    case 0x04:
      return object->data.uint64t_data;
    case 0x07: {
      struct Object reference_data = *object->data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x01:
            return reference_data.data.byte_data;
          case 0x02:
            return reference_data.data.long_data;
          case 0x03:
            return reference_data.data.double_data;
          case 0x04:
            return reference_data.data.uint64t_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetUint64tObjectData(struct Object*)",
                    "Unsupported type.");
            break;
        }
      }
    }
    case 0x08: {
      struct Object const_data = *object->data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x01:
            return const_data.data.byte_data;
          case 0x02:
            return const_data.data.long_data;
          case 0x03:
            return const_data.data.double_data;
          case 0x04:
            return const_data.data.uint64t_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetUint64tObjectData(struct Object*)",
                    "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetUint64tObjectData(struct Object*)", "Invalid type.");
      break;
  }
  return 0;
}

const char* GetStringData(size_t index) {
  TRACE_FUNCTION;
  switch (object_table[index].type[0]) {
    case 0x05:
      if (index >= object_table_size)
        EXIT_VM("GetStringData(size_t)", "Out of memory.");
      return object_table[index].data.string_data;

    case 0x07: {
      struct Object reference_data = *object_table[index].data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x05:
            return reference_data.data.string_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetStringData(size_t)", "Unsupported type.");
            break;
        }
      }
    }

    case 0x08: {
      struct Object const_data = *object_table[index].data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x05:
            return const_data.data.string_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetStringData(size_t)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      // printf("Index: %zu\n", index);
      // printf("Type: %d\n", object_table[index].type[0]);
      EXIT_VM("GetStringData(size_t)", "Invalid type.");
      break;
  }
  return NULL;
}

const char* GetStringObjectData(struct Object* object) {
  TRACE_FUNCTION;
  switch (object->type[0]) {
    case 0x05:
      return object->data.string_data;

    case 0x07: {
      struct Object reference_data = *object->data.reference_data;
      while (true) {
        switch (reference_data.type[0]) {
          case 0x05:
            return reference_data.data.string_data;
          case 0x07:
            reference_data = *reference_data.data.reference_data;
            break;
          case 0x08:
            reference_data = *reference_data.data.const_data;
            break;
          default:
            EXIT_VM("GetStringObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }

    case 0x08: {
      struct Object const_data = *object->data.const_data;
      while (true) {
        switch (const_data.type[0]) {
          case 0x05:
            return const_data.data.string_data;
          case 0x07:
            const_data = *const_data.data.reference_data;
            break;
          case 0x08:
            const_data = *const_data.data.const_data;
            break;
          default:
            EXIT_VM("GetStringObjectData(struct Object*)", "Unsupported type.");
            break;
        }
      }
    }
    default:
      EXIT_VM("GetStringObjectData(struct Object*)", "Invalid type.");
      break;
  }
  return NULL;
}
struct Object* GetObjectData(size_t index) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("GetObjectData(size_t)", "Out of memory.");

  if (object_table[index].type[0] == 0x09) {
    return object_table[index].data.object_data;
  } else if (object_table[index].type[0] == 0x07) {
    struct Object reference_data = *object_table[index].data.reference_data;
    while (true) {
      switch (reference_data.type[0]) {
        case 0x07:
          reference_data = *reference_data.data.reference_data;
          break;
        case 0x08:
          reference_data = *reference_data.data.const_data;
          break;
        case 0x09:
          return reference_data.data.object_data;
        default:
          EXIT_VM("GetObjectData(size_t)", "Unsupported type.");
          break;
      }
    }
  } else if (object_table[index].type[0] == 0x08) {
    struct Object const_data = *object_table[index].data.const_data;
    while (true) {
      switch (const_data.type[0]) {
        case 0x07:
          const_data = *const_data.data.reference_data;
          break;
        case 0x08:
          const_data = *const_data.data.const_data;
          break;
        case 0x09:
          return const_data.data.object_data;
        default:
          EXIT_VM("GetObjectData(size_t)", "Unsupported type.");
          break;
      }
    }
  } else {
    EXIT_VM("GetObjectData(size_t)", "Unsupported Type.");
  }
  return NULL;
}

struct Object* GetObjectObjectData(struct Object* data) {
  TRACE_FUNCTION;

  if (data->type[0] == 0x09) {
    return data->data.object_data;
  } else if (data->type[0] == 0x07) {
    struct Object reference_data = *data->data.reference_data;
    while (true) {
      switch (reference_data.type[0]) {
        case 0x07:
          reference_data = *reference_data.data.reference_data;
          break;
        case 0x08:
          reference_data = *reference_data.data.const_data;
          break;
        case 0x09:
          return reference_data.data.object_data;
        default:
          EXIT_VM("GetObjectObjectData(struct Object*)", "Unsupported type.");
          break;
      }
    }
  } else if (data->type[0] == 0x08) {
    struct Object const_data = *data->data.const_data;
    while (true) {
      switch (const_data.type[0]) {
        case 0x07:
          const_data = *const_data.data.reference_data;
          break;
        case 0x08:
          const_data = *const_data.data.const_data;
          break;
        case 0x09:
          return const_data.data.object_data;
        default:
          EXIT_VM("GetObjectObjectData(struct Object*)", "Unsupported type.");
          break;
      }
    }
  } else {
    EXIT_VM("GetObjectObjectData(struct Object*)", "Unsupported Type.");
  }
  return NULL;
}

struct Object* GetOriginData(struct Object* object) {
  TRACE_FUNCTION;
  while (true) {
    if (object == NULL)
      EXIT_VM("GetOriginData(struct Object*)", "object is NULL.");
    switch (object->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x09:
        return object;

      case 0x07:
        object = object->data.reference_data;
        break;

      case 0x08:
        object = object->data.const_data;
        break;

      default:
        EXIT_VM("GetOriginData(struct Object*)", "Unsupported type.");
        break;
    }
  }
}

struct Object* GetOriginDataWithoutConst(struct Object* object) {
  TRACE_FUNCTION;
  while (true) {
    if (object == NULL)
      EXIT_VM("GetOriginDataWithoutConst(struct Object*)", "object is NULL.");
    switch (object->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x09:
      case 0x08:
        return object;

      case 0x07:
        object = object->data.reference_data;
        break;

      default:
        EXIT_VM("GetOriginDataWithoutConst(struct Object*)",
                "Unsupported type.");
        break;
    }
  }
}

/*void SetByteData(size_t, int8_t);
void SetLongData(size_t, int64_t);
void SetDoubleData(size_t, double);
void SetUint64tData(size_t, uint64_t);*/

void SetPtrData(size_t index, struct Object* ptr) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetPtrData(size_t,struct Object*)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetPtrData(size_t,struct Object*)", "Cannot change const data.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;
  if(data->type[0]==0x08)EXIT_VM("SetPtrData(size_t,struct Object*)", "Cannot change const type.");
  if (data->const_type && data->type[0] != 0x06)
    EXIT_VM("SetPtrData(size_t,struct Object*)", "Cannot change const type.");

  if (ptr == NULL) {
    data->type[0] = 0x06;
    data->data.ptr_data = ptr;
    return;
  }

  while (ptr->type[0] == 0x07) ptr = ptr->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x06;
    data->data.ptr_data = ptr;
    return;
  }

  size_t size = 0;
  bool is_end = false;
  while (!is_end) {
    switch (data->type[size]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        size++;
        is_end = true;
        break;

      case 0x06:
      case 0x07:
      case 0x08:
        size++;
        break;

      default:
        EXIT_VM("SetPtrData(size_t,struct Object*)", "Unsupported type.");
        break;
    }
  }

  struct Object* temp = ptr + 1;
  for (size_t i = 1; i < size; i++) {
    if (temp == NULL)
      EXIT_VM("SetPtrData(size_t,struct Object*)", "Invalid ptr.");
    if (data->type[i] == 0x00) break;
    if (temp->type[0] != data->type[i]) {
      // printf("%i,%i", temp->type[0], data->type[i]);
      EXIT_VM("SetPtrData(size_t,struct Object*)", "Invalid type.");
    }
    switch (temp->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        break;
      case 0x06:
        temp = temp->data.ptr_data;
        break;
      case 0x07:
        temp = temp->data.reference_data;
        break;
      case 0x08:
        temp = temp->data.const_data;
        break;
      default:
        EXIT_VM("SetPtrData(size_t,struct Object*)", "Unsupported type.");
        break;
    }
  }

  data->data.ptr_data = ptr;
}

void SetByteData(size_t index, int8_t value) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetByteData(size_t,int8_t)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetByteData(size_t,int8_t)", "Cannot change const data.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;
  if(data->type[0]==0x08)EXIT_VM("SetByteData(size_t,int8_t)", "Cannot change const type.");

  if (data->const_type && data->type[0] != 0x01) {
    switch (data->type[0]) {
      case 0x02:
        SetLongData(index, value);
        return;
      case 0x03:
        SetDoubleData(index, value);
        return;
      case 0x04:
        SetUint64tData(index, value);
        return;
      default:
        break;
    }
    EXIT_VM("SetByteData(size_t,int8_t)", "Cannot change const type.");
  }
  data->type[0] = 0x01;
  data->data.byte_data = value;
}

/*void SetIntData(size_t index, int value) {
  TRACE_FUNCTION;
  // printf("SetIntData: %zu, value: %d\n", index, value);
  switch (object_table[index].type[0]) {
    case 0x01:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.byte_data = value;
      break;
    case 0x02:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x03:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.long_data = value;
      break;
    case 0x04:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x05:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.double_data = value;
      break;
    case 0x06:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.uint64t_data = value;
      break;
    default:
      EXIT_VM("SetIntData(size_t, int)", "Invalid type.");
      break;
  }
  // printf("SetIntData: %zu, Result: %d\n", index, GetIntData(index));
}*/

void SetLongData(size_t index, int64_t value) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetLongData(size_t,long)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetLongData(size_t,long)", "Cannot change const data.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;

  if(data->type[0]==0x08)EXIT_VM("SetLongData(size_t,long)", "Cannot change const type.");

  if (data->const_type && data->type[0] != 0x02) {
    switch (data->type[0]) {
      case 0x01:
        SetByteData(index, value);
        return;
      case 0x03:
        SetDoubleData(index, value);
        return;
      case 0x04:
        SetUint64tData(index, value);
        return;
      default:
        // printf("%zu,%i,%lld", index, data->type[0], value);
        break;
    }
    EXIT_VM("SetLongData(size_t,long)", "Cannot change const type.");
  }
  data->type[0] = 0x02;
  data->data.long_data = value;
}

/*void SetFloatData(size_t index, float value) {
  TRACE_FUNCTION;
  // printf("SetFloatData: %f\n", value);
  switch (object_table[index].type[0]) {
    case 0x01:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.byte_data = value;
      break;
    case 0x02:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x03:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.long_data = value;
      break;
    case 0x04:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x05:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.double_data = value;
      break;
    case 0x06:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.uint64t_data = value;
      break;
    default:
      EXIT_VM("SetFloatData(size_t, float)", "Invalid type.");
      break;
  }
}*/

void SetDoubleData(size_t index, double value) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetDoubleData(size_t,double)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetDoubleData(size_t,double)", "Cannot change const data.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;
  if(data->type[0]==0x08)EXIT_VM("SetDoubleData(size_t,double)", "Cannot change const type.");

  if (data->const_type && data->type[0] != 0x03) {
    switch (data->type[0]) {
      case 0x01:
        SetByteData(index, value);
        return;
      case 0x02:
        SetLongData(index, value);
        return;
      case 0x04:
        SetUint64tData(index, value);
        return;
      default:
        break;
    }
    EXIT_VM("SetDoubleData(size_t,double)", "Cannot change const type.");
  }

  data->type[0] = 0x03;
  data->data.double_data = value;
}

void SetUint64tData(size_t index, uint64_t value) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetUint64tData(size_t,uint64_t)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetUint64tData(size_t,uint64_t)", "Cannot change const data.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;
  if(data->type[0]==0x08)EXIT_VM("SetUint64tData(size_t,uint64_t)", "Cannot change const type.");

  if (data->const_type && data->type[0] != 0x04) {
    switch (data->type[0]) {
      case 0x01:
        SetByteData(index, value);
        return;
      case 0x02:
        SetLongData(index, value);
        return;
      case 0x03:
        SetDoubleData(index, value);
        return;
      default:
        break;
    }
    EXIT_VM("SetUint64tData(size_t,uint64_t)", "Cannot change const type.");
  }

  data->type[0] = 0x04;
  data->data.uint64t_data = value;
}

void SetStringData(size_t index, const char* string) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetStringData(size_t,const char*)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetStringData(size_t,const char*)", "Cannot change const data.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) {
    data = data->data.reference_data;
    // printf("REF\n");
  }
  if(data->type[0]==0x08)EXIT_VM("SetStringData(size_t,const char*)", "Cannot change const type.");

  if (data->const_type && data->type[0] != 0x05) {
    // printf("%zu,%i,%s", index, data->type[0], string);
    EXIT_VM("SetStringData(size_t,const char*)", "Cannot change const type.");
  }

  data->type[0] = 0x05;
  data->data.string_data = string;
}

void SetReferenceData(size_t index, struct Object* object) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetReferenceData(size_t,struct Object*)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetReferenceData(size_t,struct Object*)",
            "Cannot change const data.");

  struct Object* data = object_table + index;
  // while (data->type[0] == 0x07) data = data->data.reference_data;
  if(data->type[0]==0x08)EXIT_VM("SetReferenceData(size_t,struct Object*)", "Cannot change const type.");

  if (object_table[index].const_type && object_table[index].type[0] != 0x07)
    EXIT_VM("SetReferenceData(size_t,struct Object*)",
            "Cannot change const type.");

  if (object == NULL) {
    EXIT_VM("SetReferenceData(size_t,struct Object*)", "object is NULL.");
    // data->type[0] = 0x07;
    // data->data.reference_data = object;
    return;
  }

  // while (object->type[0] == 0x07) object = object->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x07;
    data->data.reference_data = object;
    return;
  }

  size_t size = 0;
  bool is_end = false;
  while (!is_end) {
    switch (data->type[size]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        size++;
        is_end = true;
        break;

      case 0x06:
      case 0x07:
      case 0x08:
        size++;
        break;

      default:
        EXIT_VM("SetReferenceData(size_t,struct Object*)", "Unsupported type.");
        break;
    }
  }

  struct Object* temp = object;
  for (size_t i = 1; i < size; i++) {
    if (temp == NULL)
      EXIT_VM("SetReferenceData(size_t,struct Object*)", "Invalid ptr.");
    if (data->type[i] == 0x00) break;
    if (temp->type[0] != data->type[i])
      EXIT_VM("SetReferenceData(size_t,struct Object*)", "Invalid type.");
    switch (temp->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        break;
      case 0x06:
        temp = temp->data.ptr_data;
        break;
      case 0x07:
        temp = temp->data.reference_data;
        break;
      case 0x08:
        temp = temp->data.const_data;
        break;
      default:
        EXIT_VM("SetReferenceData(size_t,struct Object*)", "Unsupported type.");
        break;
    }
  }

  data->data.reference_data = object;
}

void SetConstData(size_t index, struct Object* object) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetConstData(size_t,struct Object*)", "Out of memory.");
  /*if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetConstData(size_t,struct Object*)", "Cannot change const
    data.");*/

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;
  //if(data->const_type &&data->type[0]==0x08)EXIT_VM("SetConstData(size_t,struct Object*)", "Cannot change const type.");

  object=GetOriginData(object);

  if (object_table[index].const_type && object_table[index].type[0] != 0x08)
    EXIT_VM("SetConstData(size_t,struct Object*)", "Cannot change const type.");

  if (object == NULL) {
    data->type[0] = 0x08;
    data->data.const_data = object;
    return;
  }

  while (object->type[0] == 0x07) object = object->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x08;
    data->data.const_data = object;
    return;
  }

  size_t size = 0;
  bool is_end = false;
  while (!is_end) {
    switch (data->type[size]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
        size++;
        is_end = true;
        break;

      case 0x06:
      case 0x07:
      case 0x08:
        size++;
        break;

      default:
        EXIT_VM("SetConstData(size_t,struct Object*)", "Unsupported type.");
        break;
    }
  }

  struct Object* temp = object;
  for (size_t i = 1; i < size; i++) {
    if (temp == NULL)
      EXIT_VM("SetConstData(size_t,struct Object*)", "Invalid ptr.");
    if (data->type[i] == 0x00) break;
    if (temp->type[0] != data->type[i])
      EXIT_VM("SetConstData(size_t,struct Object*)", "Invalid type.");
    switch (temp->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
        break;
      case 0x06:
        temp = temp->data.ptr_data;
        break;
      case 0x07:
        temp = temp->data.reference_data;
        break;
      case 0x08:
        temp = temp->data.const_data;
        break;
      default:
        EXIT_VM("SetConstData(size_t,struct Object*)", "Unsupported type.");
        break;
    }
  }

  data->data.const_data = object;
}
void SetObjectData(size_t index, struct Object* object) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetObjectData(size_t,struct Object*)", "Out of memory.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;
  if(data->type[0]==0x08)EXIT_VM("SetObjectData(size_t,struct Object*)", "Cannot change const type.");

  if (data->const_type && data->type != NULL && data->type[0] != 0x09)
    EXIT_VM("SetObjectData(size_t,struct Object*)",
            "Cannot change const type.");

  if (object == NULL) {
    EXIT_VM("SetObjectData(size_t,struct Object*)", "object is NULL.");
    // data->type[0] = 0x09;
    // data->data.object_data = object;
    return;
  }

  // while (object->type[0] == 0x07) object = object->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x09;
    data->data.object_data = object;
    return;
  }

  if (data->data.object_data == NULL) {
    data->type[0] = 0x09;
    data->data.object_data = object;
    return;
  }

  /*if (GetOriginData(data->data.object_data)->type[0] != 0x05 ||
      GetOriginData(object)->type[0] != 0x05) {
    EXIT_VM("SetObjectData(size_t,struct Object*)", "Invalid name type.");
  }

  if (strcmp(GetOriginData(data->data.object_data)->data.string_data,
             GetOriginData(object)->data.string_data) != 0) {
    EXIT_VM("SetObjectData(size_t,struct Object*)", "Different name type.");
  }*/

  data->data.object_data = object;
}

void SetPtrObjectData(struct Object* data, struct Object* ptr) {
  TRACE_FUNCTION;

  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetPtrObjectData(struct Object*,struct Object*)",
            "Cannot change const value.");

  if (data->const_type && data->type[0] != 0x06)
    EXIT_VM("SetPtrObjectData(struct Object*,struct Object*)",
            "Cannot change const type.");

  if (ptr == NULL) {
    data->type[0] = 0x06;
    data->data.ptr_data = ptr;
    return;
  }

  while (ptr->type[0] == 0x07) ptr = ptr->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x06;
    data->data.ptr_data = ptr;
    return;
  }

  size_t size = 0;
  bool is_end = false;
  while (!is_end) {
    switch (data->type[size]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        size++;
        is_end = true;
        break;

      case 0x06:
      case 0x07:
      case 0x08:
        size++;
        break;

      default:
        EXIT_VM("SetPtrObjectData(struct Object*,struct Object*)",
                "Unsupported type.");
        break;
    }
  }

  struct Object* temp = ptr + 1;
  for (size_t i = 1; i < size; i++) {
    if (temp == NULL)
      EXIT_VM("SetPtrObjectData(struct Object*,struct Object*)",
              "Invalid ptr.");
    if (data->type[i] == 0x00) break;
    if (temp->type[0] != data->type[i]) {
      // printf("%i,%i", temp->type[0], data->type[i]);
      EXIT_VM("SetPtrObjectData(struct Object*,struct Object*)",
              "Invalid type.");
    }
    switch (temp->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        break;
      case 0x06:
        temp = temp->data.ptr_data;
        break;
      case 0x07:
        temp = temp->data.reference_data;
        break;
      case 0x08:
        temp = temp->data.const_data;
        break;
      default:
        EXIT_VM("SetPtrObjectData(struct Object*,struct Object*)",
                "Unsupported type.");
        break;
    }
  }

  data->data.ptr_data = ptr;
}

void SetByteObjectData(struct Object* data, int8_t value) {
  TRACE_FUNCTION;

  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetByteObjectData(struct Object*,int8_t)",
            "Cannot change const value.");

  if (data->const_type && data->type[0] != 0x01) {
    switch (data->type[0]) {
      case 0x02:
        SetLongObjectData(data, value);
        return;
      case 0x03:
        SetDoubleObjectData(data, value);
        return;
      case 0x04:
        SetUint64tObjectData(data, value);
        return;
      default:
        break;
    }
    EXIT_VM("SetByteObjectData(struct Object*,int8_t)",
            "Cannot change const type.");
  }
  data->type[0] = 0x01;
  data->data.byte_data = value;
}

/*void SetIntData(size_t index, int value) {
  TRACE_FUNCTION;
  // printf("SetIntData: %zu, value: %d\n", index, value);
  switch (object_table[index].type[0]) {
    case 0x01:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.byte_data = value;
      break;
    case 0x02:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x03:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.long_data = value;
      break;
    case 0x04:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x05:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.double_data = value;
      break;
    case 0x06:
      if (index >= object_table_size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      object_table[index].data.uint64t_data = value;
      break;
    default:
      EXIT_VM("SetIntData(size_t, int)", "Invalid type.");
      break;
  }
  // printf("SetIntData: %zu, Result: %d\n", index, GetIntData(index));
}*/

void SetLongObjectData(struct Object* data, int64_t value) {
  TRACE_FUNCTION;
  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetLongObjectData(struct Object*,int64_t)",
            "Cannot change const value.");

  if (data->const_type && data->type[0] != 0x02) {
    switch (data->type[0]) {
      case 0x01:
        SetByteObjectData(data, value);
        return;
      case 0x03:
        SetDoubleObjectData(data, value);
        return;
      case 0x04:
        SetUint64tObjectData(data, value);
        return;
      default:
        break;
    }
    EXIT_VM("SetLongObjectData(struct Object*,long)",
            "Cannot change const type.");
  }
  data->type[0] = 0x02;
  data->data.long_data = value;
}

/*void SetFloatData(size_t index, float value) {
  TRACE_FUNCTION;
  // printf("SetFloatData: %f\n", value);
  switch (object_table[index].type[0]) {
    case 0x01:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.byte_data = value;
      break;
    case 0x02:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x03:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.long_data = value;
      break;
    case 0x04:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x05:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.double_data = value;
      break;
    case 0x06:
      if (index >= object_table_size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      object_table[index].data.uint64t_data = value;
      break;
    default:
      EXIT_VM("SetFloatData(size_t, float)", "Invalid type.");
      break;
  }
}*/

void SetDoubleObjectData(struct Object* data, double value) {
  TRACE_FUNCTION;
  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetDoubleObjectData(struct Object*,double)",
            "Cannot change const value.");

  if (data->const_type && data->type[0] != 0x03) {
    switch (data->type[0]) {
      case 0x01:
        SetByteObjectData(data, value);
        return;
      case 0x02:
        SetLongObjectData(data, value);
        return;
      case 0x04:
        SetUint64tObjectData(data, value);
        return;
      default:
        break;
    }
    EXIT_VM("SetDoubleObjectData(struct Object*,double)",
            "Cannot change const type.");
  }

  data->type[0] = 0x03;
  data->data.double_data = value;
}

void SetUint64tObjectData(struct Object* data, uint64_t value) {
  TRACE_FUNCTION;
  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetUint64tObjectData(struct Object*,uint64_t)",
            "Cannot change const value.");

  if (data->const_type && data->type[0] != 0x04) {
    switch (data->type[0]) {
      case 0x01:
        SetByteObjectData(data, value);
        return;
      case 0x02:
        SetLongObjectData(data, value);
        return;
      case 0x03:
        SetDoubleObjectData(data, value);
        return;
      default:
        break;
    }
    EXIT_VM("SetUint64tObjectData(struct Object*,uint64_t)",
            "Cannot change const type.");
  }

  data->type[0] = 0x04;
  data->data.uint64t_data = value;
}

void SetStringObjectData(struct Object* data, const char* string) {
  TRACE_FUNCTION;
  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetStringObjectData(struct Object*,const char*)",
            "Cannot change const value.");

  if (data->const_type && data->type[0] != 0x05) {
    // printf("%zu,%i,%s", index, data->type[0], string);
    EXIT_VM("SetStringObjectData(struct Object*,const char*)",
            "Cannot change const type.");
  }

  data->type[0] = 0x05;
  data->data.string_data = string;
}

void SetReferenceObjectData(struct Object* data, struct Object* object) {
  TRACE_FUNCTION;
  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetReferenceObjectData(struct Object*,struct Object*)",
            "Cannot change const value.");

  if (object == NULL) {
    EXIT_VM("SetReferenceObjectData(struct Object*,struct Object*)",
            "object is NULL.");
    // data->type[0] = 0x07;
    // data->data.reference_data = object;
    return;
  }

  // while (object->type[0] == 0x07) object = object->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x07;
    data->data.reference_data = object;
    return;
  }

  size_t size = 0;
  bool is_end = false;
  while (!is_end) {
    switch (data->type[size]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        size++;
        is_end = true;
        break;

      case 0x06:
      case 0x07:
      case 0x08:
        size++;
        break;

      default:
        EXIT_VM("SetReferenceObjectData(struct Object*,struct Object*)",
                "Unsupported type.");
        break;
    }
  }

  struct Object* temp = object;
  for (size_t i = 1; i < size; i++) {
    if (temp == NULL)
      EXIT_VM("SetReferenceObjectData(struct Object*,struct Object*)",
              "Invalid ptr.");
    if (data->type[i] == 0x00) break;
    if (temp->type[0] != data->type[i])
      EXIT_VM("SetReferenceObjectData(struct Object*,struct Object*)",
              "Invalid type.");
    switch (temp->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x09:
        break;
      case 0x06:
        temp = temp->data.ptr_data;
        break;
      case 0x07:
        temp = temp->data.reference_data;
        break;
      case 0x08:
        temp = temp->data.const_data;
        break;
      default:
        EXIT_VM("SetReferenceObjectData(struct Object*,struct Object*)",
                "Unsupported type.");
        break;
    }
  }

  data->data.reference_data = object;
}

void SetConstObjectData(struct Object* data, struct Object* object) {
  TRACE_FUNCTION;
  data = GetOriginDataWithoutConst(data);
  object=GetOriginData(object);
  if (data->type[0] == 0x08)
    EXIT_VM("SetConstObjectData(struct Object*,struct Object*)",
            "Cannot change const value.");

  if (object == NULL) {
    data->type[0] = 0x08;
    data->data.const_data = object;
    return;
  }

  while (object->type[0] == 0x07) object = object->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x08;
    data->data.const_data = object;
    return;
  }

  size_t size = 0;
  bool is_end = false;
  while (!is_end) {
    switch (data->type[size]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
        size++;
        is_end = true;
        break;

      case 0x06:
      case 0x07:
      case 0x08:
        size++;
        break;

      default:
        EXIT_VM("SetConstObjectData(struct Object*,struct Object*)",
                "Unsupported type.");
        break;
    }
  }

  struct Object* temp = object;
  for (size_t i = 1; i < size; i++) {
    if (temp == NULL)
      EXIT_VM("SetConstObjectData(struct Object*,struct Object*)",
              "Invalid ptr.");
    if (data->type[i] == 0x00) break;
    if (temp->type[0] != data->type[i])
      EXIT_VM("SetConstObjectData(struct Object*,struct Object*)",
              "Invalid type.");
    switch (temp->type[0]) {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
        break;
      case 0x06:
        temp = temp->data.ptr_data;
        break;
      case 0x07:
        temp = temp->data.reference_data;
        break;
      case 0x08:
        temp = temp->data.const_data;
        break;
      default:
        EXIT_VM("SetConstObjectData(struct Object*,struct Object*)",
                "Unsupported type.");
        break;
    }
  }

  data->data.const_data = object;
}
void SetObjectObjectData(struct Object* data, struct Object* object) {
  TRACE_FUNCTION;
  data = GetOriginDataWithoutConst(data);

  if (data->type[0] == 0x08)
    EXIT_VM("SetObjectObjectData(struct Object*,struct Object*)",
            "Cannot change const value.");

  if (data->const_type && data->type != NULL && data->type[0] != 0x09)
    EXIT_VM("SetObjectObjectData(struct Object*,struct Object*)",
            "Cannot change const type.");

  if (object == NULL) {
    EXIT_VM("SetObjectObjectData(struct Object*,struct Object*)",
            "object is NULL.");
    // data->type[0] = 0x09;
    // data->data.object_data = object;
    return;
  }

  // while (object->type[0] == 0x07) object = object->data.reference_data;

  if (!data->const_type) {
    data->type[0] = 0x09;
    data->data.object_data = object;
    return;
  }

  if (data->data.object_data == NULL) {
    data->type[0] = 0x09;
    data->data.object_data = object;
    return;
  }

  /*if (GetOriginData(data->data.object_data)->type[0] != 0x05 ||
      GetOriginData(object)->type[0] != 0x05) {
    EXIT_VM("SetObjectData(size_t,struct Object*)", "Invalid name type.");
  }

  if (strcmp(GetOriginData(data->data.object_data)->data.string_data,
             GetOriginData(object)->data.string_data) != 0) {
    EXIT_VM("SetObjectData(size_t,struct Object*)", "Different name type.");
  }*/

  data->data.object_data = object;
}

/*void CopyObjectData(size_t index, struct Object* object) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("CopyObjectData(size_t,struct Object*)", "Out of memory.");
  if (object == NULL) {
    EXIT_VM("CopyObjectData(size_t,struct Object*)", "object is NULL.");
  }

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;

  if (data->const_type && data->type[0] != 0x09)
    EXIT_VM("CopyObjectData(size_t,struct Object*)",
            "Cannot change const type.");

  if (data->const_type) {
    if (data->data.object_data == NULL)
      EXIT_VM("CopyObjectData(size_t,struct Object*)", "data object is NULL.");

    if (data->data.object_data->type[0] != 0x05 || object->type[0] != 0x05)
      EXIT_VM("CopyObjectData(size_t,struct Object*)", "Invalid name type.");

    if (data->data.object_data->data.string_data == NULL ||
        object->data.string_data == NULL)
      EXIT_VM("CopyObjectData(size_t,struct Object*)", "Invalid name string.");

    if (strcmp(data->data.object_data->data.string_data,
               object->data.string_data) != 0)
      EXIT_VM("CopyObjectData(size_t,struct Object*)", "Different name type.");

    struct Object* new_data = data->data.object_data + 1;
    struct Object* origin_data = object->data + 1;
    size_t length = GetUint64tObjectData(origin_data);
    for (size_t i = 0; i < length; i++) {
      new_data[i].const_type = origin_data[i].const_data;
      uint8_t* location = origin_data[i].type;
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
            EXIT_VM("CopyObjectData(size_t,struct Object*)",
                    "Unsupported type.");
            break;
        }
      }

      memcpy(new_data[i].type, origin_data[i].type, length);
      new_data[i].data = origin_data[i].data;
    }

  } else {
    struct Object* origin_data = object->data + 1;
    if (data->type[0] != 0x09 || data->data.object_data->type == NULL ||
        data->data.object_data->type[0] != 0x05 ||
        data->data.object_data == NULL ||
        strcmp(data->data.object_data->data.string_data,
               object->data.string_data) != 0) {
      struct Object* type_data = object_table + type;
      type_data = GetOriginData(type_data);
      if (type_data->type[0] == 0x05) {
        for (size_t i = 0; i < size_value; i++) {
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
            EXIT_VM("CopyObjectData(size_t,struct Object*)", "Class not
found.");
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
                  EXIT_VM("CopyObjectData(size_t,struct Object*)", "Unsupported
type."); break;
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
          printf("Class Name NEW: %s\n", class_object[0].data.string_data);
          class_object[1].const_type = true;
          class_object[1].type[0] = 0x04;
          class_object[1].data.uint64t_data = class_data->members_size;
          data[i].data.object_data = class_object;
        }
      } else {
        EXIT_VM("CopyObjectData(size_t,struct Object*)","Invalid type.");
      }

    } else {
    }
  }
}*/

size_t DecodeUleb128(const uint8_t* input, size_t* result) {
  TRACE_FUNCTION;
  *result = 0;
  size_t shift = 0;
  size_t count = 0;

  while (1) {
    uint8_t byte = input[count++];
    *result |= (byte & 0x7F) << shift;
    if ((byte & 0x80) == 0) {
      break;
    }
    shift += 7;
  }

  return count;
}

void* Get1Parament(void* ptr, size_t* first) {
  TRACE_FUNCTION;
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  return ptr;
}

void* Get2Parament(void* ptr, size_t* first, size_t* second) {
  TRACE_FUNCTION;
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, second));
  return ptr;
}

void* Get3Parament(void* ptr, size_t* first, size_t* second, size_t* third) {
  TRACE_FUNCTION;
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, second));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, third));

  // printf("Decoded Parameters: %zu, %zu, %zu\n", *first, *second, *third);
  return ptr;
}

void* Get4Parament(void* ptr, size_t* first, size_t* second, size_t* third,
                   size_t* fourth) {
  TRACE_FUNCTION;
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, second));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, third));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, fourth));
  return ptr;
}

int INVOKE(size_t* args);

size_t* GetUnknownCountParament(void** ptr) {
  TRACE_FUNCTION;
  size_t func = 0;
  size_t arg_count = 0;
  size_t return_value = 0;
  *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, &func));
  *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, &arg_count));
  *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, &return_value));

  size_t* args = malloc((arg_count + 3) * sizeof(size_t));
  args[0] = func;
  args[1] = arg_count;
  args[2] = return_value;

  for (size_t i = 3; i < arg_count + 2; i++) {
    *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, args + i));
  }

  return args;
}

size_t* GetUnknownCountParamentForClass(void** ptr) {
  TRACE_FUNCTION;
  size_t class = 0;
  size_t func = 0;
  size_t arg_count = 0;
  size_t return_value = 0;
  *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, &class));
  *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, &func));
  *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, &arg_count));
  *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, &return_value));

  size_t* args = malloc((arg_count + 4) * sizeof(size_t));
  args[0] = class;
  args[1] = func;
  args[2] = arg_count;
  args[3] = return_value;

  for (size_t i = 4; i < arg_count + 3; i++) {
    *ptr = (void*)((uintptr_t)*ptr + DecodeUleb128(*ptr, args + i));
  }

  return args;
}

int GetFuncOverloadCost(size_t* args, size_t args_size, size_t* func_args,
                        size_t func_args_size, bool va_flag) {
  int cost = 0;
  // printf("%i,%i", args_size, func_args_size);
  // printf("VAF1\n");
  if (args_size != func_args_size && !va_flag) {
    // printf("%i,%i", args_size, func_args_size);
    // printf("VAF2\n");
    return -1;
  }
  if (args_size < func_args_size) {
    // printf("VAF3\n");
    return -1;
  }
  if (func_args_size == 0)
    EXIT_VM("GetFuncOverloadCost(size_t*,size_t,size_t*,size_t,bool)",
            "Unexpected args size.");
  // printf("now out for loop1\n");
  if (func_args_size == 1) {
    // printf("ARGS SIZE == 1.");
    return 0;
  }
  // printf("now out for loop\n");
  for (size_t i = 0; i < func_args_size - 1; i++) {
    if (!object_table[func_args[i + 1]].const_type) cost += 10;
    // printf("NOW IN FOR LOOP %i\n", object_table[func_args[i + 1]].type[0]);
    if (object_table[func_args[i + 1]].const_type) {
      switch (object_table[func_args[i + 1]].type[0]) {
        case 0x01:
          if (object_table[args[i]].type[0] == 0x06 ||
              object_table[args[i]].type[0] == 0x09) {
            return -1;
          } else {
            struct Object* origin_object =
                GetOriginData(object_table + args[i]);
            switch (origin_object->type[0]) {
              case 0x01:
                break;
              case 0x02:
                cost += 1;
                break;
              case 0x03:
                cost += 3;
                break;
              case 0x04:
                cost += 2;
                break;
              default:
                return -1;
            }
          }
          break;
        case 0x02:
          if (object_table[args[i]].type[0] == 0x06 ||
              object_table[args[i]].type[0] == 0x09) {
            // printf("ERROR 1\n");
            return -1;
          } else {
            struct Object* origin_object =
                GetOriginData(object_table + args[i]);
            switch (origin_object->type[0]) {
              case 0x01:
                cost += 3;
                break;
              case 0x02:
                // printf("RUN OK\n");
                break;
              case 0x03:
                cost += 2;
                break;
              case 0x04:
                cost += 1;
                break;
              default:
                // printf("ERROR\n");
                return -1;
            }
          }
          break;
        case 0x03:
          if (object_table[args[i]].type[0] == 0x06 ||
              object_table[args[i]].type[0] == 0x09) {
            return -1;
          } else {
            struct Object* origin_object =
                GetOriginData(object_table + args[i]);
            switch (origin_object->type[0]) {
              case 0x01:
                cost += 3;
                break;
              case 0x02:
                cost += 2;
                break;
              case 0x03:
                break;
              case 0x04:
                cost += 1;
                break;
              default:
                return -1;
            }
          }
          break;
        case 0x04:
          if (object_table[args[i]].type[0] == 0x06 ||
              object_table[args[i]].type[0] == 0x09) {
            return -1;
          } else {
            struct Object* origin_object =
                GetOriginData(object_table + args[i]);
            switch (origin_object->type[0]) {
              case 0x01:
                cost += 3;
                break;
              case 0x02:
                cost += 2;
                break;
              case 0x03:
                cost += 1;
                break;
              case 0x04:
                break;
              default:
                return -1;
            }
          }
          break;
        case 0x05:
          if (object_table[args[i]].type[0] == 0x06 ||
              object_table[args[i]].type[0] == 0x09) {
            return -1;
          } else {
            struct Object* origin_object =
                GetOriginData(object_table + args[i]);
            switch (origin_object->type[0]) {
              case 0x05:
                break;
              default:
                return -1;
            }
          }
          break;
        case 0x06: {
          bool is_end = false;
          uint8_t final_type = 0x00;
          for (size_t j = 0; !is_end; j++) {
            switch (object_table[func_args[i + 1]].type[j]) {
              case 0x00:
              case 0x01:
              case 0x02:
              case 0x03:
              case 0x04:
              case 0x05:
              case 0x06:
              case 0x09:
                final_type = object_table[func_args[i + 1]].type[j];
                is_end = true;
                break;
              default:
                break;
            }
          }
          if (final_type != 0x06) return -1;
          if (object_table[func_args[i + 1]].type[1] == 0x00) break;
          is_end = false;
          final_type = 0x00;
          for (size_t j = 0; !is_end; j++) {
            if (object_table[func_args[i + 1]].type[j] == 0x00) {
              is_end = true;
              break;
            }
            if (object_table[func_args[i + 1]].type[j] !=
                object_table[args[i]].type[j]) {
              return -1;
            }
            if (object_table[func_args[i + 1]].type[j] == 0x01 ||
                object_table[func_args[i + 1]].type[j] == 0x02 ||
                object_table[func_args[i + 1]].type[j] == 0x03 ||
                object_table[func_args[i + 1]].type[j] == 0x04 ||
                object_table[func_args[i + 1]].type[j] == 0x05 ||
                object_table[func_args[i + 1]].type[j] == 0x09) {
              final_type = object_table[func_args[i + 1]].type[j];
              is_end = true;
              break;
            }
          }
          if (final_type == 0x08) {
            struct Object* arg_object = GetObjectData(args[i]);
            struct Object* func_arg_object = GetObjectData(func_args[i + 1]);
            if (strcmp(arg_object->data.string_data,
                       func_arg_object->data.string_data) != 0) {
              return -1;
            }
          }
          break;
        }
        case 0x07: {
          bool is_end = false;
          uint8_t final_type = 0x00;
          for (size_t j = 0; !is_end; j++) {
            switch (object_table[func_args[i + 1]].type[j]) {
              case 0x00:
              case 0x01:
              case 0x02:
              case 0x03:
              case 0x04:
              case 0x05:
              case 0x06:
              case 0x09:
                final_type = object_table[func_args[i + 1]].type[j];
                is_end = true;
                break;
              default:
                break;
            }
          }
          if (final_type == 0x00) break;

          is_end = false;
          uint8_t arg_final_type =
              GetOriginData(object_table + args[i])->type[0];
          /*for (size_t j = 0; !is_end; j++) {
            switch (object_table[args[i]].type[j]) {
              case 0x00:
              case 0x01:
              case 0x02:
              case 0x03:
              case 0x04:
              case 0x05:
              case 0x06:
              case 0x09:
                arg_final_type = object_table[args[i]].type[j];
                is_end = true;
                break;
              default:
                printf("TYPE: %i, \n",object_table[args[i]].type[j]);
                break;
            }
          }
          printf("TYPE: %i\n",arg_final_type);*/
          if (final_type != arg_final_type) return -1;
          if (final_type == 0x08) {
            struct Object* arg_object = GetObjectData(args[i]);
            struct Object* func_arg_object = GetObjectData(func_args[i + 1]);
            if (strcmp(arg_object->data.string_data,
                       func_arg_object->data.string_data) != 0) {
              return -1;
            }
          }
          break;
        }
        case 0x08: {
          bool is_end = false;
          uint8_t final_type = 0x00;
          for (size_t j = 0; !is_end; j++) {
            switch (object_table[func_args[i + 1]].type[j]) {
              case 0x00:
              case 0x01:
              case 0x02:
              case 0x03:
              case 0x04:
              case 0x05:
              case 0x06:
              case 0x09:
                final_type = object_table[func_args[i + 1]].type[j];
                is_end = true;
                break;
              default:
                break;
            }
          }
          if (final_type == 0x00) break;

          is_end = false;
          uint8_t arg_final_type = 0x00;
          for (size_t j = 0; !is_end; j++) {
            switch (object_table[args[i]].type[j]) {
              case 0x00:
              case 0x01:
              case 0x02:
              case 0x03:
              case 0x04:
              case 0x05:
              case 0x06:
              case 0x09:
                arg_final_type = object_table[args[i]].type[j];
                is_end = true;
                break;
              default:
                break;
            }
          }
          if (final_type != arg_final_type) return -1;
          if (final_type == 0x08) {
            struct Object* arg_object = GetObjectData(args[i]);
            struct Object* func_arg_object = GetObjectData(func_args[i + 1]);
            if (strcmp(arg_object->data.string_data,
                       func_arg_object->data.string_data) != 0) {
              return -1;
            }
          }
          break;
        }
        case 0x09: {
          bool is_end = false;
          uint8_t arg_final_type = 0x00;
          for (size_t j = 0; !is_end; j++) {
            switch (object_table[args[i]].type[j]) {
              case 0x00:
              case 0x01:
              case 0x02:
              case 0x03:
              case 0x04:
              case 0x05:
              case 0x06:
              case 0x09:
                arg_final_type = object_table[args[i]].type[j];
                is_end = true;
                break;
              default:
                break;
            }
          }
          if (arg_final_type != 0x09) return -1;
          struct Object* arg_object = GetObjectData(args[i]);
          struct Object* func_arg_object = GetObjectData(func_args[i + 1]);
          if (strcmp(arg_object->data.string_data,
                     func_arg_object->data.string_data) == 0) {
            break;
          } else {
            return -1;
          }
          break;
        }
        default:
          return -1;
      }
    }
  }
  if (args_size < func_args_size) {
    cost += (func_args_size - args_size) * 100;
  }
  return cost;
}

int NOP() {
  TRACE_FUNCTION;
  return 0;
}
int LOAD(size_t ptr, size_t operand) {
  TRACE_FUNCTION;
  if (object_table[operand].type[0] != 0x06)
    EXIT_VM("LOAD(size_t, size_t)", "Invalid type.");
  struct Object data = *object_table[ptr].data.ptr_data;
  // printf("LOAD: %zu\n", (uint64_t)data);
  switch (object_table[ptr].data.ptr_data->type[0]) {
    case 0x01:
      SetByteData(operand, data.data.byte_data);
      break;
    case 0x02:
      SetLongData(operand, data.data.long_data);
      break;
    case 0x03:
      SetDoubleData(operand, data.data.double_data);
      break;
    case 0x04:
      SetUint64tData(operand, data.data.uint64t_data);
      break;
    case 0x05:
      SetStringData(operand, data.data.string_data);
      break;
    case 0x06:
      SetPtrData(operand, data.data.ptr_data);
      break;
    case 0x07:
      SetReferenceData(operand, data.data.reference_data);
      break;
    case 0x09:
      SetPtrData(operand, data.data.object_data);
      break;
    case 0x08:
      EXIT_VM("LOAD(size_t, size_t)", "Cannot load const data.");
      break;
    default:
      EXIT_VM("LOAD(size_t, size_t)", "Invalid type.");
      break;
  }
  return 0;
}
int STORE(size_t ptr, size_t operand) {
  TRACE_FUNCTION;
  struct Object* data = GetPtrData(ptr);
  data->type[0] = object_table[operand].type[0];
  switch (data->type[0]) {
    case 0x01:
      data->data.byte_data = GetByteData(operand);
      break;
    case 0x02:
      data->data.long_data = GetLongData(operand);
      break;
    case 0x03:
      data->data.double_data = GetDoubleData(operand);
      break;
    case 0x04:
      data->data.uint64t_data = GetUint64tData(operand);
      break;
    case 0x05:
      data->data.string_data = GetStringData(operand);
      break;
    case 0x06:
      data->data.ptr_data = GetPtrData(operand);
      break;
    case 0x07:
      data->data.reference_data = object_table[operand].data.reference_data;
      break;
    case 0x08:
      data->data.const_data = object_table[operand].data.const_data;
      break;
    case 0x09:
      data->data.object_data = object_table[operand].data.object_data;
      break;
    default:
      EXIT_VM("STORE(size_t, size_t)", "Invalid type.");
      break;
  }
  return 0;
}
unsigned int hash(const char* str);

int InvokeClassFunction(size_t class, const char* name, size_t args_size,
                        size_t return_value, size_t* args);

void AddBytecodeFile(const char* file);

int INVOKE_METHOD(size_t* args);

int NEW(size_t ptr, size_t size, size_t type) {
  TRACE_FUNCTION;
  if (ptr >= object_table_size)
    EXIT_VM("NEW(size_t, size_t)", "ptr is out of memory.");
  if (size >= object_table_size)
    EXIT_VM("NEW(size_t, size_t)", "size is out of memory.");

  struct Object* type_data = object_table + type;
  type_data = GetOriginData(type_data);

  bool is_bytecode_file_main_program = false;
  struct BytecodeFileList* current_bytecode_file_table = NULL;
  if (type_data->type[0] == 0x05 && type_data->data.string_data != NULL &&
      *type_data->data.string_data == '~') {
    const char* temp_bytecode_file = type_data->data.string_data + 1;
    const char* filename_start = temp_bytecode_file;
    while (*temp_bytecode_file != '~') {
      temp_bytecode_file++;
    }
    char* file_name =
        calloc(temp_bytecode_file - filename_start + 1, sizeof(char));
    memcpy(file_name, filename_start, temp_bytecode_file - filename_start);
    temp_bytecode_file++;
    printf("YES %c.\n", *temp_bytecode_file);
    const unsigned int class_hash = hash(file_name);
    current_bytecode_file_table = &bytecode_file_table[class_hash];
    while (current_bytecode_file_table->next != NULL) {
      if (strcmp(current_bytecode_file_table->name, file_name) == 0) {
        printf("YES 0.\n");
        is_bytecode_file_main_program = true;
        break;
      }
      current_bytecode_file_table = current_bytecode_file_table->next;
    }
    if (memcmp(temp_bytecode_file, ".!__start", 9) == 0 &&
        strlen(temp_bytecode_file) == 9) {
      if (current_bytecode_file_table->object != NULL) {
        object_table[ptr].type = calloc(1, sizeof(uint8_t));
        AddFreePtr(object_table[ptr].type);
        object_table[ptr].type[0] = 0x07;
        object_table[ptr].const_type = false;
        object_table[ptr].data.reference_data =
            current_bytecode_file_table->object;
        return 0;
      }

      printf("NEW C: %s\n", type_data->data.string_data);
      AddBytecodeFile(type_data->data.string_data);

      current_bytecode_file_table = &bytecode_file_table[class_hash];
      while (current_bytecode_file_table->next != NULL) {
        if (strcmp(current_bytecode_file_table->name, file_name) == 0) {
          printf("YES 0.\n");
          is_bytecode_file_main_program = true;
          break;
        }
        current_bytecode_file_table = current_bytecode_file_table->next;
      }
    }
  }
  size_t size_value = GetUint64tData(size);

  if ((type == 0 ||
       (type_data->type[0] != 0x05 || type_data->data.string_data == NULL)) &&
      size_value == 0)
    size_value = 1;

  struct Object* data = calloc(size_value + 1, sizeof(struct Object));
  AddFreePtr(data);

  /*if (type == 0) {
    for (size_t i = 0; i < size_value; i++) {
      uint8_t* type = calloc(1, sizeof(uint8_t));
      data[i].type = 0x00;
      data[i].const_type = false;
      AddFreePtr(type);
    }
  } else {
    struct Object* type_data = object_table + type;
    type_data = GetOriginData(type_data);

    if (type_data->type[0] == 0x06) {
      struct Object* current_type = type_data->data.ptr_data;
      for (size_t i = 0; i < size_value; i++) {
        uint8_t* type = calloc(1, sizeof(uint8_t));
        data[i].type = GetByteObjectData(current_type);
        data[i].const_type = false;
        AddFreePtr(type);
        current_type++;
      }
    } else {
      for (size_t i = 0; i < size_value; i++) {
        uint8_t* type = calloc(1, sizeof(uint8_t));
        data[i].type = GetByteData(type);
        data[i].const_type = false;
        AddFreePtr(type);
      }
    }
  }*/

  uint8_t* type_ptr = calloc(1, sizeof(uint8_t));
  data[0].type = type_ptr;
  type_ptr[0] = 0x04;
  data[0].const_type = true;
  data[0].data.uint64t_data = size_value;

  AddFreePtr(type_ptr);

  if (type == 0) {
    for (size_t i = 1; i < 2 /*size_value + 1*/; i++) {
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
          // printf("%s,%s\n", current_class_table->class.name, class);
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
        // printf("Class Name NEW: %s\n", class_object[0].data.string_data);
        class_object[1].const_type = true;
        class_object[1].type[0] = 0x04;
        class_object[1].data.uint64t_data = class_data->members_size;
        data[i].data.object_data = class_object;

        /*uint8_t* ptr_type = object_table[ptr].type;
        bool ptr_is_const = object_table[ptr].const_type;
        object_table[ptr].type = calloc(1, sizeof(uint8_t));
        object_table[ptr].type[0] = 0x07;
        object_table[ptr].const_type = false;
        union Data ptr_data = object_table[ptr].data;
        object_table[ptr].data.reference_data = data + i;
        // InvokeClassFunction(ptr, "@constructor", 1, 0, NULL);
        object_table[ptr].type = ptr_type;
        object_table[ptr].const_type = ptr_is_const;
        object_table[ptr].data = ptr_data;*/
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
          // printf("Class Name NEW: %s\n", class_object[0].data.string_data);
          class_object[1].const_type = true;
          class_object[1].type[0] = 0x04;
          class_object[1].data.uint64t_data = class_data->members_size;
          data[i].data.object_data = class_object;

          /*uint8_t* ptr_type = object_table[ptr].type;
          bool ptr_is_const = object_table[ptr].const_type;

          object_table[ptr].type = calloc(1, sizeof(uint8_t));
          object_table[ptr].type[0] = 0x07;
          object_table[ptr].const_type = false;
          union Data ptr_data = object_table[ptr].data;
          object_table[ptr].data.reference_data = data + i;
          // InvokeClassFunction(ptr, "@constructor", 1, 0, NULL);
          object_table[ptr].type = ptr_type;
          object_table[ptr].const_type = ptr_is_const;
          object_table[ptr].data = ptr_data;*/
        }
        /*size_t i = 0;
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
        // printf("Class Name NEW: %s\n", class_object[0].data.string_data);
        class_object[1].const_type = true;
        class_object[1].type[0] = 0x04;
        class_object[1].data.uint64t_data = class_data->members_size;
        data[i].data.object_data = class_object;

        uint8_t* ptr_type = object_table[ptr].type;
        bool ptr_is_const = object_table[ptr].const_type;
        object_table[ptr].type = calloc(1, sizeof(uint8_t));
        object_table[ptr].type[0] = 0x07;
        object_table[ptr].const_type = false;
        union Data ptr_data = object_table[ptr].data;
        object_table[ptr].data.reference_data = data + i;
        InvokeClassFunction(ptr, "@constructor", 1, 0, NULL);
        object_table[ptr].type = ptr_type;
        object_table[ptr].const_type = ptr_is_const;
        object_table[ptr].data = ptr_data;*/
      }
    } else {
      for (size_t i = 1; i < 2 /*size_value + 1*/; i++) {
        data[i].type = type_data->type;
        data[i].const_type = true;
      }
    }
  }

  if (size_value == 0 && type_data->type[0] == 0x05 &&
      type_data->data.string_data != NULL) {
    // printf("TEST");
    SetObjectData(ptr, data->data.object_data);
    if (is_bytecode_file_main_program) {
      printf("YES 1.\n");
      current_bytecode_file_table->object = data;
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
      class_data->memory->object_table[2] = *data;

      struct Memory origin_memory = {object_table, object_table_size,
                                     const_object_table,
                                     const_object_table_size};
      object_table = class_data->memory->object_table;
      object_table_size = class_data->memory->object_table_size;
      const_object_table = class_data->memory->const_object_table;
      const_object_table_size = class_data->memory->const_object_table_size;

      printf("CONSTRUCTOR FILE: %s\n", type_data->data.string_data);
      struct Object zero_object = *object_table;
      uint8_t str_type = 0x05;
      object_table[0].type = &str_type;
      object_table[0].const_type = false;
      object_table[0].data.string_data = "@constructor";
      size_t args[] = {2, 0, 1, 0};
      INVOKE_METHOD(args);
      *object_table = zero_object;

      object_table = origin_memory.object_table;
      object_table_size = origin_memory.object_table_size;
      const_object_table = origin_memory.const_object_table;
      const_object_table_size = origin_memory.const_object_table_size;
    }
  } else {
    // printf("TEST 1");
    SetPtrData(ptr, data);
  }
  // WriteData(memory, ptr, &data, sizeof(data));
  return 0;
}

int CrossMemoryNew(struct Memory* memory, size_t ptr, size_t size,
                   size_t type) {
  TRACE_FUNCTION;
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
    for (size_t i = 1; i < 2 /*size_value + 1*/; i++) {
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
      for (size_t i = 1; i < 2 /*size_value + 1*/; i++) {
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
  // free(GetPtrData(ptr));
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
    /*(array_object+1+index)->const_type = true;
    (array_object+1+index)->type = (array_object+1)->type;
    size_t class_data_size = GetUint64tObjectData(
    (array_object+1)->data.object_data + 1);
    (array_object+1+index)->data.object_data =
    calloc(class_data_size,sizeof(struct Object));
    if((array_object+1+index)->data.object_data==NULL)EXIT_VM("ARRAY(size_t,size_t,size_t)","Object
    is NULL."); (array_object+1+index)->data.object_data->const_type = true;
    (array_object+1+index)->data.object_data->type =
    (array_object+1)->data.object_data->type;
    (array_object+1+index)->data.object_data->data =
    (array_object+1)->data.object_data->data;
    ((array_object+1+index)->data.object_data+1)->const_type = true;
    ((array_object+1+index)->data.object_data+1)->type =
    ((array_object+1)->data.object_data+1)->type;
    ((array_object+1+index)->data.object_data+1)->data =
    ((array_object+1)->data.object_data+1)->data; SetReferenceData(result,
    array_object + 1 + index); printf("DEBUG ARRAY\n");
    InvokeClassFunction(result, "@constructor", 1, result, NULL);*/
    SetReferenceData(result, array_object + 1 + index);
    InvokeCustomFunction((array_object + 1)->data.object_data->data.string_data,
                         1, result, NULL);
  }

  // printf("DEBUG ARRAY OUT OF OBJECT.\n");
  // printf("t: %i\n", (array_object + 1 + index)->type[0]);
  SetReferenceData(result, array_object + 1 + index);

  return 0;
}
int PTR(size_t index, size_t ptr) {
  TRACE_FUNCTION;
  // printf("index: %zu\n", index);
  // printf("PTR: %p\n", (void*)((uintptr_t)memory->data + index));
  SetPtrData(ptr, object_table + index);
  return 0;
}
int ADD(size_t result, size_t operand1, size_t operand2) {
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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

  /*if (operand1_data->type[0] == operand2_data->type[0]) {
    switch (operand1_data->type[0]) {
      case 0x01:
        if (GetByteData(operand1) / GetByteData(operand2) > INT8_MAX ||
            GetByteData(operand1) / GetByteData(operand2) < INT8_MIN) {
          SetLongData(result, GetByteData(operand1) / GetByteData(operand2));
        } else {
          SetByteData(result, GetByteData(operand1) / GetByteData(operand2));
        }
        break;
      case 0x02:
        SetLongData(result, GetLongData(operand1) / GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) / GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("DIV(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  } else {
    if (operand1_data->type[0] == 0x05 || operand2_data->type[0] == 0x05)
      EXIT_VM("DIV(size_t,size_t,size_t)", "Unsupported type.");
    uint8_t result_type = operand1_data->type[0] > operand2_data->type[0]
                              ? operand1_data->type[0]
                              : operand2_data->type[0];
    switch (result_type) {
      case 0x02:
        SetLongData(result, GetLongData(operand1) / GetLongData(operand2));
        break;
      case 0x03:
        SetDoubleData(result,
                      GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x04:
        SetUint64tData(result,
                       GetUint64tData(operand1) / GetUint64tData(operand2));
        break;
      default:
        EXIT_VM("DIV(size_t,size_t,size_t)", "Unsupported type.");
        break;
    }
  }*/
  return 0;
}
int REM(size_t result, size_t operand1, size_t operand2) {
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
  if (result >= object_table_size)
    EXIT_VM("REFER(size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("REFER(size_t,size_t)", "Out of object_table_size.");

  // printf("REFER: %zu , %zu\n", result,operand1);
  struct Object* data = GetOriginData(object_table + operand1);

  SetReferenceData(result, data);

  return 0;
}
size_t IF(size_t condition, size_t true_branche, size_t false_branche) {
  TRACE_FUNCTION;
  // printf("condition: %d\n", GetByteData(condition));
  if (GetByteData(condition) != 0) {
    return true_branche;
  } else {
    return false_branche;
  }
}
int AND(size_t result, size_t operand1, size_t operand2) {
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
  if (args == NULL) EXIT_VM("INVOKE(size_t*)", "Invalid args.");
  // printf("INVOKE: %zu\n", args[0]);
  size_t func = args[0];
  size_t arg_count = args[1];
  size_t return_value = args[2];
  size_t* invoke_args = NULL;
  // printf("arg_count: %zu\n", arg_count);
  if (arg_count > 0) {
    invoke_args = args + 3;
  }
  InternalObject args_obj = {arg_count - 1, invoke_args};
  func_ptr invoke_func = GetFunction(GetStringData(func));
  if (invoke_func != NULL) {
    // printf("invoke_func: %p\n", invoke_func);
    invoke_func(args_obj, return_value);
    return 0;
  }

  return InvokeCustomFunction(GetStringData(func), arg_count, return_value,
                              invoke_args);
}
int EQUAL(size_t result, size_t value) {
  TRACE_FUNCTION;
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
      // printf("result: %zu, value: %s\n", result,GetStringData(value));
      SetStringData(result, GetStringData(value));
      break;
    case 0x06:
      SetPtrData(result, GetPtrData(value));
      break;
    case 0x09:
      // CopyObjectData(result, GetObjectData(value));
      SetObjectData(result, GetObjectData(value));
      break;
    default:
      // printf("value type: %d\n", value_data->type[0]);
      EXIT_VM("EQUAL(size_t,size_t)", "Unsupported type.");
  }
  return 0;
}

int CrossMemoryEqual(struct Memory* result_memory, size_t result,
                     struct Memory* value_memory, size_t value) {
  TRACE_FUNCTION;
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
      // printf("result: %zu, value: %s\n", result,GetStringData(value));
      SetStringObjectData(
          result_memory->object_table + result,
          GetStringObjectData(value_memory->object_table + value));
      break;
    case 0x06:
      SetPtrObjectData(result_memory->object_table + result,
                       GetPtrObjectData(value_memory->object_table + value));
      break;
    case 0x09:
      // CopyObjectData(result, GetObjectData(value));
      SetObjectObjectData(
          result_memory->object_table + result,
          GetObjectObjectData(value_memory->object_table + value));
      break;
    default:
      // printf("value type: %d\n", value_data->type[0]);
      EXIT_VM("CrossMemoryEqual(struct Memory*,size_t,struct Memory*,size_t)",
              "Unsupported type.");
  }
  return 0;
}

size_t GOTO(size_t location) {
  TRACE_FUNCTION;
  return GetUint64tData(location);
}
int LOAD_CONST(size_t object, size_t const_object) {
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
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
  TRACE_FUNCTION;
  if (result >= object_table_size)
    EXIT_VM("CONST(size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("CONST(size_t,size_t)", "Out of object_table_size.");

  SetConstData(result, object_table + operand1);
  return 0;
}

int INVOKE_METHOD(size_t* args) {
  TRACE_FUNCTION;
  if (args == NULL) EXIT_VM("INVOKE_METHOD(size_t*)", "Invalid args.");
  size_t func = args[1];
  size_t arg_count = args[2];
  size_t return_value = args[3];
  size_t* invoke_args = NULL;
  if (arg_count > 0) {
    invoke_args = args + 4;
  }
  InternalObject args_obj = {arg_count - 1, invoke_args};

  printf("func: %s\n", GetStringData(func));

  func_ptr invoke_func = GetFunction(GetStringData(func));
  if (invoke_func != NULL) {
    invoke_func(args_obj, return_value);
    return 0;
  }

  return InvokeClassFunction(args[0], GetStringData(func), arg_count,
                             return_value, invoke_args);
}

int LOAD_MEMBER(size_t result, size_t class, size_t operand) {
  TRACE_FUNCTION;
  if (result >= object_table_size)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Out of object_table_size.");
  if (class >= object_table_size)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Out of object_table_size.");

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

  printf("object_data: %p\n", object_data);

  // uint8_t* original_type = object_table[result].type;
  // bool original_const_type = object_table[result].const_type;

  SetReferenceData(result, object_data);

  // if (object_data->type[0] == 0x09) {
  // object_table[result].type = original_type;
  // object_table[result].const_type = original_const_type;
  // }

  return 0;

  /*struct Object* class_name = GetOriginData(object_table + class);
  if (class_name != NULL && class_name->type[0] != 0x05)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Invalid class name.");
  const unsigned int class_hash = hash(class_name->data.string_data);
  struct ClassList* class_table = &class_table[class_hash];
  struct Class* class_decl = NULL;
  bool is_end = false;
  while (class_table != NULL && class_table->class.name != NULL && !is_end) {
    if (strcmp(class_table->class.name, class_name->data.string_data) == 0) {
      class_decl = &class_table->class;
      is_end = true;
    }
    class_table = class_table->next;
  }
  if (!is_end) EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Class not found.");

  if (operand >= class_decl->members_size)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Out of class members.");

  struct Object* object_data = class_decl->members + operand;
  object_data = GetOriginData(object_data);

  switch (object_data->type[0]) {
    case 0x01:
      SetByteData(result, object_data->data.byte_data);
      break;
    case 0x02:
      SetLongData(result, object_data->data.long_data);
      break;
    case 0x03:
      SetDoubleData(result, object_data->data.double_data);
      break;
    case 0x04:
      SetUint64tData(result, object_data->data.uint64t_data);
      break;
    case 0x05:
      SetStringData(result, object_data->data.string_data);
      break;
    case 0x06:
      SetPtrData(result, object_data->data.ptr_data);
      break;
    case 0x09:
      SetObjectData(result, object_data->data.object_data);
      break;
    default:
      EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Unsupported type.");
  }

  return 0;*/
}

int WIDE() {
  TRACE_FUNCTION;
  return 0;
}

void print(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("print(InternalObject,size_t)", "Invalid args.");
  // printf("%zu", (uint64_t)GetUint64tData(*args.index));
  struct Object* object = object_table + *args.index;
  object = GetOriginData(object);

  switch (object->type[0]) {
    case 0x01:
      SetLongData(return_value, printf("%d", GetByteData(*args.index)));
      break;
    case 0x02:
      // printf("print long");
      SetLongData(return_value, printf("%lld", GetLongData(*args.index)));
      break;
    case 0x03:
      SetLongData(return_value, printf("%.15f", GetDoubleData(*args.index)));
      break;
    case 0x04:
      SetLongData(return_value, printf("%zu", GetUint64tData(*args.index)));
      break;
    case 0x05:
      SetLongData(return_value, printf("%s", GetStringData(*args.index)));
      break;
    case 0x06:
      SetLongData(return_value, printf("%p", GetPtrData(*args.index)));
      break;
    default:
      // printf("object type: %d\n", object->type[0]);
      EXIT_VM("print(InternalObject,size_t)", "Unsupported type.");
      break;
  }
}

void* AddClassMethod(void* location, struct FuncList* methods) {
  TRACE_FUNCTION;
  // void* original_location = location;
  // printf("point 1\n");

  if (*(char*)location == '.') location = (void*)((uintptr_t)location + 1);

  struct FuncList* table = &methods[hash(location)];
  if (table == NULL)
    EXIT_VM("AddClassMethod(void*,struct FuncList*)", "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.second.location = location;
  table->pair.first = location;
  table->pair.second.name = location;
  printf("ADD CLASS METHOD Name: %s\n", table->pair.second.name);
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  if (*(uint8_t*)location == 0xFF) {
    location = (void*)((uintptr_t)location + 1);
    table->pair.second.va_flag = true;
  } else {
    table->pair.second.va_flag = false;
  }

  location = (void*)((uintptr_t)location +
                     DecodeUleb128(location, &table->pair.second.args_size));
  if (table->pair.second.va_flag) {
    // printf("TEST 2");
    table->pair.second.args_size--;
    table->pair.second.args =
        (size_t*)calloc(table->pair.second.args_size + 1, sizeof(size_t));
    // printf("args_size: %zu", table->pair.second.args_size);
    for (size_t i = 0; i < table->pair.second.args_size + 1; i++) {
      location = (void*)((uintptr_t)location +
                         DecodeUleb128(location, &table->pair.second.args[i]));
    }
  } else {
    table->pair.second.args =
        (size_t*)calloc(table->pair.second.args_size, sizeof(size_t));
    // printf("args_size: %zu", table->pair.second.args_size);
    for (size_t i = 0; i < table->pair.second.args_size; i++) {
      location = (void*)((uintptr_t)location +
                         DecodeUleb128(location, &table->pair.second.args[i]));
    }
  }

  // printf("\n\n\n%s",table->pair.second.name);

  /*printf("%02x,%02x, %02x, %02x\n", *(uint8_t*)location,
         *(uint8_t*)((uintptr_t)location + 1),
         *(uint8_t*)((uintptr_t)location + 2),
         *(uint8_t*)((uintptr_t)location + 3));
         printf("%02x,%02x, %02x, %02x\n", *(uint8_t*)((uintptr_t)location + 4),
         *(uint8_t*)((uintptr_t)location +5),
         *(uint8_t*)((uintptr_t)location +6),
         *(uint8_t*)((uintptr_t)location +7));*/

  table->pair.second.commands_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  struct Bytecode* bytecode = (struct Bytecode*)calloc(
      table->pair.second.commands_size, sizeof(struct Bytecode));
  // printf("commands_size: %zu", table->pair.second.commands_size);
  if (bytecode == NULL)
    EXIT_VM("AddClassMethod(void*,struct FuncList*)", "calloc failed.");
  AddFreePtr(bytecode);

  table->pair.second.commands = bytecode;

  for (size_t i = 0; i < table->pair.second.commands_size; i++) {
    bytecode[i].operator= *(uint8_t*) location;
    location = (void*)((uintptr_t)location + 1);
    switch (bytecode[i].operator) {
      case OPERATOR_NOP:
        bytecode[i].args = NULL;
        break;

      case OPERATOR_LOAD:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_STORE:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_NEW:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_ARRAY:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_PTR:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_ADD:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_SUB:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_MUL:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_DIV:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_REM:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_NEG:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_SHL:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_SHR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_REFER:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_AND:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_OR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_XOR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_IF:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_CMP:
        bytecode[i].args = (size_t*)malloc(4 * sizeof(size_t));
        location =
            Get4Parament(location, bytecode[i].args, bytecode[i].args + 1,
                         bytecode[i].args + 2, bytecode[i].args + 3);
        break;

      case OPERATOR_GOTO:
        bytecode[i].args = (size_t*)malloc(sizeof(size_t));
        location = Get1Parament(location, bytecode[i].args);
        break;

      case OPERATOR_INVOKE:
        bytecode[i].args = GetUnknownCountParament(&location);
        break;

      case OPERATOR_EQUAL:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_LOAD_CONST:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_CONVERT:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_CONST:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_INVOKE_METHOD:
        bytecode[i].args = GetUnknownCountParamentForClass(&location);
        break;

      case OPERATOR_LOAD_MEMBER:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_WIDE:
        bytecode[i].args = NULL;
        break;

      default:
        EXIT_VM("AddClassMethod(void*,struct FuncList*)", "Invalid operator.");
    }
    AddFreePtr(bytecode[i].args);
  }

  table->next = (struct FuncList*)calloc(1, sizeof(struct FuncList));
  AddFreePtr(table->next);

  return location;
}

void* AddClassMethodFromOutside(void* location, struct FuncList* methods) {
  TRACE_FUNCTION;
  // void* original_location = location;
  // printf("point 1\n");

  if (*(char*)location == '.') location = (void*)((uintptr_t)location + 1);
  // location = (void*)((uintptr_t)location + 1);

  struct FuncList* table = &methods[hash(location)];
  if (table == NULL)
    EXIT_VM("AddClassMethod(void*,struct FuncList*)", "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }

  table->pair.second.location = location;
  table->pair.first = location;
  table->pair.second.name = location;
  // printf("Name: %s\n", table->pair.second.name);
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  if (*(uint8_t*)location == 0xFF) {
    location = (void*)((uintptr_t)location + 1);
    table->pair.second.va_flag = true;
  } else {
    table->pair.second.va_flag = false;
  }

  location = (void*)((uintptr_t)location +
                     DecodeUleb128(location, &table->pair.second.args_size));
  if (table->pair.second.va_flag) {
    // printf("TEST 2");
    table->pair.second.args_size--;
    table->pair.second.args =
        (size_t*)calloc(table->pair.second.args_size + 1, sizeof(size_t));
    // printf("args_size: %zu", table->pair.second.args_size);
    for (size_t i = 0; i < table->pair.second.args_size + 1; i++) {
      location = (void*)((uintptr_t)location +
                         DecodeUleb128(location, &table->pair.second.args[i]));
    }
  } else {
    table->pair.second.args =
        (size_t*)calloc(table->pair.second.args_size, sizeof(size_t));
    // printf("args_size: %zu", table->pair.second.args_size);
    for (size_t i = 0; i < table->pair.second.args_size; i++) {
      location = (void*)((uintptr_t)location +
                         DecodeUleb128(location, &table->pair.second.args[i]));
    }
  }

  // printf("\n\n\n%s",table->pair.second.name);

  /*printf("%02x,%02x, %02x, %02x\n", *(uint8_t*)location,
         *(uint8_t*)((uintptr_t)location + 1),
         *(uint8_t*)((uintptr_t)location + 2),
         *(uint8_t*)((uintptr_t)location + 3));
         printf("%02x,%02x, %02x, %02x\n", *(uint8_t*)((uintptr_t)location + 4),
         *(uint8_t*)((uintptr_t)location +5),
         *(uint8_t*)((uintptr_t)location +6),
         *(uint8_t*)((uintptr_t)location +7));*/

  table->pair.second.commands_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  struct Bytecode* bytecode = (struct Bytecode*)calloc(
      table->pair.second.commands_size, sizeof(struct Bytecode));
  // printf("commands_size: %zu", table->pair.second.commands_size);
  if (bytecode == NULL)
    EXIT_VM("AddClassMethod(void*,struct FuncList*)", "calloc failed.");
  AddFreePtr(bytecode);

  table->pair.second.commands = bytecode;

  for (size_t i = 0; i < table->pair.second.commands_size; i++) {
    bytecode[i].operator= *(uint8_t*) location;
    location = (void*)((uintptr_t)location + 1);
    switch (bytecode[i].operator) {
      case OPERATOR_NOP:
        bytecode[i].args = NULL;
        break;

      case OPERATOR_LOAD:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_STORE:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_NEW:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_ARRAY:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_PTR:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_ADD:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_SUB:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_MUL:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_DIV:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_REM:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_NEG:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_SHL:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_SHR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_REFER:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_AND:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_OR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_XOR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_IF:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_CMP:
        bytecode[i].args = (size_t*)malloc(4 * sizeof(size_t));
        location =
            Get4Parament(location, bytecode[i].args, bytecode[i].args + 1,
                         bytecode[i].args + 2, bytecode[i].args + 3);
        break;

      case OPERATOR_GOTO:
        bytecode[i].args = (size_t*)malloc(sizeof(size_t));
        location = Get1Parament(location, bytecode[i].args);
        break;

      case OPERATOR_INVOKE:
        bytecode[i].args = GetUnknownCountParament(&location);
        break;

      case OPERATOR_EQUAL:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_LOAD_CONST:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_CONVERT:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_CONST:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_INVOKE_METHOD:
        bytecode[i].args = GetUnknownCountParamentForClass(&location);
        break;

      case OPERATOR_LOAD_MEMBER:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_WIDE:
        bytecode[i].args = NULL;
        break;

      default:
        EXIT_VM("AddClassMethod(void*,struct FuncList*)", "Invalid operator.");
    }
    AddFreePtr(bytecode[i].args);
  }

  table->next = (struct FuncList*)calloc(1, sizeof(struct FuncList));
  AddFreePtr(table->next);

  return location;
}

void* AddFunction(void* location);
void* AddClass(void* location) {
  TRACE_FUNCTION;

  // printf("AddClass START.\n");

  struct ClassList* table = &class_table[hash(location)];
  if (table == NULL) EXIT_VM("AddClass(void*)", "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }
  table->class.name = location;
  // printf("Class Name: %s\n", table->class.name);
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  size_t object_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);
  table->class.members_size = object_size;
  table->class.members =
      (struct Object*)calloc(object_size, sizeof(struct Object));
  if (table->class.members == NULL)
    EXIT_VM("AddClass(void*)", "calloc failed.");
  AddFreePtr(table->class.members);

  for (size_t i = 0; i < object_size; i++) {
    struct ClassVarInfoList* var_info =
        &table->class.var_info_table[hash(location)];
    if (var_info == NULL) EXIT_VM("AddClass(void*)", "var info table is NULL.");
    while (var_info->next != NULL) {
      var_info = var_info->next;
    }
    var_info->name = location;
    // printf("MEMBER: %s\n", var_info->name);
    var_info->index = i;
    var_info->next =
        (struct ClassVarInfoList*)calloc(1, sizeof(struct ClassVarInfoList));
    AddFreePtr(var_info->next);
    while (*(char*)location != '\0') {
      location = (void*)((uintptr_t)location + 1);
    }
    location = (void*)((uintptr_t)location + 1);

    table->class.members[i].type = location;
    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)location) {
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
          location = (void*)((uintptr_t)location + 1);
          break;

        default:
          // printf("%d\n", *(uint8_t*)location);
          // printf("%d\n", *(uint8_t*)((uintptr_t)location + 1));
          // printf("%d\n", *(uint8_t*)((uintptr_t)location + 2));
          // printf("%d\n", *(uint8_t*)((uintptr_t)location + 3));
          EXIT_VM("AddClass(void*)", "Unsupported type.");
          break;
      }
    }
    if (table->class.members[i].type[0] != 0x00)
      table->class.members[i].const_type = true;
    location = (void*)((uintptr_t)location + 1);
  }

  size_t method_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  if (strcmp(table->class.name, ".!__start") == 0) {
    for (size_t i = 0; i < method_size; i++) {
      // location = AddFunction(location);
      location = AddClassMethod(location, table->class.methods);
    }
  } else {
    for (size_t i = 0; i < method_size; i++) {
      location = AddClassMethod(location, table->class.methods);
    }
  }

  table->class.memory = global_memory;

  table->next = (struct ClassList*)calloc(1, sizeof(struct ClassList));
  AddFreePtr(table->next);

  return location;
}

void* AddFunction(void* location) {
  TRACE_FUNCTION;
  // void* original_location = location;
  // printf("point 1\n");

  struct FuncList* table = &func_table[hash(location)];
  if (table == NULL) EXIT_VM("AddFunction(void*)", "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.second.location = location;
  table->pair.first = location;
  table->pair.second.name = location;

  // printf("Name: %s\n", table->pair.second.name);

  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  if (*(uint8_t*)location == 0xFF) {
    location = (void*)((uintptr_t)location + 1);
    table->pair.second.va_flag = true;
  } else {
    table->pair.second.va_flag = false;
  }

  location = (void*)((uintptr_t)location +
                     DecodeUleb128(location, &table->pair.second.args_size));
  if (table->pair.second.va_flag) {
    // printf("TEST 1");
    table->pair.second.args_size--;
    table->pair.second.args =
        (size_t*)calloc(table->pair.second.args_size + 1, sizeof(size_t));
    // printf("args_size: %zu", table->pair.second.args_size);
    for (size_t i = 0; i < table->pair.second.args_size + 1; i++) {
      location = (void*)((uintptr_t)location +
                         DecodeUleb128(location, &table->pair.second.args[i]));
    }
  } else {
    // printf("TEST WITHOUT VA");
    table->pair.second.args =
        (size_t*)calloc(table->pair.second.args_size, sizeof(size_t));
    // printf("args_size: %zu", table->pair.second.args_size);
    for (size_t i = 0; i < table->pair.second.args_size; i++) {
      location = (void*)((uintptr_t)location +
                         DecodeUleb128(location, &table->pair.second.args[i]));
    }
  }

  table->pair.second.commands_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  struct Bytecode* bytecode = (struct Bytecode*)calloc(
      table->pair.second.commands_size, sizeof(struct Bytecode));
  // printf("commands_size: %zu", table->pair.second.commands_size);
  // printf("%zu\n", table->pair.second.commands_size);
  if (bytecode == NULL) EXIT_VM("AddFunction(void*)", "calloc failed.");
  AddFreePtr(bytecode);

  table->pair.second.commands = bytecode;

  for (size_t i = 0; i < table->pair.second.commands_size; i++) {
    bytecode[i].operator= *(uint8_t*) location;
    location = (void*)((uintptr_t)location + 1);
    switch (bytecode[i].operator) {
      case OPERATOR_NOP:
        bytecode[i].args = NULL;
        break;

      case OPERATOR_LOAD:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_STORE:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_NEW:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_ARRAY:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_PTR:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_ADD:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_SUB:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_MUL:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_DIV:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_REM:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_NEG:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_SHL:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_SHR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_REFER:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_AND:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_OR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_XOR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_IF:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_CMP:
        bytecode[i].args = (size_t*)malloc(4 * sizeof(size_t));
        location =
            Get4Parament(location, bytecode[i].args, bytecode[i].args + 1,
                         bytecode[i].args + 2, bytecode[i].args + 3);
        break;

      case OPERATOR_GOTO:
        bytecode[i].args = (size_t*)malloc(sizeof(size_t));
        location = Get1Parament(location, bytecode[i].args);
        break;

      case OPERATOR_INVOKE:
        bytecode[i].args = GetUnknownCountParament(&location);
        break;

      case OPERATOR_EQUAL:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_LOAD_CONST:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_CONVERT:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_CONST:
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
        break;

      case OPERATOR_INVOKE_METHOD:
        bytecode[i].args = GetUnknownCountParamentForClass(&location);
        break;

      case OPERATOR_LOAD_MEMBER:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
        break;

      case OPERATOR_WIDE:
        bytecode[i].args = NULL;
        break;

      default:
        EXIT_VM("AddFunction(void*)", "Invalid operator.");
    }
    AddFreePtr(bytecode[i].args);
  }

  table->next = (struct FuncList*)calloc(1, sizeof(struct FuncList));
  AddFreePtr(table->next);

  return location;
}

FuncInfo GetClassFunction(const char* class, const char* name, size_t* args,
                          size_t args_size) {
  TRACE_FUNCTION;
  if (class == NULL)
    EXIT_VM("GetClassFunction(const char*,const char*,size_t*,size_t)",
            "Invalid class name.");
  if (name == NULL)
    EXIT_VM("GetClassFunction(const char*,const char*,size_t*,size_t)",
            "Invalid func name.");
  // printf("Class: %s, Name: %s\n", class, name);
  if (*name == '.') name++;
  const unsigned int class_hash = hash(class);
  const struct ClassList* current_class_table = &class_table[class_hash];
  FuncInfo temp_func;
  int cost = -1;
  while (current_class_table != NULL &&
         current_class_table->class.name != NULL) {
    if (strcmp(current_class_table->class.name, class) == 0) {
      const unsigned int name_hash = hash(name);
      const struct FuncList* table =
          &current_class_table->class.methods[name_hash];
      while (table != NULL && table->pair.first != NULL) {
        if (table->pair.first == NULL)
          EXIT_VM("GetClassFunction(const char*,const char*,size_t*,size_t)",
                  "Invalid name.");
        if (strcmp(table->pair.first, name) == 0) {
          // printf("FOUND BUT NOT MATCH.\n");
          if (table->pair.second.args_size <= args_size) {
            // bool is_same = true;
            // for (size_t i = 0; i < args_size - 1; i++) {
            /*if (args == NULL)
              EXIT_VM(
                  "GetClassFunction(const char*,const char*,size_t*,size_t)",
                  "Invalid args.");*/
            int temp_cost = GetFuncOverloadCost(
                args, args_size, table->pair.second.args,
                table->pair.second.args_size, table->pair.second.va_flag);
            if (temp_cost == -1) {
            } else if (temp_cost == 0) {
              // printf("Check 0.\n");
              return table->pair.second;
            } else if (temp_cost < cost || cost < 0) {
              temp_func = table->pair.second;
              cost = temp_cost;
            }
          }
          // if (is_same) return table->pair.second;
          //}
          // return table->pair.second;
        }
        table = table->next;
      }
    } else {
      current_class_table = current_class_table->next;
    }
    printf("Class Func Name: %s\n", name);
    if (cost == -1)
      EXIT_VM("GetClassFunction(const char*,const char*,size_t*,size_t)",
              "Not found func.");
    return temp_func;
  }

  /*const struct FuncList* table = &func_table[name_hash];
  while (table != NULL && table->pair.first != NULL) {
    if (table->pair.first == NULL)
      EXIT_VM("GetClassFunction(const char*,const char*,size_t*,size_t)",
              "Invalid name.");
    if (strcmp(table->pair.first, name) == 0) {
      if (table->pair.second.args_size == args_size) {
        bool is_same = true;
        for (size_t i = 0; i < args_size - 1; i++) {
          if (args == NULL)
            EXIT_VM("GetClassFunction(const char*,const char*,size_t*,size_t)",
                    "Invalid args.");
          if (object_table[table->pair.second.args[i]].const_type &&
              object_table[table->pair.second.args[i]].type[0] !=
                  object_table[args[i]].type[0]) {
            is_same = false;
            break;
          }
        }
        if (is_same) return table->pair.second;
      }
      // return table->pair.second;
    }
    table = table->next;
  }*/

  EXIT_VM("GetClassFunction(const char*,const char*,size_t*,size_t)",
          "Function not found.");
  return (FuncInfo){NULL, NULL, 0, NULL};
}

FuncInfo GetCustomFunction(const char* name, size_t* args, size_t args_size) {
  TRACE_FUNCTION;
  if (name == NULL)
    EXIT_VM("GetCustomFunction(const char*,size_t*,size_t)", "Invalid name.");
  // printf("name: %s\n", name);
  const unsigned int name_hash = hash(name);
  const struct FuncList* table = &func_table[name_hash];
  FuncInfo temp_func;
  int cost = -1;
  while (table != NULL && table->pair.first != NULL) {
    if (table->pair.first == NULL)
      EXIT_VM("GetCustomFunction(const char*,size_t*,size_t)", "Invalid name.");
    if (strcmp(table->pair.first, name) == 0) {
      if (table->pair.second.args_size <= args_size) {
        // bool is_same = true;
        // for (size_t i = 0; i < args_size - 1; i++) {
        /*if (args == NULL)
          EXIT_VM("GetCustomFunction(const char*,size_t*,size_t)",
                  "Invalid args.");*/
        // printf("Type compare:
        // %i,%i",object_table[table->pair.second.args[i+1]].type[0],object_table[args[i]].type[0]);
        int temp_cost = GetFuncOverloadCost(
            args, args_size, table->pair.second.args,
            table->pair.second.args_size, table->pair.second.va_flag);
        if (temp_cost == -1) {
        } else if (temp_cost == 0) {
          // printf("Check 0.\n");
          // temp_func = table->pair.second;
          // cost = temp_cost;
          return table->pair.second;
        } else if (temp_cost < cost || cost < 0) {
          temp_func = table->pair.second;
          cost = temp_cost;
        }
        //}
      }
      // return table->pair.second;
    }
    table = table->next;
  }
  // printf("%s",name);
  if (cost == -1)
    EXIT_VM("GetCustomFunction(const char*,const char*,size_t*,size_t)",
            "Not found func.");
  return temp_func;
}

func_ptr GetFunction(const char* name) {
  TRACE_FUNCTION;
  if (name == NULL) EXIT_VM("GetFunction(const char*)", "Invalid name.");
  const unsigned int name_hash = hash(name);
  const struct LinkedList* table = &name_table[name_hash];
  while (table->pair.first != NULL) {
    if (strcmp(table->pair.first, name) == 0) {
      return table->pair.second;
    }
    table = table->next;
  }
  return (func_ptr)NULL;
}

int EQUAL(size_t result, size_t value);
size_t GOTO(size_t location);
int LOAD_CONST(size_t object, size_t const_object);
int WIDE();

int InvokeClassFunction(size_t class, const char* name, size_t args_size,
                        size_t return_value, size_t* args) {
  TRACE_FUNCTION;
  struct Object* class_name_object = GetOriginData(object_table + class);
  // struct Object* class_object = GetOriginData(object_table + class);
  class_name_object = class_name_object->data.object_data;
  class_name_object = GetOriginData(class_name_object);
  if (class_name_object == NULL)
    EXIT_VM("InvokeClassFunction(size_t,const char*,size_t,size_t,size_t*)",
            "Invalid class name object.");
  const char* class_name = class_name_object->data.string_data;

  struct Memory* class_memory = NULL;
  struct ClassList* current_class_table = &class_table[hash(class_name)];
  while (current_class_table != NULL &&
         current_class_table->class.name != NULL) {
    if (strcmp(current_class_table->class.name, class_name) == 0) {
      class_memory = current_class_table->class.memory;
      break;
    }
    current_class_table = current_class_table->next;
  }

  if (class_memory == NULL) {
    EXIT_VM("InvokeClassFunction(size_t,const char*,size_t,size_t,size_t*)",
            "Class memory not found.");
  }

  struct Memory origin_memory = {object_table, object_table_size,
                                 const_object_table, const_object_table_size};

  object_table = class_memory->object_table;
  object_table_size = class_memory->object_table_size;
  const_object_table = class_memory->const_object_table;
  const_object_table_size = class_memory->const_object_table_size;

  FuncInfo func_info = GetClassFunction(class_name, name, args, args_size);
  object_table = origin_memory.object_table;
  object_table_size = origin_memory.object_table_size;
  const_object_table = origin_memory.const_object_table;
  const_object_table_size = origin_memory.const_object_table_size;

  if (args_size < func_info.args_size) {
    // printf("args_size: %zu\n", args_size);
    // printf("func_info.args_size: %zu\n", func_info.args_size);
    EXIT_VM("InvokeClassFunction(size_t,const char*,size_t,size_t,size_t*)",
            "Invalid args_size.");
  }
  // printf("object: %zu , %zu", func_info.args[0], return_value);
  // TODO(Class): Fixed this bug about return value.
  // object_table[func_info.args[0]] = object_table[return_value];

  if (func_info.va_flag) {
    uint8_t* type = calloc(1, sizeof(uintptr_t));
    type[0] = 0x04;
    object_table[return_value].type = type;
    object_table[return_value].data.uint64t_data =
        args_size - func_info.args_size;

    CrossMemoryNew(class_memory, func_info.args[func_info.args_size],
                   return_value, 0x00);

    // printf("RUN OK!\n");
    for (size_t i = 0; i < args_size - func_info.args_size; i++) {
      class_memory->object_table[func_info.args[func_info.args_size]]
          .data.ptr_data[i + 1]
          .type = object_table[args[func_info.args_size + i - 1]].type;
      class_memory->object_table[func_info.args[func_info.args_size]]
          .data.ptr_data[i + 1]
          .data = object_table[args[func_info.args_size + i - 1]].data;
    }
  }

  func_info.args++;
  args_size--;
  for (size_t i = 0; i < func_info.args_size - 1; i++) {
    if (class_memory->object_table[func_info.args[i]].const_type &&
        class_memory->object_table[func_info.args[i]].type[0] == 0x07 &&
        class_memory->object_table[func_info.args[i]].type[1] != 0x08) {
      class_memory->object_table[func_info.args[i]].data.reference_data =
          object_table + args[i];
    } else if (class_memory->object_table[func_info.args[i]].const_type &&
               class_memory->object_table[func_info.args[i]].type[0] == 0x07 &&
               class_memory->object_table[func_info.args[i]].type[1] == 0x08) {
      class_memory->object_table[func_info.args[i]].type =
          class_memory->object_table[func_info.args[i]].type + 1;
      class_memory->object_table[func_info.args[i]].data.const_data =
          object_table + args[i];
    } else if (class_memory->object_table[func_info.args[i]].const_type &&
               class_memory->object_table[func_info.args[i]].type[0] == 0x08) {
      class_memory->object_table[func_info.args[i]].data.const_data =
          object_table + args[i];
    } else {
      CrossMemoryEqual(class_memory, func_info.args[i], &origin_memory,
                       args[i]);
    }
  }

  object_table = class_memory->object_table;
  object_table_size = class_memory->object_table_size;
  const_object_table = class_memory->const_object_table;
  const_object_table_size = class_memory->const_object_table_size;

  struct Bytecode* run_code = func_info.commands;
  for (size_t i = 0; i < func_info.commands_size; i++) {
    // printf("operator: %d\n", run_code[i].operator);
    switch (run_code[i].operator) {
      case 0x00:
        NOP();
        break;
      case 0x01:
        LOAD(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x02:
        STORE(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x03:
        NEW(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x04:
        ARRAY(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x05:
        PTR(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x06:
        ADD(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x07:
        SUB(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x08:
        MUL(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x09:
        DIV(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0A:
        REM(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0B:
        NEG(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x0C:
        SHL(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0D:
        SHR(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0E:
        REFER(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x0F:
        i = IF(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        i--;
        break;
      case 0x10:
        AND(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x11:
        OR(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x12:
        XOR(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x13:
        CMP(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2],
            run_code[i].args[3]);
        break;
      case 0x14:
        INVOKE(run_code[i].args);
        break;
      case 0x15:
        EQUAL(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x16:
        i = GOTO(run_code[i].args[0]);
        i--;
        break;
      case 0x17:
        LOAD_CONST(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x18:
        CONVERT(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x19:
        _CONST(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x1A:
        INVOKE_METHOD(run_code[i].args);
        break;
      case 0x1B:
        if (run_code[i].args[1] == 0) {
          LOAD_MEMBER(run_code[i].args[0], class, run_code[i].args[2]);
        } else {
          LOAD_MEMBER(run_code[i].args[0], run_code[i].args[1],
                      run_code[i].args[2]);
        }
        break;
      case 0xFF:
        WIDE();
        break;
      default:
        EXIT_VM("InvokeClassFunction(size_t,const char*,size_t,size_t,size_t*)",
                "Invalid operator.");
        break;
    }
  }

  object_table = origin_memory.object_table;
  object_table_size = origin_memory.object_table_size;
  const_object_table = origin_memory.const_object_table;
  const_object_table_size = origin_memory.const_object_table_size;

  return 0;
}

int InvokeCustomFunction(const char* name, size_t args_size,
                         size_t return_value, size_t* args) {
  TRACE_FUNCTION;
  FuncInfo func_info = GetCustomFunction(name, args, args_size);
  if (args_size < func_info.args_size) {
    // printf("args_size: %zu\n", args_size);
    // printf("func_info.args_size: %zu\n", func_info.args_size);
    EXIT_VM("InvokeCustomFunction(const char*,size_t,size_t,size_t*)",
            "Invalid args_size.");
  }
  // printf("object: %zu , %zu", func_info.args[0], return_value);

  if (func_info.va_flag) {
    uint8_t* type = calloc(1, sizeof(uintptr_t));
    type[0] = 0x04;
    object_table[return_value].type = type;
    object_table[return_value].data.uint64t_data =
        args_size - func_info.args_size;

    NEW(func_info.args[func_info.args_size], return_value, 0x00);

    // printf("Type: %i\n",
    // object_table[func_info.args[func_info.args_size]].type[0]);

    if (object_table[func_info.args[func_info.args_size]].type == NULL ||
        object_table[func_info.args[func_info.args_size]].type[0] != 0x06)
      EXIT_VM("InvokeCustomFunction(const char*,size_t,size_t,size_t*)",
              "Invalid va_arg array.");

    // printf("RUN OK!\n");
    for (size_t i = 0; i < args_size - func_info.args_size; i++) {
      object_table[func_info.args[func_info.args_size]]
          .data.ptr_data[i + 1]
          .type = object_table[args[func_info.args_size - 1 + i]].type;

      object_table[func_info.args[func_info.args_size]]
          .data.ptr_data[i + 1]
          .data = object_table[args[func_info.args_size - 1 + i]].data;
    }
  }

  object_table[func_info.args[0]] = object_table[return_value];
  struct Object* return_object = object_table + func_info.args[0];
  func_info.args++;
  args_size--;
  for (size_t i = 0; i < func_info.args_size - 1; i++) {
    if (object_table[func_info.args[i]].const_type &&
        object_table[func_info.args[i]].type[0] == 0x07 &&
        object_table[func_info.args[i]].type[1] != 0x08) {
      object_table[func_info.args[i]].data.reference_data =
          object_table + args[i];
    } else if (object_table[func_info.args[i]].const_type &&
               object_table[func_info.args[i]].type[0] == 0x07 &&
               object_table[func_info.args[i]].type[1] == 0x08) {
      object_table[func_info.args[i]].type =
          object_table[func_info.args[i]].type + 1;
      object_table[func_info.args[i]].data.const_data = object_table + args[i];
    } else if (object_table[func_info.args[i]].const_type &&
               object_table[func_info.args[i]].type[0] == 0x08) {
      object_table[func_info.args[i]].data.const_data = object_table + args[i];
    } else {
      EQUAL(func_info.args[i], args[i]);
    }
  }
  struct Bytecode* run_code = func_info.commands;
  for (size_t i = 0; i < func_info.commands_size; i++) {
    // printf("operator: %d\n", run_code[i].operator);
    switch (run_code[i].operator) {
      case 0x00:
        NOP();
        break;
      case 0x01:
        LOAD(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x02:
        STORE(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x03:
        NEW(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x04:
        ARRAY(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x05:
        PTR(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x06:
        ADD(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x07:
        SUB(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x08:
        MUL(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x09:
        DIV(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0A:
        REM(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0B:
        NEG(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x0C:
        SHL(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0D:
        SHR(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x0E:
        REFER(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x0F:
        i = IF(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        i--;
        break;
      case 0x10:
        AND(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x11:
        OR(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x12:
        XOR(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
        break;
      case 0x13:
        CMP(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2],
            run_code[i].args[3]);
        break;
      case 0x14:
        INVOKE(run_code[i].args);
        break;
      case 0x15:
        EQUAL(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x16:
        i = GOTO(run_code[i].args[0]);
        i--;
        break;
      case 0x17:
        LOAD_CONST(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x18:
        CONVERT(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x19:
        _CONST(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x1A:
        INVOKE_METHOD(run_code[i].args);
        break;
      case 0x1B:
        LOAD_MEMBER(run_code[i].args[0], run_code[i].args[1],
                    run_code[i].args[2]);
        break;
      case 0xFF:
        WIDE();
        break;
      default:
        EXIT_VM("InvokeCustomFunction(const char*,size_t,size_t,size_t*)",
                "Invalid operator.");
        break;
    }
  }

  return 0;
}

void* AddBytecodeFileClass(const char* name, struct Memory* memory,
                           void* location) {
  TRACE_FUNCTION;

  // printf("ABFC. location: %s\n",location);

  char* class_name =
      calloc(strlen(name) + strlen((char*)location) + 1, sizeof(char));
  AddFreePtr(class_name);
  snprintf(class_name, strlen(name) + strlen((char*)location) + 1, "%s%s", name,
           (char*)location);

  printf("ABFC: %s\n", class_name);

  struct ClassList* table = &class_table[hash(class_name)];
  if (table == NULL)
    EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
            "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }
  table->class.name = class_name;
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  size_t object_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);
  table->class.members_size = object_size;
  table->class.members =
      (struct Object*)calloc(object_size, sizeof(struct Object));
  if (table->class.members == NULL)
    EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
            "calloc failed.");
  AddFreePtr(table->class.members);

  for (size_t i = 0; i < object_size; i++) {
    struct ClassVarInfoList* var_info =
        &table->class.var_info_table[hash(location)];
    if (var_info == NULL)
      EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
              "var info table is NULL.");
    while (var_info->next != NULL) {
      var_info = var_info->next;
    }
    var_info->name = location;
    var_info->index = i;
    var_info->next =
        (struct ClassVarInfoList*)calloc(1, sizeof(struct ClassVarInfoList));
    AddFreePtr(var_info->next);
    while (*(char*)location != '\0') {
      location = (void*)((uintptr_t)location + 1);
    }
    location = (void*)((uintptr_t)location + 1);

    table->class.members[i].type = location;
    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)location) {
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
          location = (void*)((uintptr_t)location + 1);
          break;

        default:
          EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
                  "Unsupported type.");
          break;
      }
    }
    if (table->class.members[i].type[0] != 0x00)
      table->class.members[i].const_type = true;
    location = (void*)((uintptr_t)location + 1);
  }

  size_t method_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  // printf("Method size: %zu\n", method_size);
  for (size_t i = 0; i < method_size; i++) {
    // printf("NOW: %zu,%zu\n", i, method_size);
    location = AddClassMethodFromOutside(location, table->class.methods);
  }

  table->next = (struct ClassList*)calloc(1, sizeof(struct ClassList));
  AddFreePtr(table->next);

  table->class.memory = memory;

  return location;
}

void HandleBytecodeFile(const char* name, void* bytecode_file, size_t size) {
  TRACE_FUNCTION;
  if (((char*)bytecode_file)[0] != 0x41 || ((char*)bytecode_file)[1] != 0x51 ||
      ((char*)bytecode_file)[2] != 0x42 || ((char*)bytecode_file)[3] != 0x43) {
    EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
            "Invalid bytecode file.");
  }

  if (((char*)bytecode_file)[4] != 0x00 || ((char*)bytecode_file)[5] != 0x00 ||
      ((char*)bytecode_file)[6] != 0x00 || ((char*)bytecode_file)[7] != 0x03) {
    EXIT_VM(
        "HandleBytecodeFile(const char*,const char*,size_t)",
        "This bytecode version is not supported, please check for updates.");
  }

  void* bytecode_end = (void*)((uintptr_t)bytecode_file + size);

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  /*size_t import_bytecode_file_size =
      is_big_endian ? *(uint64_t*)bytecode_file
                    : SwapUint64t(*(uint64_t*)bytecode_file);
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  void* import_bytecode_file = bytecode_file;

  for (size_t i = 0; i < import_bytecode_file_size; i++) {
    while (*(char*)bytecode_file != '\0') {
      bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    }
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    while (*(char*)bytecode_file != '\0') {
      bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    }
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
  }*/

  struct Memory* memory = calloc(1, sizeof(struct Memory));

  // bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  /*size_t bytecode_file_size = is_big_endian
                                  ? *(uint64_t*)bytecode_file
                                  : SwapUint64t(*(uint64_t*)bytecode_file);
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
  bytecode_file = AddBytecodeFile(bytecode_file, bytecode_file_size);*/

  memory->const_object_table_size =
      is_big_endian ? *(uint64_t*)bytecode_file
                    : SwapUint64t(*(uint64_t*)bytecode_file);

  memory->const_object_table = (struct Object*)malloc(
      memory->const_object_table_size * sizeof(struct Object));

  if (memory->const_object_table == NULL)
    EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
            "const_object_table malloc failed.");

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  for (size_t i = 0; i < memory->const_object_table_size; i++) {
    memory->const_object_table[i].type = bytecode_file;
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    switch (memory->const_object_table[i].type[0]) {
      case 0x01:
        memory->const_object_table[i].data.byte_data = *(int8_t*)bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        break;

      case 0x02:
        memory->const_object_table[i].data.long_data = *(int64_t*)bytecode_file;
        memory->const_object_table[i].data.long_data =
            is_big_endian
                ? memory->const_object_table[i].data.long_data
                : SwapLong(memory->const_object_table[i].data.long_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x03:
        memory->const_object_table[i].data.double_data =
            *(double*)bytecode_file;
        memory->const_object_table[i].data.double_data =
            is_big_endian
                ? const_object_table[i].data.double_data
                : SwapDouble(memory->const_object_table[i].data.double_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x04:
        memory->const_object_table[i].data.uint64t_data =
            *(uint64_t*)bytecode_file;
        memory->const_object_table[i].data.uint64t_data =
            is_big_endian
                ? const_object_table[i].data.uint64t_data
                : SwapUint64t(memory->const_object_table[i].data.uint64t_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x05: {
        size_t str_size = 0;
        bytecode_file = (void*)((uintptr_t)bytecode_file +
                                DecodeUleb128(bytecode_file, &str_size));
        memory->const_object_table[i].data.string_data = bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + str_size);
        break;
      }

      case 0x06:
        memory->const_object_table[i].data.ptr_data = *(void**)bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      default:
        EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
                "Unknown type.");
        break;
    }
  }

  memory->object_table_size = is_big_endian
                                  ? *(uint64_t*)bytecode_file
                                  : SwapUint64t(*(uint64_t*)bytecode_file);

  memory->object_table =
      (struct Object*)calloc(memory->object_table_size, sizeof(struct Object));

  if (memory->object_table == NULL)
    EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
            "object_table calloc failed.");

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  for (size_t i = 0; i < memory->object_table_size; i++) {
    memory->object_table[i].type = bytecode_file;
    // printf("object_table type: %zu\n", i);
    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)bytecode_file) {
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
          bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
          break;

        default:
          // printf("object_table type: %i\n", *(uint8_t*)bytecode_file);
          EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
                  "Unsupported type.");
          break;
      }
    }
    if (memory->object_table[i].type[0] != 0x00)
      memory->object_table[i].const_type = true;
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
  }

  while (bytecode_file < bytecode_end) {
    bytecode_file = AddBytecodeFileClass(name, memory, bytecode_file);

    // printf("%p , %p",bytecode_file,(void*)((uintptr_t)bytecode_file + size));
  }

  /*struct Memory origin_memory = {object_table, object_table_size,
                                 const_object_table, const_object_table_size};

  object_table = memory->object_table;
  object_table_size = memory->object_table_size;
  const_object_table = memory->const_object_table;
  const_object_table_size = memory->const_object_table_size;

  char* temp_class_name = calloc(30, sizeof(char));
  AddFreePtr(temp_class_name);
  snprintf(temp_class_name, 30, "%s.!__start", name);

  object_table[2].type[0] = 0x05;
  object_table[2].const_type = false;
  object_table[2].data.string_data = temp_class_name;

  struct Object temp;
  temp = object_table[0];
  object_table[0].type[0] = 0x04;
  object_table[0].const_type = false;
  object_table[0].data.uint64t_data = 0;
  NEW(2, 0, 2);
  object_table[0] = temp;
  struct Object* last_running_object = current_running_object;
  current_running_object = object_table + 2;

  InvokeClassFunction(2,"!__start", 1, 1, NULL);

  // AddBytecodeFile(import_bytecode_file, import_bytecode_file_size);

  object_table = origin_memory.object_table;
  object_table_size = origin_memory.object_table_size;
  const_object_table = origin_memory.const_object_table;
  const_object_table_size = origin_memory.const_object_table_size;

  current_running_object = last_running_object;*/

  // return memory->object_table + 2;
  //  free_list = NULL;

  //  InvokeCustomFunction(".!__start", 1, 1, NULL);
}

void AddBytecodeFile(const char* file) {
  TRACE_FUNCTION;
  // if (*file == '~') return;
  file++;
  printf("AddBytecodeFile: ~%s\n", file);
  const char* filename_start = file;
  while (*file != '~') {
    file++;
  }
  // file++;
  char* file_name = calloc(file - filename_start + 1, sizeof(char));
  memcpy(file_name, filename_start, file - filename_start);
  printf("file_name: %s\n", file_name);

  // const char* file_name = file;
  FILE* bytecode = fopen(file_name, "rb");
  if (bytecode == NULL) {
    printf("Error: Could not open file %s\n", file_name);
    EXIT_VM("AddBytecodeFile(const char*,size_t)", "Could not open file.");
  }
  fseek(bytecode, 0, SEEK_END);
  size_t bytecode_size = ftell(bytecode);
  void* bytecode_file = malloc(bytecode_size);
  void* bytecode_begin = bytecode_file;
  void* bytecode_end = (void*)((uintptr_t)bytecode_file + bytecode_size);
  fseek(bytecode, 0, SEEK_SET);
  fread(bytecode_file, 1, bytecode_size, bytecode);
  fclose(bytecode);

  /*while (*file != '\0') {
    file++;
  }
  file++;*/

  const unsigned int class_hash = hash(file_name);
  struct BytecodeFileList* current_bytecode_file_table =
      &bytecode_file_table[class_hash];
  bool is_exist = false;
  // const char* current_scope_name = NULL;
  while (current_bytecode_file_table->next != NULL) {
    if (strcmp(current_bytecode_file_table->name, file_name) == 0) {
      is_exist = true;
      break;
    }
    current_bytecode_file_table = current_bytecode_file_table->next;
  }
  // is_exist = true;

  if (!is_exist) {
    printf("Adding bytecode file: %s\n", file_name);

    current_file_count++;
    current_bytecode_file_table->next = malloc(sizeof(struct BytecodeFileList));
    // current_bytecode_file_table = current_bytecode_file_table->next;
    current_bytecode_file_table->name = file_name;
    char* name = calloc(strlen(file_name) + 3, sizeof(char));
    snprintf(name, strlen(file_name) + 3, "~%s~", file_name);
    // char* temp_class_name = calloc(30, sizeof(char));
    // AddFreePtr(temp_class_name);
    // snprintf(temp_class_name, 30, "%s.!__start", name);

    // const char* last_bytecode_file = current_bytecode_file;

    // current_scope_name = current_bytecode_file = temp_class_name;

    // current_bytecode_file_table->object =
    HandleBytecodeFile(name, bytecode_file, bytecode_size);

    // current_bytecode_file = last_bytecode_file;

    // current_bytecode_file_table->index = current_file_count;

    /*struct FileIndexList *current_bytecode_file_index_table =
    &file_index_table[hash(file)]; while
    (current_bytecode_file_index_table->next != NULL) {
      current_bytecode_file_index_table =
    current_bytecode_file_index_table->next;
    }
    current_bytecode_file_index_table->name = file;
    current_bytecode_file_index_table->index = current_file_count;
    current_bytecode_file_index_table->next = calloc(1,sizeof(struct
    FileIndexList)); AddFreePtr(current_bytecode_file_index_table->next);*/
    /*struct Object* bytecode_file_object = calloc(1, sizeof(struct Object));

    uint8_t* type_ptr = calloc(1, sizeof(uint8_t));
    bytecode_file_object->type = type_ptr;
    bytecode_file_object->type[0] = 0x09;
    bytecode_file_object->const_type = true;
    AddFreePtr(type_ptr);

    struct Class* class_data = NULL;
    const unsigned int class_hash = hash(temp_class_name);
    struct ClassList* current_class_table = &class_table[class_hash];
    while (current_class_table != NULL &&
           current_class_table->class.name != NULL) {
      if (strcmp(current_class_table->class.name, temp_class_name) == 0) {
        class_data = &current_class_table->class;
        break;
      }
      current_class_table = current_class_table->next;
    }

    if (class_data == NULL) {
      EXIT_VM("AddBytecodeFile(const char*,size_t)", "Class not found.");
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
            EXIT_VM("AddBytecodeFile(const char*,size_t)",
                    "Unsupported type.");
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
    class_object[0].data.string_data = temp_class_name;

    class_object[1].const_type = true;
    class_object[1].type[0] = 0x04;
    class_object[1].data.uint64t_data = class_data->members_size;
    bytecode_file_object->data.object_data = class_object;

    current_bytecode_file_table->object = bytecode_file_object;*/
  }

  /*const char* scope = file;
  while (*file != '\0') {
    file++;
  }
  file++;

  struct Class* current_object = NULL;

  unsigned int current_class_hash = hash(current_bytecode_file);
  struct ClassList* current_class_table = &class_table[current_class_hash];
  while (current_class_table != NULL &&
         current_class_table->class.name != NULL) {
    if (strcmp(current_class_table->class.name, current_bytecode_file) == 0) {
      current_object = &current_class_table->class;
      break;
    }
    current_class_table = current_class_table->next;
  }

  if (current_object == NULL)
    EXIT_VM("AddBytecodeFile(const char*,size_t)",
            "Not current class object.");
  if (current_object->members_size == 0)
    EXIT_VM("AddBytecodeFile(const char*,size_t)",
            "Not current class object.");

  const char* var_name = scope;

  size_t offset = 0;

  bool is_var_find = false;
  const unsigned int member_hash = hash(var_name);
  //printf("BF: %s\n", var_name);
  struct ClassVarInfoList* current_var_table =
      &(current_object->var_info_table[member_hash]);
  while (current_var_table != NULL && current_var_table->name != NULL) {
    if (strcmp(current_var_table->name, var_name) == 0) {
      offset = current_var_table->index;
      is_var_find = true;
      break;
    }
    current_var_table = current_var_table->next;
  }
  if (!is_var_find)
    EXIT_VM("AddBytecodeFile(const char*,size_t)", "Class Var not found.");*/

  // struct Object* object_data = current_object->members + offset;

  /*struct Object* object_data =
      current_running_object->data.object_data + offset;

  printf("object_data: %p\n",object_data);

  object_data->type = calloc(2, sizeof(uint8_t));
  AddFreePtr(object_data->type);
  object_data->type[0] = 0x07;
  object_data->type[1] = 0x09;
  object_data->const_type = true;
  // object_data->data.object_data = current_bytecode_file_table->object;
  object_data->data.reference_data =
      current_bytecode_file_table->object;

  struct BytecodeFileList* bytecode_file_list =
      &current_object->bytecode_file[hash(scope)];
  while (bytecode_file_list->next != NULL) {
    bytecode_file_list = bytecode_file_list->next;
  }
  bytecode_file_list->next = calloc(1, sizeof(struct BytecodeFileList));
  bytecode_file_list->object = current_bytecode_file_table->object;
  bytecode_file_list->name = scope;*/

  /*int temp_index = -1;
  struct FileIndexList *current_bytecode_file_index_table =
  &file_index_table[hash(file)]; while (current_bytecode_file_index_table->next
  != NULL) { if (strcmp(current_bytecode_file_index_table->name, file) == 0) {
      temp_index = current_bytecode_file_index_table->index;
      break;
    }
    current_bytecode_file_index_table = current_bytecode_file_index_table->next;
  }

  if(temp_index == -1)EXIT_VM("AddBytecodeFile(const char*,size_t)", "Not found
  file index.");*/

  /*char* name = calloc(15, sizeof(char));
  snprintf(name, 15, "~%d", current_bytecode_file_table->index);
  char* temp_class_name = calloc(30, sizeof(char));
  AddFreePtr(temp_class_name);
  snprintf(temp_class_name, 30, "%s.!__start", name);
  current_scope_name = temp_class_name;

  current_class_hash = hash(current_scope_name);
  current_class_table = &class_table[current_class_hash];
  while (current_class_table != NULL &&
         current_class_table->class.name != NULL) {
    if (strcmp(current_class_table->class.name, current_scope_name) == 0) {
      current_object = &current_class_table->class;
      break;
    }
    current_class_table = current_class_table->next;
  }*/

  /*current_object->memory->object_table[2].data.object_data =
      object_data->data.object_data;*/
  // current_bytecode_file = NULL;
}

int main(int argc, char* argv[]) {
  time_t start_time = clock();

  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    // EXIT_VM("main(int, char**)", "Invalid arguments.");
    return -1;
  }

  FILE* bytecode = fopen(argv[1], "rb");
  if (bytecode == NULL) {
    printf("Error: Could not open file %s\n", argv[1]);
    EXIT_VM("main(int, char**)", "Could not open file.");
    return -2;
  }

  if (aqvm_init() != 0) EXIT_VM("main(int, char**)", "INIT ERROR.");

  IsBigEndian();

  fseek(bytecode, 0, SEEK_END);
  size_t bytecode_size = ftell(bytecode);
  void* bytecode_file = malloc(bytecode_size);
  void* bytecode_begin = bytecode_file;
  void* bytecode_end = (void*)((uintptr_t)bytecode_file + bytecode_size);
  fseek(bytecode, 0, SEEK_SET);
  fread(bytecode_file, 1, bytecode_size, bytecode);
  fclose(bytecode);

  if (((char*)bytecode_file)[0] != 0x41 || ((char*)bytecode_file)[1] != 0x51 ||
      ((char*)bytecode_file)[2] != 0x42 || ((char*)bytecode_file)[3] != 0x43) {
    EXIT_VM("main(int, char**)", "Invalid bytecode file.");
    return -3;
  }

  if (((char*)bytecode_file)[4] != 0x00 || ((char*)bytecode_file)[5] != 0x00 ||
      ((char*)bytecode_file)[6] != 0x00 || ((char*)bytecode_file)[7] != 0x03) {
    EXIT_VM(
        "main(int, char**)",
        "This bytecode version is not supported, please check for updates.");
    return -4;
  }

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  /*memory = (struct Memory*)malloc(sizeof(struct Memory));
  object_table_size =  *(uint64_t*)bytecode_file
                               : SwapUint64t(*(uint64_t*)bytecode_file);
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
  memory->data = bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + object_table_size);
  memory->type[0] = bytecode_file;
  if (object_table_size % 2 != 0) {
    bytecode_file = (void*)((uintptr_t)bytecode_file + object_table_size / 2 +
  1); } else { bytecode_file = (void*)((uintptr_t)bytecode_file +
  object_table_size / 2);
  }*/

  /*size_t import_bytecode_file_size =
      is_big_endian ? *(uint64_t*)bytecode_file
                    : SwapUint64t(*(uint64_t*)bytecode_file);
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  void* import_bytecode_file = bytecode_file;

  for (size_t i = 0; i < import_bytecode_file_size; i++) {
    while (*(char*)bytecode_file != '\0') {
      bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    }
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    while (*(char*)bytecode_file != '\0') {
      bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    }
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
  }*/

  // bytecode_file = AddBytecodeFile(bytecode_file, bytecode_file_size);

  const_object_table_size = is_big_endian
                                ? *(uint64_t*)bytecode_file
                                : SwapUint64t(*(uint64_t*)bytecode_file);

  const_object_table =
      (struct Object*)malloc(const_object_table_size * sizeof(struct Object));

  if (const_object_table == NULL)
    EXIT_VM("main(int,char**)", "const_object_table malloc failed.");

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  for (size_t i = 0; i < const_object_table_size; i++) {
    // printf("const_object_table type: %i\n", *(uint8_t*)bytecode_file);
    const_object_table[i].type = bytecode_file;
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    switch (const_object_table[i].type[0]) {
      case 0x01:
        const_object_table[i].data.byte_data = *(int8_t*)bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        break;

      case 0x02:
        const_object_table[i].data.long_data = *(int64_t*)bytecode_file;
        const_object_table[i].data.long_data =
            is_big_endian ? const_object_table[i].data.long_data
                          : SwapLong(const_object_table[i].data.long_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x03:
        const_object_table[i].data.double_data = *(double*)bytecode_file;
        const_object_table[i].data.double_data =
            is_big_endian ? const_object_table[i].data.double_data
                          : SwapDouble(const_object_table[i].data.double_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x04:
        const_object_table[i].data.uint64t_data = *(uint64_t*)bytecode_file;
        const_object_table[i].data.uint64t_data =
            is_big_endian
                ? const_object_table[i].data.uint64t_data
                : SwapUint64t(const_object_table[i].data.uint64t_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x05: {
        size_t str_size = 0;
        bytecode_file = (void*)((uintptr_t)bytecode_file +
                                DecodeUleb128(bytecode_file, &str_size));
        const_object_table[i].data.string_data = bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + str_size);
        break;
      }

      case 0x06:
        const_object_table[i].data.ptr_data = *(void**)bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      default:
        EXIT_VM("main(int,char**)", "Unknown type.");
        break;
    }
  }

  object_table_size = is_big_endian ? *(uint64_t*)bytecode_file
                                    : SwapUint64t(*(uint64_t*)bytecode_file);

  object_table =
      (struct Object*)calloc(object_table_size, sizeof(struct Object));

  if (object_table == NULL)
    EXIT_VM("main(int,char**)", "object_table calloc failed.");

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  for (size_t i = 0; i < object_table_size; i++) {
    object_table[i].type = bytecode_file;
    // printf("object_table type: %zu\n", i);
    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)bytecode_file) {
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
          bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
          break;

        default:
          // printf("object_table type: %i\n", *(uint8_t*)bytecode_file);
          EXIT_VM("main(int,char**)", "Unsupported type.");
          break;
      }
    }
    if (object_table[i].type[0] != 0x00) object_table[i].const_type = true;
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
  }

  global_memory = calloc(1, sizeof(struct Memory));
  AddFreePtr(global_memory);
  global_memory->object_table = object_table;
  global_memory->const_object_table = const_object_table;
  global_memory->object_table_size = object_table_size;
  global_memory->const_object_table_size = const_object_table_size;

  while (bytecode_file < bytecode_end) {
    // printf("bytecode_file: %p\n", bytecode_file);
    // printf("bytecode_end: %p\n", bytecode_end);
    // printf("offset: %zu\n", (uintptr_t)bytecode_end -
    // (uintptr_t)bytecode_file);
    bytecode_file = AddClass(bytecode_file);
  }

  current_bytecode_file = ".!__start";
  free_list = NULL;

  InitializeNameTable(name_table);
  // printf("\nProgram started.\n");

  object_table[2].type[0] = 0x05;
  object_table[2].const_type = false;
  object_table[2].data.string_data = ".!__start";

  struct Object temp;
  temp = object_table[0];
  object_table[0].type[0] = 0x04;
  object_table[0].const_type = false;
  object_table[0].data.uint64t_data = 0;
  NEW(2, 0, 2);
  object_table[0] = temp;
  current_running_object = object_table + 2;

  // AddBytecodeFile(import_bytecode_file, import_bytecode_file_size);

  // InvokeCustomFunction(".!__start", 1, 1, NULL);
  InvokeClassFunction(2, "!__start", 1, 1, NULL);

  size_t* args = (size_t*)malloc(1 * sizeof(size_t));
  args[0] = 0;
  InternalObject args_obj = {1, args};
  printf("\n[INFO] EXIT value: ");
  print(args_obj, 0);
  free(args);

  // printf("\nProgram finished\n");
  FreeAllPtr();
  // FreeMemory(memory);
  free(bytecode_begin);
  free(object_table);
  free(const_object_table);

  time_t end_time = clock();

  time_t time_diff = end_time - start_time;
  printf("\n[INFO] Execution time: %zu ms\n", time_diff);

  return 0;
}
