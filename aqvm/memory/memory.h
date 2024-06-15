// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_MEMORY_H_
#define AQ_AQVM_MEMORY_MEMORY_H_

#include <stddef.h>
#include <stdint.h>

#include "aqvm/memory/types.h"

// The struct stores information about the memory.
// |type| is a pointer to an array that stores the type of each byte in the
// memory. Each byte uses 4 bits to store the type. So a uint8_t variable can
// store 2 types. Each uint8_t variable's first 4 bits are used for the even
// byte's type and the next 4 bits are used for the odd byte's type.
// |value| is a pointer of type void* to the memory.
// |size| is the size of the memory.
struct AqvmMemory_Memory {
  uint8_t* type;
  void* value;
  size_t size;
};

// Allocate memory of size |size| of size_t type parameter. Returns a void*
// pointer to the allocated memory.
void* AqvmMemory_AllocateMemory(size_t size);

// Free the memory that the void* pointer parameter |ptr| points to. No return.
void AqvmMemory_FreeMemory(void* ptr);

// Sets the type of the data at |index| bytes in |memory| to |type|. |type|
// should be less than 4 bits.
// Returns 0 if successful. Returns -1 if memory is NULL. Returns -2 if the
// index is out of range. Returns -3 if the type is out of range.
int AqvmMemory_SetType(const struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type);

// Gets the type of the data at |index| bytes in |memory|. Returns the type that
// is less than 4 bits (0X0F) if success. Returns 0x10 if the memory is NULL.
// Returns 0x20 if the index is out of memory range.
uint8_t AqvmMemory_GetType(struct AqvmMemory_Memory* memory, size_t index);

#endif