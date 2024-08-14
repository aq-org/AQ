// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_HASH_TABLE_TABLE_H_
#define AQ_AQVM_BASE_HASH_TABLE_TABLE_H_

#include <stddef.h>

#include "aqvm/base/hash/table/table.h"

struct AqvmBaseHashMap_HashMap {
  size_t capacity;
  size_t size;
  struct AqvmBaseHashTable_HashTable* data;
};

struct AqvmBaseHashMap_HashMap* AqvmBaseHashMap_CreateHashMap(size_t capacity);

int AqvmBaseHashMap_DestroyHashMap(struct AqvmBaseHashMap_HashMap* hash_map);

#endif