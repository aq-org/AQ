// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_MEMORY_H_
#define AQ_AQVM_MEMORY_MEMORY_H_

#include <stddef.h>
#include <stdint.h>

#include "aqvm/memory/types.h"

// The struct stores infomation about the memory.
// |type| is a pointer to an array that stores the type of each bytes in the
// memory. Each bytes use 4 bits to store the type. So a uint8_t variable can
// store 2 types. Each uint8_t variables' first 4 bits are used by the even
// byte's type and the next 4 bits are used by the odd byte's type.
// |value| is a pointer which is void* type to the memory.
// |size| is the size of the memory.
struct AqvmMemory_Memory {
  uint8_t* type;
  void* value;
  size_t size;
};

// Create a memory with the given size. Returns the pointer to the memory.
void* AqvmMemory_MemoryAllocation(size_t size);

void AqvmMemory_FreeMemory(void* ptr);

int AqvmMemory_SetType(struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type);

uint8_t AqvmMemory_GetType(struct AqvmMemory_Memory* memory, size_t index);

#endif