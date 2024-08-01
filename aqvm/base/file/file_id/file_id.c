// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/file_id/file_id.h"

#include <stdint.h>

#include "aqvm/base/file/file.h"

#ifdef __unix__
#include "aqvm/base/file/file_id/unix/file_id.h"
#elif _WIN32
#include "aqvm/base/file/file_id/windows/file_id.h"
#endif

int AqvmBaseFileFileId_GetFileId(const char* filename,
                                 AqvmBaseFileFileId_FileId* file_id) {
  if (filename == NULL || file_id == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseFileFileIdUnix_GetFileId(filename, file_id) != 0) {
    // TODO
    return -2;
  }
#elif _WIN32
  if (AqvmBaseFileFileIdWindows_GetFileId(filename, file_id) != 0) {
    // TODO
    return -3;
  }
#else
  // TODO
  return -4;
#endif

  return 0;
}

uint32_t AqvmBaseFileFileId_GetFileIdHash(
    const AqvmBaseFileFileId_FileId* file_id) {
  if (file_id == NULL) {
    // TODO
    return 0;
  }

#ifdef __unix__
  return AqvmBaseFileFileIdUnix_GetFileIdHash(file_id);
#elif _WIN32
  return AqvmBaseFileFileIdWindows_GetFileIdHash(file_id);
#else
  // TODO
  return 0;
#endif
}