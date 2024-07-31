#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/process/file_lock/windows/file_lock.h"

#include <stdio.h>
#include <windows.h>

#include "aqvm/base/file/windows/file.h"

int AqvmBaseProcessFileLockWindows_LockFile(FILE* file) {
  HANDLE handle_file = AqvmBaseFileWindows_FileToHandle(file);
  if (handle_file == INVALID_HANDLE_VALUE) {
    // TODO
    return -1;
  }

  if (!LockFile(handle_file, 0, 0, MAXDWORD, MAXDWORD)) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseProcessFileLockWindows_UnlockFile(FILE* file) {
  HANDLE handle_file = AqvmBaseFileWindows_FileToHandle(file);
  if (handle_file == INVALID_HANDLE_VALUE) {
    // TODO
    return -1;
  }

  if (!UnlockFile(handle_file, 0, 0, MAXDWORD, MAXDWORD)) {
    // TODO
    return -2;
  }
  return 0;
}

#endif