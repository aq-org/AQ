// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_HASH_TABLE_TABLE_H_
#define AQ_AQVM_BASE_HASH_TABLE_TABLE_H_

#include <stddef.h>

#include "aqvm/base/linked_list/linked_list.h"

struct AqvmBaseHashTable_HashTable {
  size_t capacity;
  size_t size;
  struct AqvmBaseLinkedList_LinkedList* data;
};

int AqvmBaseHashTable_CloseHashTable(
    struct AqvmBaseHashTable_HashTable* hash_table);

int AqvmBaseHashTable_InitializeHashTable(
    struct AqvmBaseHashTable_HashTable* hash_table, size_t capacity);

#endif