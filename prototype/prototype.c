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
  size_t memory_size;
  void* memory;
  void* types;
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

struct Memory* memory;

struct LinkedList name_table[1024];

struct FuncList func_table[1024];

bool is_big_endian;

#define GET_SIZE(x)  \
  ((x) == 0x00   ? 0 \
   : (x) == 0x01 ? 1 \
   : (x) == 0x02 ? 4 \
   : (x) == 0x03 ? 8 \
   : (x) == 0x04 ? 4 \
   : (x) == 0x02 ? 8 \
                 : 0)

/*typedef struct {
  void* ptr;
  uint8_t type;
} Ptr;*/

void IsBigEndian() {
  uint16_t test_data = 0x0011;
  is_big_endian = (*(uint8_t*)&test_data == 0x00);
}

/*int16_t Swap16(int16_t x) {
    uint16_t ux = (uint16_t)x;
    ux = (ux << 8) | (ux >> 8);
    return (int16_t)ux;
}*/

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

int WriteData(const struct Memory* memory, const size_t index, const void* data_ptr,
              const size_t size) {
  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);

  return 0;
}

uint8_t GetType(const struct Memory* memory, size_t index) {
  if (index % 2 != 0) {
    // fprintf(stderr, "index: %zu\n", *(memory->type + (index / 2)) & 0x0F);
    return *(memory->type + (index / 2)) & 0x0F;
  } else {
    // fprintf(stderr, "index: %zu\n", (*(memory->type + (index / 2)) & 0xF0) >>
    // 4); //fprintf(stderr, "index: %zu\n", *(memory->type + (index / 2)));
    return (*(memory->type + (index / 2)) & 0xF0) >> 4;
  }
}

void* GetPtrData(size_t index) {
  switch (GetType(memory, index)) {
    /*case 0x01:
      return (void*)(*(int8_t*)((uintptr_t)memory->data + index));
    case 0x02:
      return (void*)(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return (void*)(*(long*)((uintptr_t)memory->data + index));*/
    default:
      return *(void**)((uintptr_t)memory->data + index);
  }
}

int8_t GetByteData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return *(int8_t*)((uintptr_t)memory->data + index);
    case 0x02:
      return *(int*)((uintptr_t)memory->data + index);
    case 0x03:
      return *(long*)((uintptr_t)memory->data + index);
    case 0x04:
      return *(float*)((uintptr_t)memory->data + index);
    case 0x05:
      return *(double*)((uintptr_t)memory->data + index);
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
    default:
      return 0;
  }
}

void SetPtrData(size_t index, void* ptr) {
  switch (GetType(memory, index)) {
    /*case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = ptr;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) = ptr;
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) = ptr;
      break;*/
    default:
      *(void**)((uintptr_t)memory->data + index) = ptr;
  }
}

void SetByteData(size_t index, int8_t value) {
  switch (GetType(memory, index)) {
    case 0x01:
      *(int8_t*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x02:
      *(int*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x03:
      *(long*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x04:
      *(float*)((uintptr_t)memory->data + index) = value;
      break;
    case 0x05:
      *(double*)((uintptr_t)memory->data + index) = value;
      break;
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
    default:
      break;
  }
}

void* Get1Parament(void* ptr, size_t* first) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *first = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

void* Get2Parament(void* ptr, size_t* first, size_t* second) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *first = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *second = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

void* Get3Parament(void* ptr, size_t* first, size_t* second, size_t* third) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *first = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *second = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *third = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

void* Get4Parament(void* ptr, size_t* first, size_t* second, size_t* third,
                   size_t* fourth) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *first = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *second = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *third = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *fourth = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

int INVOKE(size_t* func, size_t return_value, InternalObject args);

void* GetUnknownCountParamentAndINVOKE(void* ptr, size_t* return_value,
                                       size_t* arg_count) {
  int state = 0;
  int size = 0;
  size_t func;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      func = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }

  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *return_value = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }

  state = 0;
  size = 0;
  while (state == 0) {
    if (*(uint8_t*)ptr < 255) {
      *arg_count = 255 * size + *(uint8_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }

  InternalObject args_obj = {*arg_count, NULL};

  size_t* args = malloc(*arg_count * sizeof(size_t));

  size_t read_arg = 0;
  while (read_arg < *arg_count) {
    state = 0;
    size = 0;
    while (state == 0) {
      if (*(uint8_t*)ptr < 255) {
        *(args + read_arg) = 255 * size + *(uint8_t*)ptr;
        state = 1;
      }
      ptr = (void*)((uintptr_t)ptr + 1);
      ++size;
    }
    read_arg++;
  }

  args_obj.index = args;

  INVOKE(&func, *return_value, args_obj);

  free(args);

  return ptr;
}

int NOP() { return 0; }
int LOAD(size_t ptr, size_t operand) {
  WriteData(memory, operand, (void*)((uintptr_t)memory->data + ptr),
            GET_SIZE(GetType(memory, operand)));
  return 0;
}
int STORE(size_t ptr, size_t operand) {
  memcpy(*((void**)((uintptr_t)memory->data + ptr)),
         (void*)((uintptr_t)memory->data + operand),
         GET_SIZE(GetType(memory, operand)));
  return 0;
}
int NEW(size_t ptr, size_t size) {
  size_t size_value = GetLongData(size);
  void* data = malloc(size_value);
  WriteData(memory, ptr, &data, sizeof(data));
  return 0;
}
int FREE(size_t ptr) {
  void* free_ptr;
  switch (GetType(memory, ptr)) {
    /*case 0x01:
      free_ptr = (void*)(*(int8_t*)((uintptr_t)memory->data + ptr));
      break;
    case 0x02:
      free_ptr = (void*)(*(int*)((uintptr_t)memory->data + ptr));
      break;
    case 0x03:
      free_ptr = (void*)(*(long*)((uintptr_t)memory->data + ptr));
      break;*/
    default:
      free_ptr = *(void**)((uintptr_t)memory->data + ptr);
      break;
  }
  free(free_ptr);
  return 0;
}
int PTR(size_t index, size_t ptr) {
  SetPtrData(ptr, (void*)((uintptr_t)memory->data + index));
  return 0;
}
int ADD(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x05 || GetType(memory, operand1) == 0x05 ||
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
      /*case 0x05:
        SetDoubleData(result,
            GetFloatData(operand1) + GetFloatData(operand2));
        break;*/
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
      /*case 0x04:
        SetFloatData(result,
            GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
            GetLongData(operand1) + GetLongData(operand2));
        break;*/
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
      /*case 0x03:
        SetLongData(result,
            GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x04:
        SetFloatData(result,
            GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
            GetIntData(operand1) + GetIntData(operand2));
        break;*/
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
      /*case 0x02:
        SetIntData(result,
            GetByteData(operand1) + GetByteData(operand2));
        break;
      case 0x03:
        SetLongData(result,
            GetByteData(operand1) + GetByteData(operand2));
        break;
      case 0x04:
        SetFloatData(result,
            GetByteData(operand1) + GetByteData(operand2));
        break;
      case 0x05:
        SetDoubleData(result,
            GetByteData(operand1) + GetByteData(operand2));
        break;*/
      default:
        break;
    }
  } else {
  }
  return 0;
}
int SUB(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x05 || GetType(memory, operand1) == 0x05 ||
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
  if (GetType(memory, result) == 0x05 || GetType(memory, operand1) == 0x05 ||
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
  if (GetType(memory, result) == 0x05 || GetType(memory, operand1) == 0x05 ||
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
  if (GetType(memory, result) == 0x03 || GetType(memory, operand1) == 0x03 ||
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
  if (GetType(memory, result) == 0x03 || GetType(memory, operand1) == 0x03 ||
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
  if (GetType(memory, result) == 0x03 || GetType(memory, operand1) == 0x03 ||
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
  if (GetType(memory, result) == 0x03 || GetType(memory, operand1) == 0x03 ||
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
void* IF(void* ptr, size_t condition, size_t true_branche,
         size_t false_branche) {
  if (GetByteData(condition) != 0) {
    return (void*)((uintptr_t)ptr + GetLongData(true_branche));
  } else {
    return (void*)((uintptr_t)ptr + GetLongData(false_branche));
  }
}
int AND(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x03 || GetType(memory, operand1) == 0x03 ||
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
  if (GetType(memory, result) == 0x03 || GetType(memory, operand1) == 0x03 ||
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
  if (GetType(memory, result) == 0x03 || GetType(memory, operand1) == 0x03 ||
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
      if (GetType(memory, result) == 0x05 ||
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
      if (GetType(memory, result) == 0x05 ||
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
      if (GetType(memory, result) == 0x05 ||
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
      if (GetType(memory, result) == 0x05 ||
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
      if (GetType(memory, result) == 0x05 ||
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
      if (GetType(memory, result) == 0x05 ||
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
int INVOKE(const size_t* func, const size_t return_value, const InternalObject args) {
  func_ptr invoke_func = GetFunction((char*)GetPtrData(*func));
  invoke_func(args, return_value);
  return 0;
}
int RETURN() { return 0; }
void* GOTO(void* ptr, size_t offset) {
  return (void*)((uintptr_t)ptr + GetLongData(offset));
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
  table->next->next = NULL;
  table->next->pair.first = NULL;
  table->next->pair.second = NULL;
}

void AddFunction(void* location) {
  size_t commands_size = *(size_t*)location;
  struct FuncList* table = &func_table[hash(location)];
  while (table->next != NULL) {
    table = table->next;
  }
  commands_size -= 8;
  location = (void*)(uintptr_t)location + 8;
  table->pair.first = location;
  table->pair.second.name = location;
  table->pair.second.location = location;
  while (*(char*)location != '\0') {
    (uintptr_t) location++;
    commands_size--;
  }
  (uintptr_t) location++;
  commands_size--;
  table->pair.second.memory_size = *(size_t*)location;
  commands_size -= 8;
  location = (void*)(uintptr_t)location + 8;
  table->pair.second.memory = location;
  commands_size -= table->pair.second.memory_size;
  location = (void*)(uintptr_t)location + table->pair.second.memory_size;
  table->pair.second.types = location;
  if (table->pair.second.memory_size % 2 != 0) {
    commands_size -= table->pair.second.memory_size / 2 + 1;
    location =
        (void*)(uintptr_t)location + table->pair.second.memory_size / 2 + 1;
  } else {
    commands_size -= table->pair.second.memory_size / 2;
    location = (void*)(uintptr_t)location + table->pair.second.memory_size / 2;
  }
  table->pair.second.commands = location;
  table->pair.second.commands_size = commands_size;
  table->next = (struct FuncList*)malloc(sizeof(struct FuncList));
}

FuncInfo GetCustomFunction(const char* name) {
  const unsigned int name_hash = hash(name);
  const struct FuncList* table = &func_table[name_hash];
  while (table != NULL) {
    if (strcmp(table->pair.first, name) == 0) {
      return table->pair.second;
    }
    table = table->next;
  }
  return FuncInfo{NULL,NULL,0,NULL,NULL,0,NULL};
}

func_ptr GetFunction(const char* name) {
  const unsigned int name_hash = hash(name);
  const struct LinkedList* table = &name_table[name_hash];
  while (table != NULL) {
    if (strcmp(table->pair.first, name) == 0) {
      return table->pair.second;
    }
    table = table->next;
  }
  return (func_ptr)NULL;
}

void DeinitializeNameTable(const struct LinkedList* list) {
  for (int i = 0; i < 1024; i++) {
    struct LinkedList* table = list[i].next;
    struct LinkedList* next;
    while (table != NULL) {
      next = table->next;
      free(table);
      table = next;
    }
  }
}

void DeinitializeFuncTable(const struct FuncList* list) {
  for (int i = 0; i < 1024; i++) {
    struct FuncList* table = list[i].next;
    struct FuncList* next;
    while (table != NULL) {
      next = table->next;
      free(table);
      table = next;
    }
  }
}

int main(int argc, char* argv[]) {
  /*LARGE_INTEGER frequency;
  LARGE_INTEGER start, end;
  double elapsedTime;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&start);*/

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

  uint64_t temp;
  memcpy(&temp, bytecode_file, sizeof(uint64_t));
  temp = SwapUint64t(temp);
  size_t memory_size = temp;
  // fprintf(stderr, "Memory size: %zu\n", memory_size);
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
  void* data = bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + memory_size);
  void* type = bytecode_file;
  // fprintf(stderr, "0x%02x%02x%02x%02x%02x%02x%02x%02x",
  // *(int8_t*)bytecode_file, *((int8_t*)bytecode_file + 1),
  // *((int8_t*)bytecode_file + 2), *((int8_t*)bytecode_file + 3),
  // *((int8_t*)bytecode_file + 4), *((int8_t*)bytecode_file + 5),
  // *((int8_t*)bytecode_file + 6), *((int8_t*)bytecode_file + 7));
  if (memory_size % 2 != 0) {
    bytecode_file = (void*)((uintptr_t)bytecode_file + memory_size / 2 + 1);
  } else {
    bytecode_file = (void*)((uintptr_t)bytecode_file + memory_size / 2);
  }
  memory = InitializeMemory(data, type, memory_size);
  void* run_code = bytecode_file;

  InitializeNameTable(name_table);
  printf("\nProgram started.\n");
  size_t first, second, result, operand1, operand2, opcode, arg_count,
      return_value;
  while (bytecode_file < bytecode_end) {
    // fprintf(stderr, "Current operand: %02x\n", *(uint8_t*)bytecode_file);
    switch (*(uint8_t*)bytecode_file) {
      case 0x00:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        NOP();
        break;
      case 0x01:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = Get2Parament(bytecode_file, &first, &second);
        LOAD(first, second);
        break;
      case 0x02:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = Get2Parament(bytecode_file, &first, &second);
        STORE(first, second);
        break;
      case 0x03:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = Get2Parament(bytecode_file, &first, &second);
        NEW(first, second);
        break;
      case 0x04:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = Get1Parament(bytecode_file, &first);
        FREE(first);
        break;
      case 0x05:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = Get2Parament(bytecode_file, &first, &second);
        PTR(first, second);
        break;
      case 0x06:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        ADD(result, operand1, operand2);
        break;
      case 0x07:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SUB(result, operand1, operand2);
        break;
      case 0x08:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        MUL(result, operand1, operand2);
        break;
      case 0x09:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        DIV(result, operand1, operand2);
        break;
      case 0x0A:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        REM(result, operand1, operand2);
        break;
      case 0x0B:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = Get2Parament(bytecode_file, &result, &operand1);
        NEG(result, operand1);
        break;
      case 0x0C:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SHL(result, operand1, operand2);
        break;
      case 0x0D:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SHR(result, operand1, operand2);
        break;
      case 0x0E:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SAR(result, operand1, operand2);
        break;
      case 0x0F:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        IF(run_code, result, operand1, operand2);
        break;
      case 0x10:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        AND(result, operand1, operand2);
        break;
      case 0x11:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        OR(result, operand1, operand2);
        break;
      case 0x12:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        XOR(result, operand1, operand2);
        break;
      case 0x13:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file =
            Get4Parament(bytecode_file, &result, &opcode, &operand1, &operand2);
        CMP(result, opcode, operand1, operand2);
        break;
      case 0x14:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = GetUnknownCountParamentAndINVOKE(
            bytecode_file, &return_value, &arg_count);
        break;
      case 0x15:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        RETURN();
        break;
      case 0x16:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        bytecode_file = Get1Parament(bytecode_file, &operand1);
        bytecode_file = GOTO(run_code, operand1);
        break;
      case 0x17:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        THROW();
        break;
      case 0xFF:
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        WIDE();
        break;
      default:
        break;
    }
  }

  printf("\nProgram finished\n");
  DeinitializeNameTable(name_table);
  FreeMemory(memory);
  free(bytecode_begin);

  /*QueryPerformanceCounter(&end);
  elapsedTime = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
  printf("Elapsed time: %f seconds\n", elapsedTime);*/

  return 0;
}
