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
  return malloc(size);
}

void AqvmMemory_FreeMemory(void* ptr) {
  if (ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory deallocation.
  }
  free(ptr);
}