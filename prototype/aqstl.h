// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#endif

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
      TraceCreate(__FUNCTION__)
*/

#define TRACE_FUNCTION

void EXIT_VM(const char* func_name, const char* message);

struct Object* object_table;

size_t object_table_size;

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

typedef struct {
  size_t size;
  size_t* index;
} InternalObject;

typedef void (*func_ptr)(InternalObject, size_t);

struct Pair {
  char* first;
  func_ptr second;
};

struct LinkedList {
  struct Pair pair;
  struct LinkedList* next;
};

struct LinkedList name_table[256];

void AddFreePtr(void* ptr);

struct Object* GetOriginData(struct Object* object);
struct Object* GetObjectData(size_t index);
struct Object* GetObjectObjectData(struct Object* data);
const char* GetStringData(size_t index);
const char* GetStringObjectData(struct Object* object);
uint64_t GetUint64tObjectData(struct Object* object);
uint64_t GetUint64tData(size_t index);
int64_t GetLongObjectData(struct Object* object);
double GetDoubleObjectData(struct Object* object);
double GetDoubleData(size_t index);
int64_t GetLongData(size_t index);
int8_t GetByteObjectData(struct Object* data);
int8_t GetByteData(size_t index);
struct Object* GetPtrObjectData(struct Object* object);
struct Object* GetPtrData(size_t index);
void SetObjectData(size_t index, struct Object* object);
void SetByteData(size_t, int8_t);
void SetLongData(size_t, int64_t);
void SetDoubleData(size_t, double);
void SetUint64tData(size_t, uint64_t);
void SetPtrData(size_t index, struct Object* ptr);
void SetStringData(size_t index, const char* string);
void SetReferenceData(size_t index, struct Object* object);
void SetConstData(size_t index, struct Object* object);
void SetObjectObjectData(struct Object* data, struct Object* object);
void SetByteObjectData(struct Object* data, int8_t);
void SetLongObjectData(struct Object* data, int64_t);
void SetDoubleObjectData(struct Object* data, double);
void SetUint64tObjectData(struct Object* data, uint64_t);
void SetPtrObjectData(struct Object* data, struct Object* ptr);
void SetStringObjectData(struct Object* data, const char* string);
void SetReferenceObjectData(struct Object* data, struct Object* object);
void SetConstObjectData(struct Object* data, struct Object* object);
void SetObjectObjectData(struct Object* data, struct Object* object);

void aqstl_print(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_print(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_print(InternalObject,size_t)", "Invalid return value.");
  struct Object* object = object_table + *args.index;
  object = GetOriginData(object);
  if (object == NULL)
    EXIT_VM("aqstl_print(InternalObject,size_t)", "Invalid object.");

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
      EXIT_VM("aqstl_print(InternalObject,size_t)", "Unsupported type.");
      break;
  }
}

void aqstl_vaprint(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid return value.");
  struct Object* object = object_table + args.index[0];
  object = GetOriginData(object);

  if (object == NULL || object->type == NULL || object->type[0] != 0x06)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid object.");

  for (size_t i = 1; i < GetUint64tObjectData(object->data.ptr_data) + 1; i++) {
    switch (GetOriginData(object->data.ptr_data + i)->type[0]) {
      case 0x01:
        SetLongData(return_value,
                    printf("%d", GetByteObjectData(object->data.ptr_data + i)));
        break;
      case 0x02:
        SetLongData(
            return_value,
            printf("%lld", GetLongObjectData(object->data.ptr_data + i)));
        break;
      case 0x03:
        SetLongData(
            return_value,
            printf("%.15f", GetDoubleObjectData(object->data.ptr_data + i)));
        break;
      case 0x04:
        SetLongData(
            return_value,
            printf("%zu", GetUint64tObjectData(object->data.ptr_data + i)));
        break;
      case 0x05:
        SetLongData(
            return_value,
            printf("%s", GetStringObjectData(object->data.ptr_data + i)));
        break;
      case 0x06:
        SetLongData(return_value,
                    printf("%p", GetPtrObjectData(object->data.ptr_data + i)));
        break;
      default:
        printf("Type: %u\n", (object->data.ptr_data + i)->type[0]);
        EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Unsupported type.");
        break;
    }
  }
}

unsigned int hash(const char* str) {
  TRACE_FUNCTION;
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % 256;
}

void AddFuncToNameTable(char* name, func_ptr func) {
  TRACE_FUNCTION;
  unsigned int name_hash = hash(name);
  struct LinkedList* table = &name_table[name_hash];
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.first = name;
  table->pair.second = func;
  table->next = (struct LinkedList*)malloc(sizeof(struct LinkedList));
  AddFreePtr(table->next);
  table->next->next = NULL;
  table->next->pair.first = NULL;
  table->next->pair.second = NULL;
}

void InitializeNameTable(struct LinkedList* list) {
  AddFuncToNameTable("__builtin_print", aqstl_print);
  AddFuncToNameTable("__builtin_vaprint", aqstl_vaprint);
}