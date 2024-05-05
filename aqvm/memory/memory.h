// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_MEMORY_H_
#define AQ_AQVM_MEMORY_MEMORY_H_

#include <stddef.h>

void* AqvmMemory_MemoryAllocation(size_t size);

void AqvmMemory_FreeMemory(void* ptr);

#endif