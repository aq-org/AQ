// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/file_lock/file_lock.h"

#include <stddef.h>

#include "aqvm/base/file/file.h"
#include "aqvm/base/file/identifier/identifier.h"
#include "aqvm/base/file/read_write_lock/read_write_lock.h"
#include "aqvm/base/hash/table/table.h"
#include "aqvm/base/linked_list/linked_list.h"
#include "aqvm/base/memory/memory.h"
#include "aqvm/base/pair.h"

struct AqvmBaseHashTable_HashTable AqvmBaseThreadingFileLock_fileLockTable;

int AqvmBaseThreadingFileLock_CloseFileLockTable() {
  if (AqvmBaseHashTable_CloseHashTable(
          &AqvmBaseThreadingFileLock_fileLockTable) != 0) {
    // TODO(logging)
    return -1;
  }
  return 0;
}

int AqvmBaseThreadingFileLock_InitializeFileLockTable() {
  if (AqvmBaseHashTable_InitializeHashTable(
          &AqvmBaseThreadingFileLock_fileLockTable, 1024) != 0) {
    return -1;
  }
  return 0;
}

int AqvmBaseThreadingFileLock_AddFileLock(struct AqvmBaseFile_File* file) {
  if (file == NULL || file->identifier == NULL) {
    // TODO(logging)
    return -1;
  }

  file->lock = AqvmBaseThreadingFileLock_GetFileLock(file);
  if (file->lock == NULL) {
    struct AqvmBase_Pair* file_lock_pair =
        (struct AqvmBase_Pair*)AqvmBaseMemory_malloc(
            sizeof(struct AqvmBase_Pair));
    if (file_lock_pair == NULL) {
      // TODO(logging)
      return -2;
    }
    file_lock_pair->key = file->identifier;
    file_lock_pair->value = file->lock;
    if (AqvmBaseLinkedList_AddNode(
            &AqvmBaseThreadingFileLock_fileLockTable
                 .data[AqvmBaseFileIdentifier_GetIdentifierHash(
                           file->identifier) %
                       1024],
            file_lock_pair) != 0) {
      // TODO(logging)
      AqvmBaseMemory_free(file_lock_pair);
      return -3;
    }
  }
  ++file->lock->lock_count;
  return 0;
}

int AqvmBaseThreadingFileLock_RemoveFileLock(struct AqvmBaseFile_File* file) {
  if (file == NULL || file->lock == NULL || file->lock->lock_count == 0) {
    // TODO(logging)
    return -1;
  }

  --file->lock->lock_count;
  /*if (file->lock->lock_count = 0) {
    AqvmBaseLinkedList_DeleteNode(AqvmBaseLinkedList_GetData(
        &AqvmBaseThreadingFileLock_fileLockTable.data,
        AqvmBaseFileIdentifier_GetIdentifierHash(file->identifier) % 1024));
  }*/
  return 0;
}

struct AqvmBaseFileReadWriteLock_ReadWriteLock*
AqvmBaseThreadingFileLock_GetFileLock(struct AqvmBaseFile_File* file) {
  if (file == NULL || file->identifier == NULL) {
    // TODO(logging)
    return NULL;
  }

  struct AqvmBaseLinkedList_LinkedList* linked_list =
      &AqvmBaseThreadingFileLock_fileLockTable
           .data[AqvmBaseFileIdentifier_GetIdentifierHash(file->identifier) %
                 1024];
  struct AqvmBaseLinkedList_Node* node = linked_list->head;

  struct AqvmBaseFileReadWriteLock_ReadWriteLock* lock = NULL;
  while (node != NULL) {
    if (AqvmBaseFileIdentifier_IsEqual(
            file->identifier,
            (AqvmBaseFileIdentifier_Identifier*)((struct AqvmBase_Pair*)
                                                     node->data)
                ->key)) {
      lock = (struct
              AqvmBaseFileReadWriteLock_ReadWriteLock*)((struct AqvmBase_Pair*)
                                                            node->data)
                 ->value;
      break;
    }
    node = node->next;
  }
  return lock;
}