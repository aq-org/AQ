#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/process/file_lock/unix/file_lock.h"

#include <fcntl.h>
#include <stdio.h>

#include "aqvm/base/file/file.h"

int AqvmBaseProcessFileLockUnix_LockFile(struct AqvmBaseFile_File* file) {
  if (file == NULL || file->file == NULL) {
    // TODO
    return -1;
  }

  int file_descriptor = fileno(file->file);
  if (file_descriptor == -1) {
    return -2;
  }

  struct flock file_lock = {0};
  file_lock.l_type = F_WRLCK;
  file_lock.l_whence = SEEK_SET;

  if (fcntl(file_descriptor, F_SETLK, &file_lock) == -1) {
    return -3;
  }
  return 0;
}

int AqvmBaseProcessFileLockUnix_UnlockFile(struct AqvmBaseFile_File* file) {
  if (file == NULL || file->file == NULL) {
    // TODO
    return -1;
  }

  int file_descriptor = fileno(file->file);
  if (file_descriptor == -1) {
    return -2;
  }

  struct flock file_lock = {0};
  file_lock.l_type = F_UNLCK;

  if (fcntl(file_descriptor, F_SETLK, &file_lock) == -1) {
    // TODO
    return -3;
  }
  return 0;
}

#endif