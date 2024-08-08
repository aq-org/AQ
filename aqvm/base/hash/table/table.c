// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/hash/table/table.h"

#include <stddef.h>
#include <stdlib.h>

#include "aqvm/base/linked_list/linked_list.h"

int AqvmBaseHashTable_CloseHashTable(
    struct AqvmBaseHashTable_HashTable* hash_table) {
  if (hash_table == NULL) {
    // TODO
    return -1;
  }

  for (size_t i = 0; i < hash_table->capacity; i++) {
    AqvmBaseLinkedList_CloseLinkedList(&hash_table->data[i]);
  }
  free(hash_table->data);
  hash_table->data = NULL;
  hash_table->capacity = 0;
  hash_table->size = 0;
  return 0;
}

int AqvmBaseHashTable_InitializeHashTable(
    struct AqvmBaseHashTable_HashTable* hash_table, size_t capacity) {
  if (hash_table == NULL || capacity == 0) {
    // TODO
    return -1;
  }

  hash_table->capacity = capacity;
  hash_table->size = 0;
  hash_table->data = (struct AqvmBaseLinkedList_LinkedList*)malloc(
      sizeof(struct AqvmBaseLinkedList_LinkedList) * capacity);
  if (hash_table->data == NULL) {
    // TODO
    return -2;
  }
  for (size_t i = 0; i < capacity; i++) {
    if (AqvmBaseLinkedList_InitializeLinkedList(&hash_table->data[i])) {
      // TODO
      return -3;
    }
  }
  return 0;
}