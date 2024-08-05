#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/process/file_lock/windows/file_lock.h"

#include <stdio.h>
#include <Windows.h>

#include "aqvm/base/file/windows/file.h"

int AqvmBaseProcessFileLockWindows_LockFile(struct AqvmBaseFile_File* file) {
  if (file == NULL || file->file == NULL) {
    // TODO
    return -1;
  }

  HANDLE handle_file = AqvmBaseFileWindows_ConvertFileToHandle(file);
  if (handle_file == INVALID_HANDLE_VALUE) {
    // TODO
    return -2;
  }

  if (!LockFile(handle_file, 0, 0, MAXDWORD, MAXDWORD)) {
    // TODO
    return -3;
  }
  return 0;
}

int AqvmBaseProcessFileLockWindows_UnlockFile(struct AqvmBaseFile_File* file) {
  if (file == NULL || file->file == NULL) {
    // TODO
    return -1;
  }

  HANDLE handle_file = AqvmBaseFileWindows_ConvertFileToHandle(file);
  if (handle_file == INVALID_HANDLE_VALUE) {
    // TODO
    return -2;
  }

  if (!UnlockFile(handle_file, 0, 0, MAXDWORD, MAXDWORD)) {
    // TODO
    return -3;
  }
  return 0;
}

#endif