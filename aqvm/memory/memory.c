// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/memory/memory.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "aqvm/memory/types.h"

int AqvmMemory_CheckMemoryConditions() {
  if (sizeof(aqint) != 4) {
    // TODO(WARNING): The length requirement for the int type does not conform
    // to the type definition.
    return -1;
  }
  if (sizeof(aqlong) != 8) {
    // TODO(WARNING): The length requirement for the long type does not conform
    // to the type definition.
    return -2;
  }
  if (sizeof(aqfloat) != 4) {
    // TODO(WARNING): The length requirement for the float type does not conform
    // to the type definition.
    return -3;
  }
  if (sizeof(aqdouble) != 4) {
    // TODO(WARNING): The length requirement for the double type does not
    // conform to the type definition.
    return -4;
  }
  if (sizeof(aqchar) != 1) {
    // TODO(WARNING): The length requirement for the char type does not conform
    // to the type definition.
    return -5;
  }
  if (sizeof(aqbool) != 1) {
    // TODO(WARNING): The length requirement for the bool type does not conform
    // to the type definition.
    return -6;
  }

  return 0;
}

struct AqvmMemory_Memory* AqvmMemory_CreateMemory(void* data, void* type,
                                                  size_t size) {
  struct AqvmMemory_Memory* memory_ptr =
      (struct AqvmMemory_Memory*)malloc(sizeof(struct AqvmMemory_Memory));
  if (memory_ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory allocation.
    return NULL;
  }

  memory_ptr->data = data;
  memory_ptr->type = type;
  memory_ptr->size = size;

  return memory_ptr;
}

void AqvmMemory_FreeMemory(struct AqvmMemory_Memory* memory_ptr) {
  free(memory_ptr);
}

int AqvmMemory_SetType(const struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type) {
  if (memory == NULL || memory->type == NULL) {
    // TODO(ERROR): The memory is NULL.
    return -1;
  }

  if (index > memory->size) {
    // TODO(ERROR): The index is out of memory range.
    return -2;
  }

  if (type > 0x0F) {
    // TODO(ERROR): The type is out of range.
    return -3;
  }

  if (index % 2 != 0) {
    memory->type[index / 2] = (memory->type[index / 2] & 0xF0) | type;
  } else {
    memory->type[index / 2] = (memory->type[index / 2] & 0x0F) | (type << 4);
  }

  return 0;
}

uint8_t AqvmMemory_GetType(struct AqvmMemory_Memory* memory, size_t index) {
  if (memory == NULL || memory->type == NULL) {
    // TODO(ERROR): The memory is NULL.
    return 0x10;
  }

  if (index > memory->size) {
    // TODO(ERROR): The index is out of memory range.
    return 0x20;
  }

  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }
}

int AqvmMemory_WriteData(struct AqvmMemory_Memory* memory, size_t index,
                         void* data_ptr, size_t size) {
  if (memory == NULL || memory->data == NULL) {
    // TODO(ERROR): The memory is NULL.
    return -1;
  }
  if (index + size > memory->size) {
    // TODO(ERROR): The index is out of memory range.
    return -2;
  }
  if (data_ptr == NULL) {
    // TODO(ERROR): The data is NULL.
    return -3;
  }

  memcpy(memory->data + index, data_ptr, size);
  return 0;
}