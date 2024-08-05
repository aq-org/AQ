#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/identifier/windows/identifier.h"

#include <Windows.h>

#include "aqvm/base/hash/hash.h"

int AqvmBaseFileIdentifierWindows_GetIdentifier(
    const char* filename, AqvmBaseFileIdentifierWindows_Identifier* identifier) {
  if (filename == NULL || identifier == NULL) {
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
    identifier->dwVolumeSerialNumber = file_info.dwVolumeSerialNumber;
    identifier->nFileIndexHigh = file_info.nFileIndexHigh;
    identifier->nFileIndexLow = file_info.nFileIndexLow;
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

uint32_t AqvmBaseFileIdentifierWindows_GetIdentifierHash(
    const AqvmBaseFileIdentifierWindows_Identifier* identifier) {
  if (identifier == NULL) {
    // TODO
    return 0;
  }
  uint32_t hash[3];

  hash[0] = identifier->dwVolumeSerialNumber;
  hash[1] = identifier->nFileIndexHigh;
  hash[2] = identifier->nFileIndexLow;

  return AqvmBaseHash_HashUnsignedIntArray(hash, 3);
}
#endif