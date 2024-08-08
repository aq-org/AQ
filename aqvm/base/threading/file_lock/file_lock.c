// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/file_lock/file_lock.h"

#include "aqvm/base/file/file.h"
#include "aqvm/base/file/identifier/identifier.h"
#include "aqvm/base/file/read_write_lock/read_write_lock.h"
#include "aqvm/base/hash/table/table.h"
#include "aqvm/base/pair.h"

struct AqvmBaseHashTable_HashTable AqvmBaseThreadingFileLock_fileLockTable;

int AqvmBaseThreadingFileLock_CloseFileLockTable() {
  if (AqvmBaseHashTable_CloseHashTable(
          &AqvmBaseThreadingFileLock_fileLockTable) != 0) {
    // TODO
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

int AqvmBaseThreadingFileLock_InsertFileLock(struct AqvmBaseFile_File* file) {
  if (file == NULL) {
    // TODO
    return -1;
  }

  AqvmBaseThreadingFileLock_fileLockTable
      .data[AqvmBaseFileIdentifier_GetIdentifierHash(&file->identifier) % 1024];
}