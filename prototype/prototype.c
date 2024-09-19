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
} Object;

typedef void (*func_ptr)(Object, Object);

struct Pair {
  char* first;
  func_ptr second;
  uint8_t type;
};

struct LinkedList {
  struct Pair pair;
  struct LinkedList* next;
};

struct Memory {
  uint8_t* type;
  void* data;
  size_t size;
};

func_ptr GetFunction(const char* name);

struct Memory* memory;

struct LinkedList name_table[1024];

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

int WriteData(struct Memory* memory, size_t index, void* data_ptr,
              size_t size) {
  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);

  return 0;
}

uint8_t GetType(const struct Memory* memory, size_t index) {
  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
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
      return (int8_t)(*(int8_t*)((uintptr_t)memory->data + index));
    case 0x02:
      return (int8_t)(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return (int8_t)(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return (int8_t)(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return (int8_t)(*(double*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

int GetIntData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return (int)(*(int8_t*)((uintptr_t)memory->data + index));
    case 0x02:
      return (int)(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return (int)(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return (int)(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return (int)(*(double*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

long GetLongData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return (long)(*(int8_t*)((uintptr_t)memory->data + index));
    case 0x02:
      return (long)(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return (long)(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return (long)(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return (long)(*(double*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

float GetFloatData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return (float)(*(int8_t*)((uintptr_t)memory->data + index));
    case 0x02:
      return (float)(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return (float)(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return (float)(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return (float)(*(double*)((uintptr_t)memory->data + index));
    default:
      return 0;
  }
}

double GetDoubleData(size_t index) {
  switch (GetType(memory, index)) {
    case 0x01:
      return (double)(*(int8_t*)((uintptr_t)memory->data + index));
    case 0x02:
      return (double)(*(int*)((uintptr_t)memory->data + index));
    case 0x03:
      return (double)(*(long*)((uintptr_t)memory->data + index));
    case 0x04:
      return (double)(*(float*)((uintptr_t)memory->data + index));
    case 0x05:
      return (double)(*(double*)((uintptr_t)memory->data + index));
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

void SetLongData(size_t index, long value) {
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

void SetFloatData(size_t index, float value) {
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

void SetDoubleData(size_t index, double value) {
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

void* Get1Parament(void* ptr, size_t* first) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
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
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *second = 255 * size + *(size_t*)ptr;
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
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *second = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *third = 255 * size + *(size_t*)ptr;
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
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *second = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *third = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *fourth = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

int INVOKE(size_t* func, Object return_value, Object args);

void* GetUnknownCountParamentAndINVOKE(void* ptr, size_t* return_value,
                                       size_t* arg_count) {
  int state = 0;
  int size = 0;
  size_t func;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      func = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }

  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *return_value = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }

  Object return_obj = {1, return_value};

  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *arg_count = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }

  size_t arg_count_num = GetLongData(*arg_count);

  Object args_obj = {arg_count_num, NULL};

  size_t* args = malloc(arg_count_num * sizeof(size_t));

  size_t read_arg = 0;
  while (read_arg < arg_count_num) {
    state = 0;
    size = 0;
    while (state == 0) {
      if (*(size_t*)ptr < 255) {
        *(args + read_arg) = 255 * size + *(size_t*)ptr;
        state = 1;
      }
      ptr = (void*)((uintptr_t)ptr + 1);
      ++size;
    }
  }

  args_obj.index = args;

  INVOKE(&func, return_obj, args_obj);

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
  size_t size_value = 0;
  switch (GetType(memory, size)) {
    case 0x01:
      size_value = (size_t)(*(int8_t*)((uintptr_t)memory->data + size));
      break;
    case 0x02:
      size_value = (size_t)(*(int*)((uintptr_t)memory->data + size));
      break;
    case 0x03:
      size_value = (size_t)(*(long*)((uintptr_t)memory->data + size));
      break;
    case 0x04:
      size_value = (size_t)(*(float*)((uintptr_t)memory->data + size));
      break;
    case 0x05:
      size_value = (size_t)(*(double*)((uintptr_t)memory->data + size));
      break;
    default:
      size_value = (size_t)(*(void**)((uintptr_t)memory->data + size));
      break;
  }
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
int SIZE() { return 0; }
int ADD(size_t result, size_t operand1, size_t operand2) {
  if (GetType(memory, result) == 0x05 || GetType(memory, operand1) == 0x05 ||
      GetType(memory, operand2) == 0x05) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetDoubleData(operand1) + GetDoubleData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetFloatData(operand1) + GetFloatData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetFloatData(operand1) + GetFloatData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetFloatData(operand1) + GetFloatData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetFloatData(operand1) + GetFloatData(operand2));
        break;
      /*case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetFloatData(operand1) + GetFloatData(operand2));
        break;*/
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) + GetLongData(operand2));
        break;
      /*case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetLongData(operand1) + GetLongData(operand2));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetLongData(operand1) + GetLongData(operand2));
        break;*/
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) + GetIntData(operand2));
        break;
      /*case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetIntData(operand1) + GetIntData(operand2));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetIntData(operand1) + GetIntData(operand2));
        break;*/
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) + GetByteData(operand2));
        break;
      /*case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetByteData(operand1) + GetByteData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetByteData(operand1) + GetByteData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetByteData(operand1) + GetByteData(operand2));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetByteData(operand1) + GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetDoubleData(operand1) - GetDoubleData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetFloatData(operand1) - GetFloatData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetFloatData(operand1) - GetFloatData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetFloatData(operand1) - GetFloatData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetFloatData(operand1) - GetFloatData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) - GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) - GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) - GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) - GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) - GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) - GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetDoubleData(operand1) * GetDoubleData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetFloatData(operand1) * GetFloatData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetFloatData(operand1) * GetFloatData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetFloatData(operand1) * GetFloatData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetFloatData(operand1) * GetFloatData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) * GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) * GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) * GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) * GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) * GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) * GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(GetDoubleData(operand1) / GetDoubleData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04 ||
             GetType(memory, operand2) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetFloatData(operand1) / GetFloatData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetFloatData(operand1) / GetFloatData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetFloatData(operand1) / GetFloatData(operand2));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(GetFloatData(operand1) / GetFloatData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03 ||
             GetType(memory, operand2) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) / GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) / GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) / GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) / GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) / GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) / GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) % GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) % GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) % GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) % GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) % GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) % GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(-GetDoubleData(operand1));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(-GetDoubleData(operand1));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(-GetDoubleData(operand1));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(-GetDoubleData(operand1));
        break;
      case 0x05:
        *(double*)((uintptr_t)memory->data + result) =
            (double)(-GetDoubleData(operand1));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x04 ||
             GetType(memory, operand1) == 0x04) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(-GetFloatData(operand1));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(-GetFloatData(operand1));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(-GetFloatData(operand1));
        break;
      case 0x04:
        *(float*)((uintptr_t)memory->data + result) =
            (float)(-GetFloatData(operand1));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x03 ||
             GetType(memory, operand1) == 0x03) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(-GetLongData(operand1));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(-GetLongData(operand1));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(-GetLongData(operand1));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(-GetIntData(operand1));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(-GetIntData(operand1));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(-GetByteData(operand1));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) << GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) << GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) << GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) << GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) << GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) << GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) >> GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) >> GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) >> GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) >> GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) >> GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) >> GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) >> GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) >> GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) >> GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) & GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) & GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) & GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) & GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) & GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) & GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) | GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) | GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) | GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) | GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) | GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) | GetByteData(operand2));
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
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetLongData(operand1) ^ GetLongData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetLongData(operand1) ^ GetLongData(operand2));
        break;
      case 0x03:
        *(long*)((uintptr_t)memory->data + result) =
            (long)(GetLongData(operand1) ^ GetLongData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x02 ||
             GetType(memory, operand1) == 0x02 ||
             GetType(memory, operand2) == 0x02) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetIntData(operand1) ^ GetIntData(operand2));
        break;
      case 0x02:
        *(int*)((uintptr_t)memory->data + result) =
            (int)(GetIntData(operand1) ^ GetIntData(operand2));
        break;
      default:
        break;
    }
  } else if (GetType(memory, result) == 0x01 ||
             GetType(memory, operand1) == 0x01 ||
             GetType(memory, operand2) == 0x01) {
    switch (GetType(memory, result)) {
      case 0x01:
        *(int8_t*)((uintptr_t)memory->data + result) =
            (int8_t)(GetByteData(operand1) ^ GetByteData(operand2));
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
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          case 0x05:
            *(double*)((uintptr_t)memory->data + result) =
                (double)(GetDoubleData(operand1) == GetDoubleData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetFloatData(operand1) == GetFloatData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetFloatData(operand1) == GetFloatData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetFloatData(operand1) == GetFloatData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetFloatData(operand1) == GetFloatData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetLongData(operand1) == GetLongData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetLongData(operand1) == GetLongData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetLongData(operand1) == GetLongData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetIntData(operand1) == GetIntData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetIntData(operand1) == GetIntData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetByteData(operand1) == GetByteData(operand2));
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
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          case 0x05:
            *(double*)((uintptr_t)memory->data + result) =
                (double)(GetDoubleData(operand1) != GetDoubleData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetFloatData(operand1) != GetFloatData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetFloatData(operand1) != GetFloatData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetFloatData(operand1) != GetFloatData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetFloatData(operand1) != GetFloatData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetLongData(operand1) != GetLongData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetLongData(operand1) != GetLongData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetLongData(operand1) != GetLongData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetIntData(operand1) != GetIntData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetIntData(operand1) != GetIntData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetByteData(operand1) != GetByteData(operand2));
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
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          case 0x05:
            *(double*)((uintptr_t)memory->data + result) =
                (double)(GetDoubleData(operand1) < GetDoubleData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetFloatData(operand1) < GetFloatData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetFloatData(operand1) < GetFloatData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetFloatData(operand1) < GetFloatData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetFloatData(operand1) < GetFloatData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetLongData(operand1) < GetLongData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetLongData(operand1) < GetLongData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetLongData(operand1) < GetLongData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetIntData(operand1) < GetIntData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetIntData(operand1) < GetIntData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetByteData(operand1) < GetByteData(operand2));
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
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          case 0x05:
            *(double*)((uintptr_t)memory->data + result) =
                (double)(GetDoubleData(operand1) <= GetDoubleData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetFloatData(operand1) <= GetFloatData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetLongData(operand1) <= GetLongData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetLongData(operand1) <= GetLongData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetLongData(operand1) <= GetLongData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetIntData(operand1) <= GetIntData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetIntData(operand1) <= GetIntData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetByteData(operand1) <= GetByteData(operand2));
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
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          case 0x05:
            *(double*)((uintptr_t)memory->data + result) =
                (double)(GetDoubleData(operand1) > GetDoubleData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetFloatData(operand1) > GetFloatData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetFloatData(operand1) > GetFloatData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetFloatData(operand1) > GetFloatData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetFloatData(operand1) > GetFloatData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetLongData(operand1) > GetLongData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetLongData(operand1) > GetLongData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetLongData(operand1) > GetLongData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetIntData(operand1) > GetIntData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetIntData(operand1) > GetIntData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetByteData(operand1) > GetByteData(operand2));
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
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          case 0x05:
            *(double*)((uintptr_t)memory->data + result) =
                (double)(GetDoubleData(operand1) >= GetDoubleData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x04 ||
                 GetType(memory, operand1) == 0x04 ||
                 GetType(memory, operand2) == 0x04) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          case 0x04:
            *(float*)((uintptr_t)memory->data + result) =
                (float)(GetFloatData(operand1) >= GetFloatData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x03 ||
                 GetType(memory, operand1) == 0x03 ||
                 GetType(memory, operand2) == 0x03) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetLongData(operand1) >= GetLongData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetLongData(operand1) >= GetLongData(operand2));
            break;
          case 0x03:
            *(long*)((uintptr_t)memory->data + result) =
                (long)(GetLongData(operand1) >= GetLongData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x02 ||
                 GetType(memory, operand1) == 0x02 ||
                 GetType(memory, operand2) == 0x02) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetIntData(operand1) >= GetIntData(operand2));
            break;
          case 0x02:
            *(int*)((uintptr_t)memory->data + result) =
                (int)(GetIntData(operand1) >= GetIntData(operand2));
            break;
          default:
            break;
        }
      } else if (GetType(memory, result) == 0x01 ||
                 GetType(memory, operand1) == 0x01 ||
                 GetType(memory, operand2) == 0x01) {
        switch (GetType(memory, result)) {
          case 0x01:
            *(int8_t*)((uintptr_t)memory->data + result) =
                (int8_t)(GetByteData(operand1) >= GetByteData(operand2));
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
int INVOKE(size_t* func, Object return_value, Object args) {
  func_ptr invoke_func = GetFunction((char*)((uintptr_t)memory->data + *func));
  invoke_func(args, return_value);
  return 0;
}
int RETURN() { return 0; }
void* GOTO(void* ptr, size_t offset) {
  size_t offset_num;
  if (GetType(memory, offset) == 0x03) {
    offset_num = GetLongData(offset);
  } else if (GetType(memory, offset) == 0x02) {
    offset_num = GetIntData(offset);
  } else if (GetType(memory, offset) == 0x01) {
    offset_num = GetByteData(offset);
  } else {
  }
  return (void*)((uintptr_t)ptr + offset_num);
}
int THROW() { return 0; }
int WIDE() { return 0; }

void print(Object args, Object return_value) {
  SetIntData(*return_value.index, printf((char*)GetPtrData(*args.index)));
}

unsigned int hash(const char* str) {
  unsigned long hash = 5381;
  int c;
  while (c = *str++) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

void InitializeNameTable(struct LinkedList* list) {
  unsigned int name_hash = hash("print");
  struct LinkedList* table = &name_table[name_hash];
  while (table != NULL) {
    table = table->next;
  }
  name_table[name_hash].pair.first = "print";
  name_table[name_hash].pair.type = 0x02;
  name_table[name_hash].pair.second = print;
  name_table[name_hash].next =
      (struct LinkedList*)malloc(sizeof(struct LinkedList));
}

uint8_t GetFuncType(const char* name) {
  unsigned int name_hash = hash(name);
  struct LinkedList* table = &name_table[name_hash];
  while (table != NULL) {
    if (strcmp(table->pair.first, name) == 0) {
      return table->pair.type;
    }
    table = table->next;
  }
  return 0;
}

func_ptr GetFunction(const char* name) {
  unsigned int name_hash = hash(name);
  struct LinkedList* table = &name_table[name_hash];
  while (table != NULL) {
    if (strcmp(table->pair.first, name) == 0) {
      return table->pair.second;
    }
    table = table->next;
  }
  return (func_ptr)NULL;
}

void DeinitializeNameTable(struct LinkedList* list) {
  unsigned int name_hash = hash("print");
  struct LinkedList* table = name_table[name_hash].next;
  while (table != NULL) {
    table = table->next;
    free(table);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  FILE* bytecode = fopen(argv[1], "r");
  if (bytecode == NULL) {
    printf("Error: Could not open file %s\n", argv[1]);
    return -2;
  }

  fseek(bytecode, 0, SEEK_END);
  size_t bytecode_size = ftell(bytecode);
  void* bytecode_file = malloc(bytecode_size + 1);
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

  size_t memory_size = *(size_t*)bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
  void* type = bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + memory_size / 2);
  void* data = bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + memory_size);
  memory = InitializeMemory(data, type, memory_size);
  void* run_code = bytecode_file;

  InitializeNameTable(name_table);

  size_t first, second, result, operand1, operand2, opcode, arg_count,
      return_value;
  while (bytecode_file < bytecode_end) {
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
        SIZE();
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
        GetUnknownCountParamentAndINVOKE(bytecode_file, &return_value,
                                         &arg_count);
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

  printf("Program finished");
  DeinitializeNameTable(name_table);
  FreeMemory(memory);
  free(bytecode_begin);
  return 0;
}