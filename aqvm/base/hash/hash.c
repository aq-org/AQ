// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/hash/hash.h"

#include <limits.h>
#include <stdint.h>

int AqvmBaseHash_Hash(int data) {
  if (data > INT_MAX - ((5381 << 5) + 5381) ||
      data < INT_MIN + ((5381 << 5) + 5381)) {
    // TODO
    return 0;
  }
  return ((5381 << 5) + 5381) + data;
}

uint32_t AqvmBaseHash_HashString(const char* str) {
  if (str == NULL) {
    // TODO
    return 0;
  }
  uint32_t hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

uint32_t AqvmBaseHash_HashUnsignedIntArray(const uint32_t* data, size_t size) {
  if (data == NULL) {
    // TODO
    return 0;
  }
  uint32_t hash = 5381;
  for (size_t i = 0; i < size; i++) {
    hash = ((hash << 5) + hash) + data[i];
  }
  return hash;
}