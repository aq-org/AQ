#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/file_id/unix/file_id.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "aqvm/base/hash/hash.h"

int AqvmBaseFileFileIdUnix_GetFileId(const char* filename,
                                     AqvmBaseFileFileIdUnix_FileId* file_id) {
  if (filename == NULL || file_id == NULL) {
    // TODO
    return -1;
  }

  struct stat* file_info;

  if (stat(filename, file_info) != 0) {
    // TODO
    return -2;
  }

  file_id->st_dev = file_info->st_dev;
  file_id->st_ino = file_info->st_ino;

  return 0;
}

uint32_t AqvmBaseFileFileIdUnix_GetFileIdHash(
    const AqvmBaseFileFileIdUnix_FileId* file_id) {
  if (file_id == NULL) {
    // TODO
    return 0;
  }
  uint32_t hash[2];

  hash[0] = file_id->st_dev;
  hash[1] = file_id->st_ino;

  return AqvmBaseHash_HashUnsignedIntArray(hash, 2);
}

#endif