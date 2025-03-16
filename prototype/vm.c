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
    // printf("[INFO] Run: ");
    return;
  }
  PrintStackRecursive(node->next);
  // printf("%s -> ", node->function_name);
}

void PrintStack() {
  PrintStackRecursive(call_stack);
  // printf("Success\n");
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

union Data {
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
};

struct Object* const_object_table;

size_t const_object_table_size;

struct Object* object_table;

size_t object_table_size;

typedef struct {
  size_t size;
  size_t* index;
} InternalObject;

typedef void (*func_ptr)(InternalObject, size_t);

struct Pair {
  char* first;
  func_ptr second;
};

enum Operator {
  OPERATOR_NOP = 0x00,
  OPERATOR_LOAD,
  OPERATOR_STORE,
  OPERATOR_NEW,
  OPERATOR_FREE,
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
  OPERATOR_INVOKE_CLASS,
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
  struct Bytecode* commands;
  size_t args_size;
  size_t* args;
} FuncInfo;

struct FuncPair {
  const char* first;
  FuncInfo second;
};

struct LinkedList {
  struct Pair pair;
  struct LinkedList* next;
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
  uint8_t* type;
  void* data;
  size_t size;
};

struct Class {
  const char* name;
  struct Object* members;
  size_t members_size;
  struct FuncList methods[1024];
};

struct ClassList {
  struct Class class;
  struct ClassList* next;
};

func_ptr GetFunction(const char* name);
FuncInfo GetCustomFunction(const char* name, size_t* args, size_t args_size);

struct Memory* memory;

struct LinkedList name_table[1024];

struct FuncList func_table[1024];

struct ClassList class_table[1024];

struct FreeList* free_list;

bool is_big_endian;

void EXIT_VM(const char* func_name, const char* message) {
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
  return (long)ux;
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
    EXIT_VM("GetPtrData(size_t)", "Unsupported Type.");
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
  if (data==NULL)
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
            printf("type: %i", reference_data.type[0]);
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

void SetPtrData(size_t index, struct Object* ptr) {
  TRACE_FUNCTION;
  if (index >= object_table_size)
    EXIT_VM("SetPtrData(size_t,struct Object*)", "Out of memory.");
  if (object_table[index].type[0] == 0x08)
    EXIT_VM("SetPtrData(size_t,struct Object*)", "Cannot change const data.");

  struct Object* data = object_table + index;
  while (data->type[0] == 0x07) data = data->data.reference_data;

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

  struct Object* temp = ptr;
  for (size_t i = 1; i < size; i++) {
    if (temp == NULL)
      EXIT_VM("SetPtrData(size_t,struct Object*)", "Invalid ptr.");
    if (data->type[i] == 0x00) break;
    if (temp->type[0] != data->type[i])
      EXIT_VM("SetPtrData(size_t,struct Object*)", "Invalid type.");
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

  if (data->const_type && data->type[0] != 0x01)
    EXIT_VM("SetByteData(size_t,int8_t)", "Cannot change const type.");

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

  if (data->const_type && data->type[0] != 0x02) {
    // printf("%zu,%i,%zu", index, data->type[0], value);
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

  if (data->const_type && data->type[0] != 0x03)
    EXIT_VM("SetDoubleData(size_t,double)", "Cannot change const type.");

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

  if (data->const_type && data->type[0] != 0x04)
    EXIT_VM("SetUint64tData(size_t,uint64_t)", "Cannot change const type.");

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
  while (data->type[0] == 0x07) data = data->data.reference_data;

  if (object_table[index].const_type && object_table[index].type[0] != 0x07)
    EXIT_VM("SetReferenceData(size_t,struct Object*)",
            "Cannot change const type.");

  if (object == NULL) {
    EXIT_VM("SetReferenceData(size_t,struct Object*)",
            "object is NULL.");
    //data->type[0] = 0x07;
    //data->data.reference_data = object;
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

  if (object_table[index].const_type && object_table[index].type[0] != 0x09)
    EXIT_VM("SetObjectData(size_t,struct Object*)",
            "Cannot change const type.");

  if (object == NULL) {
    EXIT_VM("SetObjectData(size_t,struct Object*)", "object is NULL.");
    //data->type[0] = 0x09;
    //data->data.object_data = object;
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
int NEW(size_t ptr, size_t size,size_t type) {
  TRACE_FUNCTION;
  if (ptr >= object_table_size)
    EXIT_VM("NEW(size_t, size_t)", "Out of memory.");
  if (size >= object_table_size)
    EXIT_VM("NEW(size_t, size_t)", "Out of memory.");

  size_t size_value = GetUint64tData(size);
  struct Object* data = calloc(size_value, sizeof(struct Object));
  AddFreePtr(data);

  if(type==0){    for (size_t i = 0; i < size_value; i++) {
    uint8_t* type = calloc(1, sizeof(uint8_t));
    data[i].type = 0x00;
    data[i].const_type = false;
    AddFreePtr(type);
  }}else{
  struct Object* type_data = object_table + type;
  type_data = GetOriginData(type_data);

  if(type_data->type[0] == 0x06){
  struct Object* current_type = type_data->data.ptr_data;
    for (size_t i = 0; i < size_value; i++) {
    uint8_t* type = calloc(1, sizeof(uint8_t));
    data[i].type = GetByteObjectData(current_type);
    data[i].const_type = false;
    AddFreePtr(type);
    current_type++;
  }
}else{
  for (size_t i = 0; i < size_value; i++) {
    uint8_t* type = calloc(1, sizeof(uint8_t));
    data[i].type = GetByteData(type);
    data[i].const_type = false;
    AddFreePtr(type);
  }
}
}

  struct Object* original_object = object_table + ptr;
  original_object = GetOriginData(original_object);

  if (original_object->type[0] == 0x09) {
    SetObjectData(ptr, data);
  } else {
    SetPtrData(ptr, data);
  }
  // WriteData(memory, ptr, &data, sizeof(data));
  return 0;
}
int FREE(size_t ptr) {
  free(GetPtrData(ptr));
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

  if (operand1_data->type[0] == operand2_data->type[0]) {
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
  }
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
  SetReferenceData(result, GetPtrData(operand1));

  return 0;
}
int InvokeCustomFunction(const char* name, size_t args_size,
                         size_t return_value, size_t* args);
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
      SetObjectData(result, GetObjectData(value));
      break;
    default:
      // printf("value type: %d\n", value_data->type[0]);
      EXIT_VM("EQUAL(size_t,size_t)", "Unsupported type.");
  }
  return 0;
}
size_t GOTO(size_t location) {
  TRACE_FUNCTION;
  return location;
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
int CONST(size_t result, size_t operand1) {
  TRACE_FUNCTION;
  if (result >= object_table_size)
    EXIT_VM("CONST(size_t,size_t)", "Out of object_table_size.");
  if (operand1 >= object_table_size)
    EXIT_VM("CONST(size_t,size_t)", "Out of object_table_size.");

  SetConstData(result, GetPtrData(operand1));
  return 0;
}

int InvokeClassFunction(size_t class, const char* name, size_t args_size,
                        size_t return_value, size_t* args);

int INVOKE_CLASS(size_t* args) {
  TRACE_FUNCTION;
  if (args == NULL) EXIT_VM("INVOKE_CLASS(size_t*)", "Invalid args.");
  size_t func = args[1];
  size_t arg_count = args[2];
  size_t return_value = args[3];
  size_t* invoke_args = NULL;
  if (arg_count > 0) {
    invoke_args = args + 4;
  }
  InternalObject args_obj = {arg_count - 1, invoke_args};

  return InvokeClassFunction(args[0], GetStringData(func), arg_count,
                             return_value, invoke_args);
}
unsigned int hash(const char* str);
int LOAD_MEMBER(size_t result, size_t class, size_t operand) {
  TRACE_FUNCTION;
  if (result >= object_table_size)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Out of object_table_size.");
  if (class >= object_table_size)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Out of object_table_size.");

  struct Object* class_data = object_table + class;
  class_data = GetOriginData(class_data);
  if (class_data == NULL || class_data->type[0] != 0x09)
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Error class data.");

  /*struct Object* object_member_count = class_data->data.object_data + 1;
  object_member_count = GetOriginData(object_member_count);
  if (operand >= GetUint64tObjectData(object_member_count))
    EXIT_VM("LOAD_MEMBER(size_t,size_t,size_t)", "Out of object_table_size.");*/

  struct Object* object_data = class_data->data.object_data + operand;

  SetReferenceData(result, object_data);
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
      SetLongData(return_value, printf("%f", GetDoubleData(*args.index)));
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

unsigned int hash(const char* str) {
  TRACE_FUNCTION;
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % 1024;
}

void InitializeNameTable(struct LinkedList* list) {
  TRACE_FUNCTION;
  unsigned int name_hash = hash("global::print");
  struct LinkedList* table = &list[name_hash];
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.first = "global::print";
  table->pair.second = print;
  table->next = (struct LinkedList*)malloc(sizeof(struct LinkedList));
  AddFreePtr(table->next);
  table->next->next = NULL;
  table->next->pair.first = NULL;
  table->next->pair.second = NULL;
}

void* AddClassMethod(void* location, struct FuncList* methods) {
  TRACE_FUNCTION;
  // void* original_location = location;
  // printf("point 1\n");

  struct FuncList* table = &methods[hash(location)];
  if (table == NULL)
    EXIT_VM("AddClassMethod(void*,struct FuncList*)", "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.second.location = location;
  table->pair.first = location;
  table->pair.second.name = location;
  printf("Name: %s\n", table->pair.second.name);
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  location = (void*)((uintptr_t)location +
                     DecodeUleb128(location, &table->pair.second.args_size));
  table->pair.second.args =
      (size_t*)calloc(table->pair.second.args_size, sizeof(size_t));
  // printf("args_size: %zu", table->pair.second.args_size);
  for (size_t i = 0; i < table->pair.second.args_size; i++) {
    location = (void*)((uintptr_t)location +
                       DecodeUleb128(location, &table->pair.second.args[i]));
  }

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
        location =
            Get3Parament(location, bytecode[i].args, bytecode[i].args + 1,bytecode[i].args + 2);
        break;

      case OPERATOR_FREE:
        bytecode[i].args = (size_t*)malloc(sizeof(size_t));
        location = Get1Parament(location, bytecode[i].args);
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

      case OPERATOR_INVOKE_CLASS:
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

  struct ClassList* table = &class_table[hash(location)];
  if (table == NULL) EXIT_VM("AddClass(void*)", "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }
  table->class.name = location;
  printf("Class Name: %s\n", table->class.name);
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
          printf("%d\n", *(uint8_t*)location);
          printf("%d\n", *(uint8_t*)((uintptr_t)location + 1));
          printf("%d\n", *(uint8_t*)((uintptr_t)location + 2));
          printf("%d\n", *(uint8_t*)((uintptr_t)location + 3));
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

  if (strcmp(table->class.name, "__start") == 0) {
    for (size_t i = 0; i < method_size; i++) {
      location = AddFunction(location);
    }
  } else {
    for (size_t i = 0; i < method_size; i++) {
      location = AddClassMethod(location, table->class.methods);
    }
  }

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

  printf("Name: %s\n", table->pair.second.name);

  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  location = (void*)((uintptr_t)location +
                     DecodeUleb128(location, &table->pair.second.args_size));
  table->pair.second.args =
      (size_t*)calloc(table->pair.second.args_size, sizeof(size_t));
  // printf("args_size: %zu", table->pair.second.args_size);
  for (size_t i = 0; i < table->pair.second.args_size; i++) {
    location = (void*)((uintptr_t)location +
                       DecodeUleb128(location, &table->pair.second.args[i]));
  }

  table->pair.second.commands_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  struct Bytecode* bytecode = (struct Bytecode*)calloc(
      table->pair.second.commands_size, sizeof(struct Bytecode));
  // printf("commands_size: %zu", table->pair.second.commands_size);
  printf("%zu\n", table->pair.second.commands_size);
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
        location =
            Get3Parament(location, bytecode[i].args, bytecode[i].args + 1,bytecode[i].args + 2);
        break;

      case OPERATOR_FREE:
        bytecode[i].args = (size_t*)malloc(sizeof(size_t));
        location = Get1Parament(location, bytecode[i].args);
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

      case OPERATOR_INVOKE_CLASS:
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
  printf("Class: %s, Name: %s\n", class,name);
  const unsigned int class_hash = hash(class);
  const struct ClassList* current_class_table = &class_table[class_hash];
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
          if (table->pair.second.args_size == args_size) {
            bool is_same = true;
            for (size_t i = 0; i < args_size - 1; i++) {
              if (args == NULL)
                EXIT_VM(
                    "GetClassFunction(const char*,const char*,size_t*,size_t)",
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
      }
    }
    current_class_table = current_class_table->next;
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
  while (table != NULL && table->pair.first != NULL) {
    if (table->pair.first == NULL)
      EXIT_VM("GetCustomFunction(const char*,size_t*,size_t)", "Invalid name.");
    if (strcmp(table->pair.first, name) == 0) {
      if (table->pair.second.args_size == args_size) {
        bool is_same = true;
        for (size_t i = 0; i < args_size - 1; i++) {
          if (args == NULL)
            EXIT_VM("GetCustomFunction(const char*,size_t*,size_t)",
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
  }

  printf("%s\n", name);
  EXIT_VM("GetCustomFunction(const char*,size_t*,size_t)",
          "Function not found.");
  return (FuncInfo){NULL, NULL, 0, NULL};
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
  FuncInfo func_info = GetClassFunction(class_name, name, args, args_size);
  if (args_size != func_info.args_size) {
    // printf("args_size: %zu\n", args_size);
    // printf("func_info.args_size: %zu\n", func_info.args_size);
    EXIT_VM("InvokeClassFunction(size_t,const char*,size_t,size_t,size_t*)",
            "Invalid args_size.");
  }
  // printf("object: %zu , %zu", func_info.args[0], return_value);
  // TODO(Class): Fixed this bug about return value.
  // object_table[func_info.args[0]] = object_table[return_value];
  func_info.args++;
  args_size--;
  for (size_t i = 0; i < args_size; i++) {
    object_table[func_info.args[i]] = object_table[args[i]];
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
        NEW(run_code[i].args[0], run_code[i].args[1],run_code[i].args[2]);
        break;
      case 0x04:
        FREE(run_code[i].args[0]);
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
        CONST(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x1A:
        INVOKE_CLASS(run_code[i].args);
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

  return 0;
}

int InvokeCustomFunction(const char* name, size_t args_size,
                         size_t return_value, size_t* args) {
  TRACE_FUNCTION;
  FuncInfo func_info = GetCustomFunction(name, args, args_size);
  if (args_size != func_info.args_size) {
    // printf("args_size: %zu\n", args_size);
    // printf("func_info.args_size: %zu\n", func_info.args_size);
    EXIT_VM("InvokeCustomFunction(const char*,size_t,size_t,size_t*)",
            "Invalid args_size.");
  }
  // printf("object: %zu , %zu", func_info.args[0], return_value);
  object_table[func_info.args[0]] = object_table[return_value];
  struct Object* return_object = object_table + func_info.args[0];
  func_info.args++;
  args_size--;
  for (size_t i = 0; i < args_size; i++) {
    object_table[func_info.args[i]] = object_table[args[i]];
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
        NEW(run_code[i].args[0], run_code[i].args[1],run_code[i].args[2]);
        break;
      case 0x04:
        FREE(run_code[i].args[0]);
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
        CONST(run_code[i].args[0], run_code[i].args[1]);
        break;
      case 0x1A:
        INVOKE_CLASS(run_code[i].args);
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

int main(int argc, char* argv[]) {
  time_t start_time = clock();

  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    EXIT_VM("main(int, char**)", "Invalid arguments.");
    return -1;
  }

  FILE* bytecode = fopen(argv[1], "rb");
  if (bytecode == NULL) {
    printf("Error: Could not open file %s\n", argv[1]);
    EXIT_VM("main(int, char**)", "Could not open file.");
    return -2;
  }

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
    printf("Error: Invalid bytecode file\n");
    EXIT_VM("main(int, char**)", "Invalid bytecode file.");
    return -3;
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

  while (bytecode_file < bytecode_end) {
    // printf("bytecode_file: %p\n", bytecode_file);
    // printf("bytecode_end: %p\n", bytecode_end);
    // printf("offset: %zu\n", (uintptr_t)bytecode_end -
    // (uintptr_t)bytecode_file);
    bytecode_file = AddClass(bytecode_file);
  }

  free_list = NULL;

  InitializeNameTable(name_table);
  // printf("\nProgram started.\n");

  InvokeCustomFunction("__start", 1, 1, NULL);

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