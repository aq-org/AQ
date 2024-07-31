#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/process/file_lock/unix/file_lock.h"

#include <fcntl.h>
#include <stdio.h>

int AqvmBaseProcessFileLockUnix_LockFile(FILE* file) {
  int file_descriptor = fileno(file);
  if (file_descriptor == -1) {
    return -1;
  }

  struct flock file_lock = {0};
  file_lock.l_type = F_WRLCK;
  file_lock.l_whence = SEEK_SET;

  if (fcntl(file_descriptor, F_SETLK, &file_lock) == -1) {
    return -1;
  }
  return 0;
}

int AqvmBaseProcessFileLockUnix_UnlockFile(FILE* file) {
  int file_descriptor = fileno(file);
  if (file_descriptor == -1) {
    return -1;
  }

  struct flock file_lock = {0};
  file_lock.l_type = F_UNLCK;

  if (fcntl(file_descriptor, F_SETLK, &file_lock) == -1) {
    // TODO
    return -1;
  }
  return 0;
}

#endif