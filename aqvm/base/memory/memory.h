// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_MEMORY_MEMORY_H_
#define AQ_AQVM_BASE_MEMORY_MEMORY_H_

#include <stddef.h>

void* AqvmBaseMemory_AqvmBaseMemory_malloc(size_t size);

int AqvmBaseMemory_free(void* ptr);

#endif