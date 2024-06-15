// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/memory/memory.h"

#include <stddef.h>
#include <stdlib.h>

void* AqvmMemory_MemoryAllocation(size_t size) {
  void* ptr = malloc(size);
  if (ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory allocation.
  }
  return ptr;
}

void AqvmMemory_FreeMemory(void* ptr) {
  free(ptr);
}

int AqvmMemory_SetType(struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type) {
  if (memory == NULL || memory->type == NULL) {
    // TODO(ERROR): The memory is NULL.
    return -1;
  }

  if (index > memory->size) {
    // TODO(ERROR): Out of memory range.
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
    return -1;
  }

  if (index > memory->size) {
    // TODO(ERROR): Out of memory range.
    return -2;
  }

  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }

  return 0;
}