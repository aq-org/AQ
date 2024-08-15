#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/identifier/windows/identifier.h"

#include <windows.h>

#include "aqvm/base/file/windows/file.h"
#include "aqvm/base/hash/hash.h"

int AqvmBaseFileIdentifierWindows_GetIdentifier(
    const struct AqvmBaseFile_File* file,
    AqvmBaseFileIdentifierWindows_Identifier* identifier) {
  if (file == NULL || file->file == NULL || identifier == NULL) {
    // TODO(logging)
    return -1;
  }

  HANDLE handle_file;
  handle_file = AqvmBaseFileWindows_ConvertFileToHandle(file);
  if (handle_file == INVALID_HANDLE_VALUE) {
    // TODO(logging)
    return -2;
  }

  BY_HANDLE_FILE_INFORMATION file_info;
  if (GetFileInformationByHandle(handle_file, &file_info)) {
    identifier->dwVolumeSerialNumber = file_info.dwVolumeSerialNumber;
    identifier->nFileIndexHigh = file_info.nFileIndexHigh;
    identifier->nFileIndexLow = file_info.nFileIndexLow;
  } else {
    // TODO(logging)
    return -3;
  }

  if (CloseHandle(handle_file) == FALSE) {
    // TODO(logging)
    return -4;
  }

  return 0;
}

uint32_t AqvmBaseFileIdentifierWindows_GetIdentifierHash(
    const AqvmBaseFileIdentifierWindows_Identifier* identifier) {
  if (identifier == NULL) {
    // TODO(logging)
    return 0;
  }
  uint32_t hash[3];

  hash[0] = identifier->dwVolumeSerialNumber;
  hash[1] = identifier->nFileIndexHigh;
  hash[2] = identifier->nFileIndexLow;

  return AqvmBaseHash_HashUnsignedIntArray(hash, 3);
}

bool AqvmBaseFileIdentifierWindows_IsEqual(
    const AqvmBaseFileIdentifierWindows_Identifier* identifier1,
    const AqvmBaseFileIdentifierWindows_Identifier* identifier2) {
  if (identifier1 == NULL || identifier2 == NULL) {
    // TODO(logging)
    return false;
  }

  if (identifier1->dwVolumeSerialNumber == identifier2->dwVolumeSerialNumber &&
      identifier1->nFileIndexHigh == identifier2->nFileIndexHigh &&
      identifier1->nFileIndexLow == identifier2->nFileIndexLow) {
    return true;
  }
  return false;
}

#endif