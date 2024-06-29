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
// byte's type and the next 4 bits are used for the odd byte's type. The type
// list is in types.h.
// |data| is a pointer of type void* to the memory that stores the data.
// |size| is the size of the memory.
// NOTICE: The struct AqvmMemory_Memory only stores information of the memory.
// The memory is allocated by the bytecode function when storing the bytecode.
// The memory of |memory| and |type| is part of the bytecode memory.
struct AqvmMemory_Memory {
  uint8_t* type;
  void* data;
  size_t size;
};

// Checks the memory conditions in the system.
// Returns the warning counts.
int AqvmMemory_CheckMemoryConditions();

// Creates the struct AqvmMemory_Memory with |data|, |type|, and |size|.
// The function will allocate a struct AqvmMemory_Memory and copy |data|,
// |type|, and |size| into the struct. Returns a pointer to the struct.
struct AqvmMemory_Memory* AqvmMemory_CreateMemory(void* data, void* type,
                                                  size_t size);

// Free the memory of the |memory_ptr|. No return.
// NOTICE: The function only free the memory of the struct. The memory pointed
// to by pointers to data and type in struct is not freed. This memory is
// managed by bytecode related functions.
void AqvmMemory_FreeMemory(struct AqvmMemory_Memory* memory_ptr);

// Sets the type of the data at |index| bytes in |memory| to |type|. |type|
// should be less than 4 bits.
// Returns 0 if successful. Returns -1 if memory is NULL. Returns -2 if the
// index is out of range. Returns -3 if the type is out of range.
int AqvmMemory_SetType(const struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type);

// Gets the type of the data at |index| bytes in |memory|.
// Returns the type that is less than 4 bits (0X0F) if successful. Returns 0x10
// if the memory is NULL. Returns 0x20 if the index is out of memory range.
uint8_t AqvmMemory_GetType(struct AqvmMemory_Memory* memory, size_t index);

// Writes the data that |data_ptr| points to of size |size| to the data of at
// |index| bytes in |memory|.
// Returns 0 if successful. Returns -1 if the memory is NULL. Returns -2 if the
// index is out of memory range. Returns -3 if the data is NULL.
int AqvmMemory_WriteData(struct AqvmMemory_Memory* memory, size_t index,
                         void* data_ptr, size_t size);

#endif