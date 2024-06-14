// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_MEMORY_H_
#define AQ_AQVM_MEMORY_MEMORY_H_

#include <stddef.h>
#include <stdint.h>

#include "aqvm/memory/types.h"

struct AqvmMemory_Memory {
  uint8_t* type;
  void* value;
  size_t size;
};

void* AqvmMemory_MemoryAllocation(size_t size);

void AqvmMemory_FreeMemory(void* ptr);

int AqvmMemory_SetType(struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type);

uint8_t AqvmMemory_GetType(struct AqvmMemory_Memory* memory, size_t index);

#endif