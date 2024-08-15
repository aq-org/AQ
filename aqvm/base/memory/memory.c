// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/memory/memory.h"

#include <stddef.h>
#include <stdlib.h>

void* AqvmBaseMemory_malloc(size_t size) {
  void* ptr = malloc(size);
  if (ptr == NULL) {
    // TODO
  }
  return ptr;
}

int AqvmBaseMemory_free(void* ptr) {
  if (ptr == NULL) {
    // TODO
    return -1;
  }
  free(ptr);
  return 0;
}