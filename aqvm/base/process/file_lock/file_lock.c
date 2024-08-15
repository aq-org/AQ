// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/process/file_lock/file_lock.h"

#include "aqvm/base/file/file.h"

#ifdef __unix__
#include "aqvm/base/process/file_lock/unix/file_lock.h"
#elif _WIN32
#include "aqvm/base/process/file_lock/windows/file_lock.h"
#endif

int AqvmBaseProcessFileLock_LockFile(struct AqvmBaseFile_File* file) {
#ifdef __unix__
  if (AqvmBaseProcessFileLockUnix_LockFile(file) != 0) {
    // TODO(logging)
    return -1;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseProcessFileLockWindows_LockFile(file) != 0) {
    // TODO(logging)
    return -1;
  }
  return 0;
#else
  // TODO(logging)
  return -2;
#endif
}

int AqvmBaseProcessFileLock_UnlockFile(struct AqvmBaseFile_File* file) {
#ifdef __unix__
  if (AqvmBaseProcessFileLockUnix_UnlockFile(file) != 0) {
    // TODO(logging)
    return -1;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseProcessFileLockWindows_UnlockFile(file) != 0) {
    // TODO(logging)
    return -1;
  }
  return 0;
#else
  // TODO(logging)
  return -2;
#endif
}