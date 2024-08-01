#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/file_id/windows/file_id.h"

#include <windows.h>

#include "aqvm/base/hash/hash.h"

int AqvmBaseFileFileIdWindows_GetFileId(
    const char* filename, AqvmBaseFileFileIdWindows_FileId* file_id) {
  if (filename == NULL || file_id == NULL) {
    // TODO
    return -1;
  }

  HANDLE handle_file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (handle_file == INVALID_HANDLE_VALUE) {
    // TODO
    return -2;
  }

  BY_HANDLE_FILE_INFORMATION file_info;
  if (GetFileInformationByHandle(handle_file, &file_info)) {
    file_id->dwVolumeSerialNumber = file_info.dwVolumeSerialNumber;
    file_id->nFileIndexHigh = file_info.nFileIndexHigh;
    file_id->nFileIndexLow = file_info.nFileIndexLow;
  } else {
    // TODO
    return -3;
  }

  if (CloseHandle(handle_file) == FALSE) {
    // TODO
    return -4;
  }

  return 0;
}

uint32_t AqvmBaseFileFileIdWindows_GetFileIdHash(
    AqvmBaseFileFileIdWindows_FileId* file_id) {
  if (file_id == NULL) {
    // TODO
    return 0;
  }
  uint32_t hash[3];

  hash[0] = file_id->dwVolumeSerialNumber;
  hash[1] = file_id->nFileIndexHigh;
  hash[2] = file_id->nFileIndexLow;

  return AqvmBaseHash_HashUnsignedIntArray(hash, 3);
}
#endif