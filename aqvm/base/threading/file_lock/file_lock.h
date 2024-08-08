// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_THREADING_FILE_LOCK_FILE_LOCK_H_
#define AQ_AQVM_BASE_THREADING_FILE_LOCK_FILE_LOCK_H_

#include "aqvm/base/file/file.h"
#include "aqvm/base/file/identifier/identifier.h"
#include "aqvm/base/file/read_write_lock/read_write_lock.h"
#include "aqvm/base/hash/table/table.h"

extern struct AqvmBaseHashTable_HashTable
    AqvmBaseThreadingFileLock_fileLockTable;

int AqvmBaseThreadingFileLock_CloseFileLockTable();

int AqvmBaseThreadingFileLock_InitializeFileLockTable();

int AqvmBaseThreadingFileLock_InsertFileLock(struct AqvmBaseFile_File* file);

#endif