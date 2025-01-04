// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

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

typedef struct {
  const char* name;
  void* location;
  size_t commands_size;
  void* commands;
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

void AddFreePtr(void* ptr) {
  struct FreeList* new_free_list =
      (struct FreeList*)malloc(sizeof(struct FreeList));
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

  memory_ptr->data = data;
  memory_ptr->type = type;
  memory_ptr->size = size;

  return memory_ptr;
}

void FreeMemory(struct Memory* memory_ptr) { free(memory_ptr); }

int SetType(const struct Memory* memory, size_t index, uint8_t type) {
  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }
}

int WriteData(const struct Memory* memory, const size_t index,
              const void* data_ptr, const size_t size) {
  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);

  return 0;
}

uint8_t GetType(const struct Memory* memory, size_t index) {
  if (index % 2 != 0) {
    return *(memory->type + (index / 2)) & 0x0F;
  } else {
    return (*(memory->type + (index / 2)) & 0xF0) >> 4;
  }
}

void* GetPtrData(size_t index) {
  return *(void**)((uintptr_t)memory->data + index);
}

int8_t GetByteData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

int GetIntData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

long GetLongData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

float GetFloatData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

double GetDoubleData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

uint64_t GetUint64tData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      return is_big_endian ? *(int*)((uintptr_t)memory->data + index)
                           : SwapInt(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return is_big_endian
                 ? *(long*)((uintptr_t)memory->data + index)
                 : SwapLong(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return is_big_endian
                 ? *(float*)((uintptr_t)memory->data + index)
                 : SwapFloat(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return is_big_endian
                 ? *(double*)((uintptr_t)memory->data + index)
                 : SwapDouble(*(double*)((uintptr_t)memory->data + index));
    case 0x06:
      return is_big_endian
                 ? *(uint64_t*)((uintptr_t)memory->data + index)
                 : SwapUint64t(*(uint64_t*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

void SetPtrData(size_t index, void* ptr) {
  *(void**)((uintptr_t)memory->data + index) = ptr;
}

void SetByteData(size_t index, int8_t value) {
  switch (GetType(memory, index)) {
    case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      break;
  }
}

void SetIntData(size_t index, int value) {
  switch (GetType(memory, index)) {
    case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      break;
  }
}

void SetLongData(size_t index, long value) {
  switch (GetType(memory, index)) {
    case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      break;
  }
}

void SetFloatData(size_t index, float value) {
  switch (GetType(memory, index)) {
    case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      break;
  }
}

void SetDoubleData(size_t index, double value) {
  switch (GetType(memory, index)) {
    case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      break;
  }
}

void SetUint64tData(size_t index, uint64_t value) {
  switch (GetType(memory, index)) {
    case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapInt(value);
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapLong(value);
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapFloat(value);
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapDouble(value);
      break;
    case 0x06:
      *(uint64_t*)((uintptr_t)memory->data + index) =
          is_big_endian ? value : SwapUint64t(value);
    default:
      break;
  }
}

size_t DecodeUleb128(const uint8_t* input, size_t* result) {
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

int INVOKE(const size_t* func, const size_t return_value,
           const InternalObject args);

void* GetUnknownCountParamentAndINVOKE(void* ptr, size_t* return_value,
                                       size_t* arg_count) {
  size_t func = 0;
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, &func));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, arg_count));
  ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, return_value));
  arg_count--;
  InternalObject args_obj = {*arg_count, NULL};

  size_t* args = malloc(*arg_count * sizeof(size_t));

  size_t read_arg = 0;
  while (read_arg < *arg_count) {
    ptr = (void*)((uintptr_t)ptr + DecodeUleb128(ptr, args + read_arg));
    read_arg++;
  }

  args_obj.index = args;

  INVOKE(&func, *return_value, args_obj);

  free(args);

  return ptr;
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
      break;
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
      break;
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
        break;
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x02:
        SetIntData(result, GetIntData(operand1) + GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) + GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
  }
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
        break;
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
        break;
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) - GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) * GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        SetFloatData(result,
                     GetUint64tData(operand1) / GetUint64tData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
                      GetUint64tData(operand1) / GetUint64tData(operand2));
        break;
      case 0x06:
        SetUint64tData(result,
                       GetUint64tData(operand1) / GetUint64tData(operand2));
      default:
        break;
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
        break;
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) / GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) % GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, -GetByteData(operand1));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) << GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) >> GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) >> GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
  }
  return 0;
}
int InvokeCustomFunction(const char* name);
void IF(size_t condition, size_t true_branche, size_t false_branche) {
  if (GetByteData(condition) != 0) {
    InvokeCustomFunction((char*)GetPtrData(true_branche));
  } else {
    InvokeCustomFunction((char*)GetPtrData(false_branche));
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) & GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) | GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
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
        break;
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
        break;
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
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        SetByteData(result, GetByteData(operand1) ^ GetByteData(operand2));
        break;
      default:
        break;
    }
  } else {
  }
  return 0;
}
int CMP(size_t result, size_t opcode, size_t operand1, size_t operand2) {
  switch (GetByteData(opcode)) {
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) == GetByteData(operand2));
            break;
          default:
            break;
        }
      } else {
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) != GetByteData(operand2));
            break;
          default:
            break;
        }
      } else {
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) > GetByteData(operand2));
            break;
          default:
            break;
        }
      } else {
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) >= GetByteData(operand2));
            break;
          default:
            break;
        }
      } else {
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) < GetByteData(operand2));
            break;
          default:
            break;
        }
      } else {
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
            break;
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
            break;
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
            break;
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
            break;
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
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            SetByteData(result, GetByteData(operand1) <= GetByteData(operand2));
            break;
          default:
            break;
        }
      } else {
      }
      break;
    default:
      break;
  }
  return 0;
}
int INVOKE(const size_t* func, const size_t return_value,
           const InternalObject args) {
  func_ptr invoke_func = GetFunction((char*)GetPtrData(*func));
  if (invoke_func != NULL) {
    invoke_func(args, return_value);
    return 0;
  }

  return InvokeCustomFunction((char*)GetPtrData(*func));
}
int RETURN() { return 0; }
FuncInfo GOTO(size_t label) {
  return GetCustomFunction((char*)GetPtrData(label));
}
int THROW() { return 0; }
int WIDE() { return 0; }

void print(InternalObject args, size_t return_value) {
  SetIntData(return_value, printf((char*)GetPtrData(*args.index)));
}

unsigned int hash(const char* str) {
  unsigned long hash = 5381;
  int c;
  while (c = *str++) {
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
  void* origin_location = location;
  size_t all_size = SwapUint64t(*(uint64_t*)location);
  size_t commands_size = all_size - 8;
  location = (void*)((uintptr_t)location + 8);
  struct FuncList* table = &func_table[hash(location)];
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.second.location = origin_location;
  table->pair.first = location;
  table->pair.second.name = location;
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
    commands_size--;
  }
  location = (void*)((uintptr_t)location + 1);
  commands_size--;
  /*table->pair.second.memory_size = SwapUint64t(*(uint64_t*)location);
  commands_size -= 8;
  location = (void*)((uintptr_t)location + 8);
  table->pair.second.memory = location;
  commands_size -= table->pair.second.memory_size;
  location = (void*)((uintptr_t)location + table->pair.second.memory_size);
  table->pair.second.types = location;
  if (table->pair.second.memory_size % 2 != 0) {
    commands_size -= table->pair.second.memory_size / 2 + 1;
    location =
        (void*)((uintptr_t)location + table->pair.second.memory_size / 2 + 1);
  } else {
    commands_size -= table->pair.second.memory_size / 2;
    location =
        (void*)((uintptr_t)location + table->pair.second.memory_size / 2);
  }*/
  table->pair.second.commands = location;
  table->pair.second.commands_size = commands_size;
  table->next = (struct FuncList*)malloc(sizeof(struct FuncList));
  AddFreePtr(table->next);
  return (void*)((uintptr_t)location + all_size);
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
  return (FuncInfo){NULL, NULL, 0, NULL, NULL, 0, NULL};
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

int RETURN();
FuncInfo GOTO(size_t offset);
int THROW();
int WIDE();
int InvokeCustomFunction(const char* name) {
  FuncInfo func_info = GetCustomFunction(name);
  /*void* temp_memory = malloc(func_info.memory_size);
  void* temp_types = NULL;
  if (func_info.memory_size % 2 != 0) {
    temp_types = malloc(func_info.memory_size / 2 + 1);
  } else {
    temp_types = malloc(func_info.memory_size / 2);
  }
  if (temp_memory == NULL || temp_types == NULL) {
    return -1;
  }
  memcpy(temp_memory, func_info.memory, func_info.memory_size);
  if (func_info.memory_size % 2 != 0) {
    memcpy(temp_types, func_info.types, func_info.memory_size / 2 + 1);
  } else {
    memcpy(temp_types, func_info.types, func_info.memory_size / 2);
  }
  struct Memory* memory_info =
      InitializeMemory(temp_memory, temp_types, func_info.memory_size);

  struct Memory* old_memory = memory;
  memory = memory_info;
  size_t arg_index = 1;
  for (size_t i = 0; i < args.size; i++) {
    while (GetType(memory, arg_index) != 0x00) arg_index++;
    int8_t byte_data;
    int int_data;
    long long_data;
    float float_data;
    double double_data;
    uint64_t uint64t_data;
    void* ptr_data;
    switch (GetType(memory, arg_index)) {
      case 0x01:
        memory = old_memory;
        byte_data = GetByteData(args.index[i]);
        memory = memory_info;
        SetByteData(arg_index, byte_data);
        break;
      case 0x02:
        memory = old_memory;
        int_data = GetIntData(args.index[i]);
        memory = memory_info;
        SetIntData(arg_index, int_data);
        break;
      case 0x03:
        memory = old_memory;
        long_data = GetLongData(args.index[i]);
        memory = memory_info;
        SetLongData(arg_index, long_data);
        break;
      case 0x04:
        memory = old_memory;
        float_data = GetFloatData(args.index[i]);
        memory = memory_info;
        SetFloatData(arg_index, float_data);
        break;
      case 0x05:
        memory = old_memory;
        double_data = GetDoubleData(args.index[i]);
        memory = memory_info;
        SetDoubleData(arg_index, double_data);
        break;
      case 0x06:
        memory = old_memory;
        uint64t_data = GetUint64tData(args.index[i]);
        memory = memory_info;
        SetUint64tData(arg_index, uint64t_data);
        break;
      case 0x0F:
        memory = old_memory;
        ptr_data = GetPtrData(args.index[i]);
        memory = memory_info;
        SetPtrData(arg_index, ptr_data);
        break;
      default:
        break;
    }
  }*/

  void* run_code = func_info.commands;
  size_t first, second, result, operand1, operand2, opcode, arg_count,
      returnvalue;
  while (run_code <
         (void*)((uintptr_t)func_info.commands + func_info.commands_size)) {
    switch (*(uint8_t*)run_code) {
      case 0x00:
        run_code = (void*)((uintptr_t)run_code + 1);
        NOP();
        break;
      case 0x01:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get2Parament(run_code, &first, &second);
        LOAD(first, second);
        break;
      case 0x02:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get2Parament(run_code, &first, &second);
        STORE(first, second);
        break;
      case 0x03:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get2Parament(run_code, &first, &second);
        NEW(first, second);
        break;
      case 0x04:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get1Parament(run_code, &first);
        FREE(first);
        break;
      case 0x05:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get2Parament(run_code, &first, &second);
        PTR(first, second);
        break;
      case 0x06:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        ADD(result, operand1, operand2);
        break;
      case 0x07:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        SUB(result, operand1, operand2);
        break;
      case 0x08:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        MUL(result, operand1, operand2);
        break;
      case 0x09:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        DIV(result, operand1, operand2);
        break;
      case 0x0A:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        REM(result, operand1, operand2);
        break;
      case 0x0B:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get2Parament(run_code, &result, &operand1);
        NEG(result, operand1);
        break;
      case 0x0C:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        SHL(result, operand1, operand2);
        break;
      case 0x0D:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        SHR(result, operand1, operand2);
        break;
      case 0x0E:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        SAR(result, operand1, operand2);
        break;
      case 0x0F:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        IF(result, operand1, operand2);
        break;
      case 0x10:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        AND(result, operand1, operand2);
        break;
      case 0x11:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        OR(result, operand1, operand2);
        break;
      case 0x12:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get3Parament(run_code, &result, &operand1, &operand2);
        XOR(result, operand1, operand2);
        break;
      case 0x13:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code =
            Get4Parament(run_code, &result, &opcode, &operand1, &operand2);
        CMP(result, opcode, operand1, operand2);
        break;
      case 0x14:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = GetUnknownCountParamentAndINVOKE(run_code, &returnvalue,
                                                    &arg_count);
        // memory = memory_info;
        break;
      case 0x15:
        run_code = (void*)((uintptr_t)run_code + 1);
        RETURN();
        break;
      case 0x16:
        run_code = (void*)((uintptr_t)run_code + 1);
        run_code = Get1Parament(run_code, &operand1);
        func_info = GOTO(operand1);
        run_code = func_info.commands;
        break;
      case 0x17:
        run_code = (void*)((uintptr_t)run_code + 1);
        THROW();
        break;
      case 0xFF:
        run_code = (void*)((uintptr_t)run_code + 1);
        WIDE();
        break;
      default:
        break;
    }
  }
  /*if (GetType(memory, return_value) != 0x00) {
    int8_t bytedata;
    int intdata;
    long longdata;
    float floatdata;
    double doubledata;
    uint64_t uint64tdata;
    void* ptrdata;
    switch (GetType(memory, return_value)) {
      case 0x01:
        bytedata = GetByteData(return_value);
        memory = old_memory;
        SetByteData(return_value, bytedata);
        memory = memory_info;
        break;
      case 0x02:
        intdata = GetIntData(return_value);
        memory = old_memory;
        SetIntData(return_value, intdata);
        memory = memory_info;
        break;
      case 0x03:
        longdata = GetLongData(return_value);
        memory = old_memory;
        SetLongData(return_value, longdata);
        memory = memory_info;
        break;
      case 0x04:
        floatdata = GetFloatData(return_value);
        memory = old_memory;
        SetFloatData(return_value, floatdata);
        memory = memory_info;
        break;
      case 0x05:
        doubledata = GetDoubleData(return_value);
        memory = old_memory;
        SetDoubleData(return_value, doubledata);
        memory = memory_info;
        break;
      case 0x06:
        uint64tdata = GetUint64tData(return_value);
        memory = old_memory;
        SetUint64tData(return_value, uint64tdata);
        memory = memory_info;
        break;
      case 0x0F:
        ptrdata = GetPtrData(return_value);
        memory = old_memory;
        SetPtrData(return_value, ptrdata);
        memory = memory_info;
        break;
      default:
        break;
    }
  }
  FreeMemory(memory_info);
  free(temp_types);
  free(temp_memory);
  memory = NULL;*/
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  FILE* bytecode = fopen(argv[1], "rb");
  if (bytecode == NULL) {
    printf("Error: Could not open file %s\n", argv[1]);
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
    return -3;
  }

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  while (bytecode_file < bytecode_end) {
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
