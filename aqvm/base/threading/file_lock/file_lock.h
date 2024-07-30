// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_THREADING_FILE_LOCK_FILE_LOCK_H_
#define AQ_AQVM_BASE_THREADING_FILE_LOCK_FILE_LOCK_H_

#include <stdio.h>

#include "aqvm/base/threading/file_lock/unix/file_lock.h"
#include "aqvm/base/threading/file_lock/windows/file_lock.h"

int AqvmBaseThreadingFileLock_LockFile(FILE* file);

int AqvmBaseThreadingFileLock_UnlockFile(FILE* file);

#endif