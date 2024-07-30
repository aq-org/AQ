#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_THREADING_FILE_LOCK_UNIX_FILE_LOCK_H_
#define AQ_AQVM_BASE_THREADING_FILE_LOCK_UNIX_FILE_LOCK_H_

#include <stdio.h>

int AqvmBaseThreadingFileLockUnix_LockFile(FILE* file);

int AqvmBaseThreadingFileLockUnix_UnlockFile(FILE* file);

#endif
#endif