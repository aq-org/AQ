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
  OPERATOR_SAR,
  OPERATOR_IF,
  OPERATOR_AND,
  OPERATOR_OR,
  OPERATOR_XOR,
  OPERATOR_CMP,
  OPERATOR_INVOKE,
  OPERATOR_EQUAL,
  OPERATOR_GOTO,
  OPERATOR_THROW,
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
} FuncInfo;

struct FuncPair {
  char* first;
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

func_ptr GetFunction(const char* name);
FuncInfo GetCustomFunction(const char* name);

struct Memory* memory;

struct LinkedList name_table[1024];

struct FuncList func_table[1024];

struct FreeList* free_list;

bool is_big_endian;

void EXIT_VM(const char* func_name, const char* message) {
  fprintf(stderr, "[ERROR] %s: %s\n", func_name, message);
  exit(1);
}

void AddFreePtr(void* ptr) {
  struct FreeList* new_free_list =
      (struct FreeList*)malloc(sizeof(struct FreeList));
  if (new_free_list == NULL) EXIT_VM("AddFreePtr(void*)", "Out of memory.");
  new_free_list->ptr = ptr;
  new_free_list->next = free_list;
  free_list = new_free_list;
}

void FreeAllPtr() {
  struct FreeList* current = free_list;
  while (current != NULL) {
    struct FreeList* next = current->next;
    free(current->ptr);
    free(current);
    current = next;
  }
}

void IsBigEndian() {
  uint16_t test_data = 0x0011;
  is_big_endian = (*(uint8_t*)&test_data == 0x00);
}

int SwapInt(int x) {
  uint32_t ux = (uint32_t)x;
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  return (int)ux;
}

long SwapLong(long x) {
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

float SwapFloat(float x) {
  uint32_t ux;
  memcpy(&ux, &x, sizeof(uint32_t));
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  float result;
  memcpy(&result, &ux, sizeof(float));
  return result;
}

double SwapDouble(double x) {
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
  x = ((x << 56) & 0xFF00000000000000ULL) |
      ((x << 40) & 0x00FF000000000000ULL) |
      ((x << 24) & 0x0000FF0000000000ULL) | ((x << 8) & 0x000000FF00000000ULL) |
      ((x >> 8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
      ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
  return x;
}

struct Memory* InitializeMemory(void* data, void* type, size_t size) {
  struct Memory* memory_ptr = (struct Memory*)malloc(sizeof(struct Memory));
  if (memory_ptr == NULL)
    EXIT_VM("InitializeMemory(void*, void*, size_t)", "Out of memory.");
  memory_ptr->data = data;
  memory_ptr->type = type;
  memory_ptr->size = size;

  return memory_ptr;
}

void FreeMemory(struct Memory* memory_ptr) { free(memory_ptr); }

/*int SetType(const struct Memory* memory, size_t index, uint8_t type) {
  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }
}*/

int WriteData(const struct Memory* memory, const size_t index,
              const void* data_ptr, const size_t size) {
  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);

  return 0;
}

uint8_t GetType(const struct Memory* memory, size_t index) {
  if (index >= memory->size) EXIT_VM("GetType(size_t)", "Out of memory.");
  if (index % 2 != 0) {
    // ERROR
    printf("1GetType: %zu, Type: %d\n", index,
           (*(memory->type + (index / 2)) & 0x0F));
    return (*(memory->type + (index / 2)) & 0x0F);
  } else {
    printf("2GetType: %zu, Type: %d\n", index,
           (*(memory->type + (index / 2)) & 0xF0) >> 4);
    return *(memory->type + (index / 2)) & 0xF0 >> 4;
  }
}

void* GetPtrData(size_t index) {
  if (index + 7 >= memory->size)
    EXIT_VM("GetPtrData(size_t)", "Out of memory.");

  printf("GetPtrData: %p\n", *(void**)((uintptr_t)memory->data + index));
  return *(void**)((uintptr_t)memory->data + index);
}

int8_t GetByteData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("GetByteData(size_t)", "Out of memory.");
      printf("GetByteData: %d\n", *(int8_t*)((uintptr_t)memory->data + index));
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("GetByteData(size_t)", "Out of memory.");
      printf("GetByteData: %d\n",
             is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index)));
      return (int8_t)is_big_endian
                 ? *(int*)((uintptr_t)memory->data + index)
                 : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("GetByteData(size_t)", "Out of memory.");
      printf("GetByteData: %ld\n",
             is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index)));
      return (int8_t)is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("GetByteData(size_t)", "Out of memory.");
      printf("GetByteData: %f\n",
             is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index)));
      return (int8_t)is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("GetByteData(size_t)", "Out of memory.");
      printf("GetByteData: %f\n",
             is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index)));
      return (int8_t)is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("GetByteData(size_t)", "Out of memory.");
      printf("GetByteData: %zu\n",
             is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index)));
      return (int8_t)is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      EXIT_VM("GetByteData(size_t)", "Invalid type.");
  }
  return -1;
}

int GetIntData(size_t index) {
  printf("index: %zu\n", index);
  printf("GetType: %hhu\n", GetType(memory, index));
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      printf("GetIntData: %zu, value: %d\n", index,
             *(int8_t*)((uintptr_t)memory->data + index));
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("GetIntData(size_t)", "0x02 Out of memory.");
      printf("GetIntData: %zu, value: %d\n", index,
             is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index)));
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      printf("GetIntData: %ld\n",
             is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index)));
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      printf("GetIntData: %f\n",
             is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index)));
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      printf("GetIntData: %f\n",
             is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index)));
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("GetIntData(size_t)", "Out of memory.");
      printf("GetIntData: %zu\n",
             is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index)));
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      EXIT_VM("GetIntData(size_t)", "Invalid type.");
  }
  return -1;
}

long GetLongData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("GetLongData(size_t)", "Out of memory.");
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("GetLongData(size_t)", "Out of memory.");
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("GetLongData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("GetLongData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("GetLongData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("GetLongData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      EXIT_VM("GetLongData(size_t)", "Invalid type.");
  }
  return -1;
}

float GetFloatData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("GetFloatData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      EXIT_VM("GetFloatData(size_t)", "Invalid type.");
  }
  return -1;
}

double GetDoubleData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("GetDoubleData(size_t)", "Out of memory.");
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("GetDoubleData(size_t)", "Out of memory.");
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("GetDoubleData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("GetDoubleData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("GetDoubleData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("GetDoubleData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      EXIT_VM("GetDoubleData(size_t)", "Invalid type.");
  }
  return -1;
}

uint64_t GetUint64tData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("GetUint64tData(size_t)", "Out of memory.");
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("GetUint64tData(size_t)", "Out of memory.");
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("GetUint64tData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("GetUint64tData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("GetUint64tData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("GetUint64tData(size_t)", "Out of memory.");
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      EXIT_VM("GetUint64tData(size_t)", "Invalid type.");
  }
  return -1;
}

void SetPtrData(size_t index, void* ptr) {
  if (index + 7 >= memory->size)
    EXIT_VM("SetPtrData(size_t, void*)", "Out of memory.");
  *(void**)((uintptr_t)memory->data + index) = ptr;
}

void SetByteData(size_t index, int8_t value) {
  printf("SetByteData: %zu, value: %d\n", index, value);
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("SetByteData(size_t, int8_t)", "Out of memory.");
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("SetByteData(size_t, int8_t)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("SetByteData(size_t, int8_t)", "Out of memory.");
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("SetByteData(size_t, int8_t)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("SetByteData(size_t, int8_t)", "Out of memory.");
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("SetByteData(size_t, int8_t)", "Out of memory.");
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      EXIT_VM("SetByteData(size_t, int8_t)", "Invalid type.");
  }
}

void SetIntData(size_t index, int value) {
  printf("SetIntData: %zu, value: %d\n", index, value);
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("SetIntData(size_t, int)", "Out of memory.");
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      EXIT_VM("SetIntData(size_t, int)", "Invalid type.");
  }
  printf("SetIntData: %zu, Result: %d\n", index, GetIntData(index));
}

void SetLongData(size_t index, long value) {
  printf("SetLongData: %ld\n", value);
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("SetLongData(size_t, long)", "Out of memory.");
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("SetLongData(size_t, long)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("SetLongData(size_t, long)", "Out of memory.");
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("SetLongData(size_t, long)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("SetLongData(size_t, long)", "Out of memory.");
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("SetLongData(size_t, long)", "Out of memory.");
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      EXIT_VM("SetLongData(size_t, long)", "Invalid type.");
  }
}

void SetFloatData(size_t index, float value) {
  printf("SetFloatData: %f\n", value);
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("SetFloatData(size_t, float)", "Out of memory.");
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      EXIT_VM("SetFloatData(size_t, float)", "Invalid type.");
  }
}

void SetDoubleData(size_t index, double value) {
  printf("SetDoubleData: %f\n", value);
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("SetDoubleData(size_t, double)", "Out of memory.");
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("SetDoubleData(size_t, double)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("SetDoubleData(size_t, double)", "Out of memory.");
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("SetDoubleData(size_t, double)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("SetDoubleData(size_t, double)", "Out of memory.");
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("SetDoubleData(size_t, double)", "Out of memory.");
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      EXIT_VM("SetDoubleData(size_t, double)", "Invalid type.");
  }
}

void SetUint64tData(size_t index, uint64_t value) {
  printf("SetUint64tData: %zu\n", value);
  switch (GetType(memory, index)) {
    case 0x01:
      if (index >= memory->size)
        EXIT_VM("SetUint64tData(size_t, uint64_t)", "Out of memory.");
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      if (index + 3 >= memory->size)
        EXIT_VM("SetUint64tData(size_t, uint64_t)", "Out of memory.");
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      if (index + 7 >= memory->size)
        EXIT_VM("SetUint64tData(size_t, uint64_t)", "Out of memory.");
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      if (index + 3 >= memory->size)
        EXIT_VM("SetUint64tData(size_t, uint64_t)", "Out of memory.");
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      if (index + 7 >= memory->size)
        EXIT_VM("SetUint64tData(size_t, uint64_t)", "Out of memory.");
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      if (index + 7 >= memory->size)
        EXIT_VM("SetUint64tData(size_t, uint64_t)", "Out of memory.");
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      EXIT_VM("SetUint64tData(size_t, uint64_t)", "Invalid type.");
  }
}

size_t DecodeUleb128(const uint8_t* input, size_t* result) {
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
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  return ptr;
}

void* Get2Parament(void* ptr, size_t* first, size_t* second) {
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, second));
  return ptr;
}

void* Get3Parament(void* ptr, size_t* first, size_t* second, size_t* third) {
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, second));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, third));
  return ptr;
}

void* Get4Parament(void* ptr, size_t* first, size_t* second, size_t* third,
                   size_t* fourth) {
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, first));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, second));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, third));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, fourth));
  return ptr;
}

int INVOKE(size_t* args);

size_t* GetUnknownCountParament(void** ptr) {
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

int NOP() { return 0; }
int LOAD(size_t ptr, size_t operand) {
  switch (GetType(memory, operand)) {
    case 0x01:
      SetByteData(operand, *(int8_t*)((uintptr_t)memory->data + ptr));
      break;
    case 0x02:
      SetIntData(operand, SwapInt(*(int*)((uintptr_t)memory->data + ptr)));
      break;
    case 0x03:
      SetLongData(operand, SwapLong(*(long*)((uintptr_t)memory->data + ptr)));
      break;
    case 0x04:
      SetFloatData(operand,
                   SwapFloat(*(float*)((uintptr_t)memory->data + ptr)));
      break;
    case 0x05:
      SetDoubleData(operand,
                    SwapDouble(*(double*)((uintptr_t)memory->data + ptr)));
      break;
    case 0x06:
      SetUint64tData(operand,
                     SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + ptr)));
      break;
    case 0x0F:
      SetPtrData(operand, *(void**)((uintptr_t)memory->data + ptr));
      break;
    default:
      EXIT_VM("LOAD(size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int STORE(size_t ptr, size_t operand) {
  switch (GetType(memory, operand)) {
    case 0x01:
      *(uint64_t*)((uintptr_t)memory->data + ptr) = GetByteData(operand);
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + ptr) = GetIntData(operand);
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + ptr) = GetLongData(operand);
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + ptr) = GetFloatData(operand);
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + ptr) = GetDoubleData(operand);
      break;
    case 0x06:
      *(uint64_t*)((uintptr_t)memory->data + ptr) = GetUint64tData(operand);
      break;
    case 0x0F:
      *(void**)((uintptr_t)memory->data + ptr) = GetPtrData(operand);
      break;
    default:
      EXIT_VM("STORE(size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int NEW(size_t ptr, size_t size) {
  size_t size_value = GetUint64tData(size);
  void* data = malloc(size_value);
  WriteData(memory, ptr, &data, sizeof(data));
  return 0;
}
int FREE(size_t ptr) {
  free(*(void**)((uintptr_t)memory->data + ptr));
  return 0;
}
int PTR(size_t index, size_t ptr) {
  printf("index: %zu\n", index);
  printf("PTR: %p\n", (void*)((uintptr_t)memory->data + index));
  SetPtrData(ptr, (void*)((uintptr_t)memory->data + index));
  return 0;
}
int ADD(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x0F && (GetType(memory, operand1) == 0x0F ||
                                          GetType(memory, operand2) == 0x0F)) {
    if (GetType(memory, operand1) == 0x0F) {
      SetPtrData(result, (void*)((uintptr_t)GetPtrData(operand1) +
                                 GetLongData(operand2)));
    } else if (GetType(memory, operand2) == 0x0F) {
      SetPtrData(result, (void*)((uintptr_t)GetPtrData(operand2) +
                                 GetLongData(operand1)));
    }
  } else if (GetType(memory, result) == 0x06 ||
             GetType(memory, operand1) == 0x06 ||
             GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) + GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) + GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) + GetUint64tData(operand2));
        break;
      case 0x04:
        SetFloatData(result,
                     GetUint64tData(operand1) + GetUint64tData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetUint64tData(operand1) + GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) + GetUint64tData(operand2));
      default:
        EXIT_VM("ADD(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x05 ||
             GetType(memory, operand1) == 0x05 ||
             GetType(memory, operand2) == 0x05) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) + GetByteData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetFloatData(operand1) + GetFloatData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      default:
        EXIT_VM("ADD(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetFloatData(operand1) + GetFloatData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetFloatData(operand1) + GetFloatData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetFloatData(operand1) + GetFloatData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetFloatData(operand1) + GetFloatData(operand2));
        break;
      default:
        EXIT_VM("ADD(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) + GetLongData(operand2));
        break;
      default:
        EXIT_VM("ADD(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x02:
        printf("Operand1: %d\n", GetIntData(operand1));
        printf("Operand2: %d\n", GetIntData(operand2));
        SetIntData(result, GetIntData(operand1) + GetIntData(operand2));
        break;
      default:
        EXIT_VM("ADD(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) + GetByteData(operand2));
        break;
      default:
        EXIT_VM("ADD(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("ADD(size_t, size_t, size_t)", "Invalid type.");
  }

  printf("Add Result: %zu\n", GetUint64tData(result));
  return 0;
}
int SUB(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x0F && (GetType(memory, operand1) == 0x0F ||
                                          GetType(memory, operand2) == 0x0F)) {
    if (GetType(memory, operand1) == 0x0F) {
      SetPtrData(result, (void*)((uintptr_t)GetPtrData(operand1) -
                                 GetLongData(operand2)));
    } else if (GetType(memory, operand2) == 0x0F) {
      SetPtrData(result, (void*)((uintptr_t)GetPtrData(operand2) -
                                 GetLongData(operand1)));
    }
  } else if (GetType(memory, operand1) == 0x0F &&
             GetType(memory, operand2) == 0x0F) {
    SetLongData(result, (uintptr_t)GetPtrData(operand1) -
                            (uintptr_t)GetPtrData(operand2));
  } else if (GetType(memory, result) == 0x06 ||
             GetType(memory, operand1) == 0x06 ||
             GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) - GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) - GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) - GetUint64tData(operand2));
        break;
      case 0x04:
        SetFloatData(result,
                     GetUint64tData(operand1) - GetUint64tData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetUint64tData(operand1) - GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) - GetUint64tData(operand2));
      default:
        EXIT_VM("SUB(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x05 ||
             GetType(memory, operand1) == 0x05 ||
             GetType(memory, operand2) == 0x05) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      default:
        EXIT_VM("SUB(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetFloatData(operand1) - GetFloatData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetFloatData(operand1) - GetFloatData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetFloatData(operand1) - GetFloatData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetFloatData(operand1) - GetFloatData(operand2));
        break;
      default:
        EXIT_VM("SUB(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) - GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) - GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) - GetLongData(operand2));
        break;
      default:
        EXIT_VM("SUB(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) - GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) - GetIntData(operand2));
        break;
      default:
        EXIT_VM("SUB(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) - GetByteData(operand2));
        break;
      default:
        EXIT_VM("SUB(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("SUB(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int MUL(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) * GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) * GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) * GetUint64tData(operand2));
        break;
      case 0x04:
        SetFloatData(result,
                     GetUint64tData(operand1) * GetUint64tData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetUint64tData(operand1) * GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) * GetUint64tData(operand2));
      default:
        EXIT_VM("MUL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x05 ||
             GetType(memory, operand1) == 0x05 ||
             GetType(memory, operand2) == 0x05) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      default:
        EXIT_VM("MUL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetFloatData(operand1) * GetFloatData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetFloatData(operand1) * GetFloatData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetFloatData(operand1) * GetFloatData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetFloatData(operand1) * GetFloatData(operand2));
        break;
      default:
        EXIT_VM("MUL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) * GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) * GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) * GetLongData(operand2));
        break;
      default:
        EXIT_VM("MUL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) * GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) * GetIntData(operand2));
        break;
      default:
        EXIT_VM("MUL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) * GetByteData(operand2));
        break;
      default:
        EXIT_VM("MUL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("MUL(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int DIV(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) / GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) / GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) / GetUint64tData(operand2));
        break;
      case 0x04:
        SetFloatData(result, (double)GetUint64tData(operand1) /
                                 (double)GetUint64tData(operand2));
        break;
      case 0x05:
        SetDoubleData(result, (double)GetUint64tData(operand1) /
                                  (double)GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) / GetUint64tData(operand2));
      default:
        EXIT_VM("DIV(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x05 ||
             GetType(memory, operand1) == 0x05 ||
             GetType(memory, operand2) == 0x05) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      default:
        EXIT_VM("DIV(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetFloatData(operand1) / GetFloatData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetFloatData(operand1) / GetFloatData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetFloatData(operand1) / GetFloatData(operand2));
        break;
      case 0x04:
        SetFloatData(result, GetFloatData(operand1) / GetFloatData(operand2));
        break;
      default:
        EXIT_VM("DIV(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) / GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) / GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) / GetLongData(operand2));
        break;
      default:
        EXIT_VM("DIV(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) / GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) / GetIntData(operand2));
        break;
      default:
        EXIT_VM("DIV(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) / GetByteData(operand2));
        break;
      default:
        EXIT_VM("DIV(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("DIV(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int REM(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) % GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) % GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) % GetUint64tData(operand2));
        break;
      case 0x04:
        SetFloatData(result,
                     GetUint64tData(operand1) % GetUint64tData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetUint64tData(operand1) % GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) % GetUint64tData(operand2));
      default:
        EXIT_VM("REM(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) % GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) % GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) % GetLongData(operand2));
        break;
      default:
        EXIT_VM("REM(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) % GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) % GetIntData(operand2));
        break;
      default:
        EXIT_VM("REM(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) % GetByteData(operand2));
        break;
      default:
        EXIT_VM("REM(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("REM(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int NEG(size_t result, size_t operand1) {
  if (GetType(memory, result) == 0x05 || GetType(memory, operand1) == 0x05) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, -GetDoubleData(operand1));
        break;
      case 0x02:
        SetIntData(result, -GetDoubleData(operand1));
        break;
      case 0x03:
        SetLongData(result, -GetDoubleData(operand1));
        break;
      case 0x04:
        SetFloatData(result, -GetDoubleData(operand1));
        break;
      case 0x05:
        SetDoubleData(result, -GetDoubleData(operand1));
        break;
      default:
        EXIT_VM("NEG(size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, -GetFloatData(operand1));
        break;
      case 0x02:
        SetIntData(result, -GetFloatData(operand1));
        break;
      case 0x03:
        SetLongData(result, -GetFloatData(operand1));
        break;
      case 0x04:
        SetFloatData(result, -GetFloatData(operand1));
        break;
      default:
        EXIT_VM("NEG(size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, -GetLongData(operand1));
        break;
      case 0x02:
        SetIntData(result, -GetLongData(operand1));
        break;
      case 0x03:
        SetLongData(result, -GetLongData(operand1));
        break;
      default:
        EXIT_VM("NEG(size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, -GetIntData(operand1));
        break;
      case 0x02:
        SetIntData(result, -GetIntData(operand1));
        break;
      default:
        EXIT_VM("NEG(size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, -GetByteData(operand1));
        break;
      default:
        EXIT_VM("NEG(size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("NEG(size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int SHL(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetUint64tData(operand1)
                                << GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1)
                               << GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetUint64tData(operand1)
                                << GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result, GetUint64tData(operand1)
                                   << GetUint64tData(operand2));
      default:
        EXIT_VM("SHL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) << GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) << GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) << GetLongData(operand2));
        break;
      default:
        EXIT_VM("SHL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) << GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) << GetIntData(operand2));
        break;
      default:
        EXIT_VM("SHL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) << GetByteData(operand2));
        break;
      default:
        EXIT_VM("SHL(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("SHL(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int SHR(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result,
                   GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) >> GetUint64tData(operand2));
      default:
        EXIT_VM("SHR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      default:
        EXIT_VM("SHR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) >> GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) >> GetIntData(operand2));
        break;
      default:
        EXIT_VM("SHR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) >> GetByteData(operand2));
        break;
      default:
        EXIT_VM("SHR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("SHR(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int SAR(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result,
                   GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) >> GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) >> GetUint64tData(operand2));
      default:
        EXIT_VM("SAR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) >> GetLongData(operand2));
        break;
      default:
        EXIT_VM("SAR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) >> GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) >> GetIntData(operand2));
        break;
      default:
        EXIT_VM("SAR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) >> GetByteData(operand2));
        break;
      default:
        EXIT_VM("SAR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("SAR(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int InvokeCustomFunction(const char* name);
size_t IF(size_t condition, size_t true_branche, size_t false_branche) {
  printf("condition: %d\n", GetByteData(condition));
  if (GetByteData(condition) != 0) {
    return true_branche;
  } else {
    return false_branche;
  }
}
int AND(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) & GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) & GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) & GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) & GetUint64tData(operand2));
      default:
        EXIT_VM("AND(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) & GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) & GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) & GetLongData(operand2));
        break;
      default:
        EXIT_VM("AND(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) & GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) & GetIntData(operand2));
        break;
      default:
        EXIT_VM("AND(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) & GetByteData(operand2));
        break;
      default:
        EXIT_VM("AND(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("AND(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int OR(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) | GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) | GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) | GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) | GetUint64tData(operand2));
      default:
        EXIT_VM("OR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) | GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) | GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) | GetLongData(operand2));
        break;
      default:
        EXIT_VM("OR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) | GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) | GetIntData(operand2));
        break;
      default:
        EXIT_VM("OR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) | GetByteData(operand2));
        break;
      default:
        EXIT_VM("OR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("OR(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int XOR(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x06 || GetType(memory, operand1) == 0x06 ||
      GetType(memory, operand2) == 0x06) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result,
                    GetUint64tData(operand1) ^ GetUint64tData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetUint64tData(operand1) ^ GetUint64tData(operand2));
        break;
      case 0x03:
        SetLongData(result,
                    GetUint64tData(operand1) ^ GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) ^ GetUint64tData(operand2));
      default:
        EXIT_VM("XOR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetLongData(operand1) ^ GetLongData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetLongData(operand1) ^ GetLongData(operand2));
        break;
      case 0x03:
        SetLongData(result, GetLongData(operand1) ^ GetLongData(operand2));
        break;
      default:
        EXIT_VM("XOR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) ^ GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) ^ GetIntData(operand2));
        break;
      default:
        EXIT_VM("XOR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) ^ GetByteData(operand2));
        break;
      default:
        EXIT_VM("XOR(size_t, size_t, size_t)", "Invalid type.");
    }
  } else {
    EXIT_VM("XOR(size_t, size_t, size_t)", "Invalid type.");
  }
  return 0;
}
int CMP(size_t result, size_t opcode, size_t operand1, size_t operand2) {
  switch (opcode) {
    case 0x00:
      if (GetType(memory, result) == 0x06 ||
          GetType(memory, operand1) == 0x06 ||
          GetType(memory, operand2) == 0x06) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetUint64tData(operand1) == GetUint64tData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetUint64tData(operand1) == GetUint64tData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetUint64tData(operand1) == GetUint64tData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetUint64tData(operand1) == GetUint64tData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetUint64tData(operand1) == GetUint64tData(operand2));
            break;
          case 0x06:
            SetUint64tData(
                result, GetUint64tData(operand1) == GetUint64tData(operand2));
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x05 ||
                 GetType(memory, operand1) == 0x05 ||
                 GetType(memory, operand2) == 0x05) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetFloatData(operand1) == GetFloatData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetFloatData(operand1) == GetFloatData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetFloatData(operand1) == GetFloatData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetFloatData(operand1) == GetFloatData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetLongData(operand1) == GetLongData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetLongData(operand1) == GetLongData(operand2));
            break;
          case 0x03:
            SetLongData(result, GetLongData(operand1) == GetLongData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetIntData(operand1) == GetIntData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetIntData(operand1) == GetIntData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) == GetByteData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else {
        EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
      }
      break;
    case 0x01:
      if (GetType(memory, result) == 0x06 ||
          GetType(memory, operand1) == 0x06 ||
          GetType(memory, operand2) == 0x06) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetUint64tData(operand1) != GetUint64tData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetUint64tData(operand1) != GetUint64tData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetUint64tData(operand1) != GetUint64tData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetUint64tData(operand1) != GetUint64tData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetUint64tData(operand1) != GetUint64tData(operand2));
            break;
          case 0x06:
            SetUint64tData(
                result, GetUint64tData(operand1) != GetUint64tData(operand2));
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x05 ||
                 GetType(memory, operand1) == 0x05 ||
                 GetType(memory, operand2) == 0x05) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetFloatData(operand1) != GetFloatData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetFloatData(operand1) != GetFloatData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetFloatData(operand1) != GetFloatData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetFloatData(operand1) != GetFloatData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetLongData(operand1) != GetLongData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetLongData(operand1) != GetLongData(operand2));
            break;
          case 0x03:
            SetLongData(result, GetLongData(operand1) != GetLongData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetIntData(operand1) != GetIntData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetIntData(operand1) != GetIntData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) != GetByteData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else {
        EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
      }
      break;
    case 0x02:
      if (GetType(memory, result) == 0x06 ||
          GetType(memory, operand1) == 0x06 ||
          GetType(memory, operand2) == 0x06) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetUint64tData(operand1) > GetUint64tData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetUint64tData(operand1) > GetUint64tData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetUint64tData(operand1) > GetUint64tData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetUint64tData(operand1) > GetUint64tData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetUint64tData(operand1) > GetUint64tData(operand2));
            break;
          case 0x06:
            SetUint64tData(result,
                           GetUint64tData(operand1) > GetUint64tData(operand2));
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x05 ||
                 GetType(memory, operand1) == 0x05 ||
                 GetType(memory, operand2) == 0x05) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetFloatData(operand1) > GetFloatData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetFloatData(operand1) > GetFloatData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetFloatData(operand1) > GetFloatData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetFloatData(operand1) > GetFloatData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetLongData(operand1) > GetLongData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetLongData(operand1) > GetLongData(operand2));
            break;
          case 0x03:
            SetLongData(result, GetLongData(operand1) > GetLongData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetIntData(operand1) > GetIntData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetIntData(operand1) > GetIntData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) > GetByteData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else {
        EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
      }
      break;
    case 0x03:
      if (GetType(memory, result) == 0x06 ||
          GetType(memory, operand1) == 0x06 ||
          GetType(memory, operand2) == 0x06) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetUint64tData(operand1) >= GetUint64tData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetUint64tData(operand1) >= GetUint64tData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetUint64tData(operand1) >= GetUint64tData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetUint64tData(operand1) >= GetUint64tData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetUint64tData(operand1) >= GetUint64tData(operand2));
            break;
          case 0x06:
            SetUint64tData(
                result, GetUint64tData(operand1) >= GetUint64tData(operand2));
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x05 ||
                 GetType(memory, operand1) == 0x05 ||
                 GetType(memory, operand2) == 0x05) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetLongData(operand1) >= GetLongData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetLongData(operand1) >= GetLongData(operand2));
            break;
          case 0x03:
            SetLongData(result, GetLongData(operand1) >= GetLongData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetIntData(operand1) >= GetIntData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetIntData(operand1) >= GetIntData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) >= GetByteData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else {
        EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
      }
      break;
    case 0x04:
      if (GetType(memory, result) == 0x06 ||
          GetType(memory, operand1) == 0x06 ||
          GetType(memory, operand2) == 0x06) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetUint64tData(operand1) < GetUint64tData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetUint64tData(operand1) < GetUint64tData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetUint64tData(operand1) < GetUint64tData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetUint64tData(operand1) < GetUint64tData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetUint64tData(operand1) < GetUint64tData(operand2));
            break;
          case 0x06:
            SetUint64tData(result,
                           GetUint64tData(operand1) < GetUint64tData(operand2));
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x05 ||
                 GetType(memory, operand1) == 0x05 ||
                 GetType(memory, operand2) == 0x05) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetFloatData(operand1) < GetFloatData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetFloatData(operand1) < GetFloatData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetFloatData(operand1) < GetFloatData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetFloatData(operand1) < GetFloatData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetLongData(operand1) < GetLongData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetLongData(operand1) < GetLongData(operand2));
            break;
          case 0x03:
            SetLongData(result, GetLongData(operand1) < GetLongData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetIntData(operand1) < GetIntData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetIntData(operand1) < GetIntData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) < GetByteData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else {
        EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
      }
      break;
    case 0x05:
      if (GetType(memory, result) == 0x06 ||
          GetType(memory, operand1) == 0x06 ||
          GetType(memory, operand2) == 0x06) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetUint64tData(operand1) <= GetUint64tData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetUint64tData(operand1) <= GetUint64tData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetUint64tData(operand1) <= GetUint64tData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetUint64tData(operand1) <= GetUint64tData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetUint64tData(operand1) <= GetUint64tData(operand2));
            break;
          case 0x06:
            SetUint64tData(
                result, GetUint64tData(operand1) <= GetUint64tData(operand2));
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x05 ||
                 GetType(memory, operand1) == 0x05 ||
                 GetType(memory, operand2) == 0x05) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x05:
            SetDoubleData(result,
                          GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result,
                        GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          case 0x02:
            SetIntData(result,
                       GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          case 0x03:
            SetLongData(result,
                        GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          case 0x04:
            SetFloatData(result,
                         GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetLongData(operand1) <= GetLongData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetLongData(operand1) <= GetLongData(operand2));
            break;
          case 0x03:
            SetLongData(result, GetLongData(operand1) <= GetLongData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetIntData(operand1) <= GetIntData(operand2));
            break;
          case 0x02:
            SetIntData(result, GetIntData(operand1) <= GetIntData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) <= GetByteData(operand2));
            break;
          default:
            EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
        }
      } else {
        EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid type.");
      }
      break;
    default:
      EXIT_VM("CMP(size_t, size_t, size_t, size_t)", "Invalid opcode.");
  }
  return 0;
}
int INVOKE(size_t* args) {
  size_t func = args[0];
  size_t arg_count = args[1];
  size_t return_value = args[2];
  size_t* invoke_args = NULL;
  if (arg_count > 0) {
    invoke_args = args + 3;
  }
  InternalObject args_obj = {arg_count, invoke_args};
  func_ptr invoke_func = GetFunction((char*)GetPtrData(func));
  if (invoke_func != NULL) {
    invoke_func(args_obj, return_value);
    return 0;
  }

  return InvokeCustomFunction((char*)GetPtrData(func));
}
int EQUAL(size_t result, size_t value) {
  switch (GetType(memory, result)) {
    case 0x01:
      SetByteData(value, GetByteData(result));
      break;
    case 0x02:
      SetIntData(value, GetIntData(result));
      break;
    case 0x03:
      SetLongData(value, GetLongData(result));
      break;
    case 0x04:
      SetFloatData(value, GetFloatData(result));
      break;
    case 0x05:
      SetDoubleData(value, GetDoubleData(result));
      break;
    case 0x06:
      SetUint64tData(value, GetUint64tData(result));
      break;
    default:
      EXIT_VM("EQUAL(size_t, size_t)", "Invalid type.");
  }
  return 0;
}
size_t GOTO(size_t location) { return location; }
int THROW() { return 0; }
int WIDE() { return 0; }

void print(InternalObject args, size_t return_value) {
  SetIntData(return_value, printf("%s", (char*)GetPtrData(*args.index)));
}

unsigned int hash(const char* str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % 1024;
}

void InitializeNameTable(struct LinkedList* list) {
  const unsigned int name_hash = hash("print");
  struct LinkedList* table = &list[name_hash];
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.first = "print";
  table->pair.second = print;
  table->next = (struct LinkedList*)malloc(sizeof(struct LinkedList));
  AddFreePtr(table->next);
  table->next->next = NULL;
  table->next->pair.first = NULL;
  table->next->pair.second = NULL;
}

void* AddFunction(void* location) {
  void* original_location = location;
  printf("point 1\n");

  struct FuncList* table = &func_table[hash(location)];
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.second.location = location;
  table->pair.first = location;
  table->pair.second.name = location;
  printf("Add: %s\n", table->pair.second.name);
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);
  printf("point 2\n");

  printf("offset: %zu\n", (uintptr_t)location - (uintptr_t)original_location);

  table->pair.second.commands_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  printf("point 3\n");
  struct Bytecode* bytecode = (struct Bytecode*)malloc(
      table->pair.second.commands_size * sizeof(struct Bytecode));
  printf("size: %zu", table->pair.second.commands_size);
  if (bytecode == NULL) EXIT_VM("AddFunction(void*)", "malloc failed.");
  AddFreePtr(bytecode);
  printf("point 4\n");

  table->pair.second.commands = bytecode;

  for (size_t i = 0; i < table->pair.second.commands_size; i++) {
    bytecode[i].operator= *(uint8_t*) location;
    location = (void*)((uintptr_t)location + 1);
    printf("operator: 0x%02x\n", bytecode[i].operator);
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
        bytecode[i].args = (size_t*)malloc(2 * sizeof(size_t));
        location =
            Get2Parament(location, bytecode[i].args, bytecode[i].args + 1);
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

      case OPERATOR_SAR:
        bytecode[i].args = (size_t*)malloc(3 * sizeof(size_t));
        location = Get3Parament(location, bytecode[i].args,
                                bytecode[i].args + 1, bytecode[i].args + 2);
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

      case OPERATOR_THROW:
        bytecode[i].args = NULL;
        break;

      case OPERATOR_WIDE:
        bytecode[i].args = NULL;
        break;

      default:
        EXIT_VM("AddFunction(void*)", "Invalid operator.");
    }
    AddFreePtr(bytecode[i].args);
  }

  table->next = (struct FuncList*)malloc(sizeof(struct FuncList));
  AddFreePtr(table->next);

  return location;
}

FuncInfo GetCustomFunction(const char* name) {
  const unsigned int name_hash = hash(name);
  const struct FuncList* table = &func_table[name_hash];
  while (table != NULL && table->pair.first != NULL) {
    if (strcmp(table->pair.first, name) == 0) {
      return table->pair.second;
    }
    table = table->next;
  }

  EXIT_VM("GetCustomFunction(const char*)", "Function not found.");
  return (FuncInfo){NULL, NULL, 0, NULL};
}

func_ptr GetFunction(const char* name) {
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
int THROW();
int WIDE();
int InvokeCustomFunction(const char* name) {
  printf("InvokeCustomFunction: %s, ", name);
  FuncInfo func_info = GetCustomFunction(name);
  struct Bytecode* run_code = func_info.commands;
  for (size_t i = 0; i < func_info.commands_size; i++) {
    printf("run index: %zu, run operator: 0x%02x\n", i, run_code[i].operator);
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
        NEW(run_code[i].args[0], run_code[i].args[1]);
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
        SAR(run_code[i].args[0], run_code[i].args[1], run_code[i].args[2]);
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
        THROW();
        break;
      case 0xFF:
        WIDE();
        break;
      default:
        EXIT_VM("InvokeCustomFunction(const char*)", "Invalid operator.");
        break;
    }
  }

  return 0;
}

int main(int argc, char* argv[]) {
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

  memory = (struct Memory*)malloc(sizeof(struct Memory));
  memory->size = is_big_endian ? *(uint64_t*)bytecode_file
                               : SwapUint64t(*(uint64_t*)bytecode_file);
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
  memory->data = bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + memory->size);
  memory->type = bytecode_file;
  if (memory->size % 2 != 0) {
    bytecode_file = (void*)((uintptr_t)bytecode_file + memory->size / 2 + 1);
  } else {
    bytecode_file = (void*)((uintptr_t)bytecode_file + memory->size / 2);
  }

  while (bytecode_file < bytecode_end) {
    printf("bytecode_file: %p\n", bytecode_file);
    printf("bytecode_end: %p\n", bytecode_end);
    printf("offset: %zu\n", (uintptr_t)bytecode_end - (uintptr_t)bytecode_file);
    bytecode_file = AddFunction(bytecode_file);
  }

  free_list = NULL;

  InitializeNameTable(name_table);
  printf("\nProgram started.\n");

  InvokeCustomFunction("main");

  printf("\nProgram finished\n");
  FreeAllPtr();
  FreeMemory(memory);
  free(bytecode_begin);

  return 0;
}
