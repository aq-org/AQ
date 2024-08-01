#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQVM_BASE_FILE_FILE_ID_WINDOWS_FILE_ID_H_
#define AQVM_BASE_FILE_FILE_ID_WINDOWS_FILE_ID_H_

#include <stdint.h>
#include <windows.h>

#include "aqvm/base/file/file.h"

typedef struct {
  DWORD dwVolumeSerialNumber;
  DWORD nFileIndexHigh;
  DWORD nFileIndexLow;
} AqvmBaseFileFileIdWindows_FileId;

int AqvmBaseFileFileIdWindows_GetFileId(
    const char* filename, AqvmBaseFileFileIdWindows_FileId* file_id);

uint32_t AqvmBaseFileFileIdWindows_GetFileIdHash(
    AqvmBaseFileFileIdWindows_FileId* file_id);

#endif
#endif