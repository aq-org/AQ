// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/file_lock/file_lock.h"

#include "aqvm/base/threading/file_lock/unix/file_lock.h"
#include "aqvm/base/threading/file_lock/windows/file_lock.h"

int AqvmBaseThreadingFileLock_LockFile(FILE* file) {
#ifdef __unix__
  if (AqvmBaseThreadingFileLockUnix_LockFile(file)) {
    // TODO
    return -1;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingFileLockWindows_LockFile(file) != 0) {
    // TODO
    return -1;
  }
  return 0;
#else
  // TODO
  return -2;
#endif
}

int AqvmBaseThreadingFileLock_UnlockFile(FILE* file) {
#ifdef __unix__
  if (AqvmBaseThreadingFileLockUnix_UnlockFile(file)) {
    // TODO
    return -1;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingFileLockWindows_UnlockFile(file) != 0) {
    // TODO
    return -1;
  }
  return 0;
#else
  // TODO
  return -2;
#endif
}