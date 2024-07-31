#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_PROCESS_FILE_LOCK_WINDOWS_FILE_LOCK_H_
#define AQ_AQVM_BASE_PROCESS_FILE_LOCK_WINDOWS_FILE_LOCK_H_

#include <stdio.h>

#include "aqvm/base/file/file.h"

int AqvmBaseProcessFileLockWindows_LockFile(struct AqvmBaseFile_File* file);

int AqvmBaseProcessFileLockWindows_UnlockFile(struct AqvmBaseFile_File* file);

#endif
#endif