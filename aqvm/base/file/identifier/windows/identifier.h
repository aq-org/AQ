#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_FILE_IDENTIFIER_WINDOWS_IDENTIFIER_H_
#define AQ_AQVM_BASE_FILE_IDENTIFIER_WINDOWS_IDENTIFIER_H_

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

struct AqvmBaseFile_File;

typedef struct {
  DWORD dwVolumeSerialNumber;
  DWORD nFileIndexHigh;
  DWORD nFileIndexLow;
} AqvmBaseFileIdentifierWindows_Identifier;

int AqvmBaseFileIdentifierWindows_GetIdentifier(
    const struct AqvmBaseFile_File* file,
    AqvmBaseFileIdentifierWindows_Identifier* identifier);

uint32_t AqvmBaseFileIdentifierWindows_GetIdentifierHash(
    const AqvmBaseFileIdentifierWindows_Identifier* identifier);

bool AqvmBaseFileIdentifierWindows_IsEqual(
    const AqvmBaseFileIdentifierWindows_Identifier* identifier1,
    const AqvmBaseFileIdentifierWindows_Identifier* identifier2);

#endif
#endif