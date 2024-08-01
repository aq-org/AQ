// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_PROCESS_FILE_LOCK_FILE_LOCK_H_
#define AQ_AQVM_BASE_PROCESS_FILE_LOCK_FILE_LOCK_H_

#include <stdio.h>

#include "aqvm/base/file/file.h"

#ifdef __unix__
#include "aqvm/base/process/file_lock/unix/file_lock.h"
#elif _WIN32
#include "aqvm/base/process/file_lock/windows/file_lock.h"
#endif

int AqvmBaseProcessFileLock_LockFile(struct AqvmBaseFile_File* file);

int AqvmBaseProcessFileLock_UnlockFile(struct AqvmBaseFile_File* file);

#endif