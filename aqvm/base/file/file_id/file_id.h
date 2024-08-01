// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQVM_BASE_FILE_FILE_ID_FILE_ID_H_
#define AQVM_BASE_FILE_FILE_ID_FILE_ID_H_

#include <stdint.h>

#ifdef __unix__
#include "aqvm/base/file/file_id/unix/file_id.h"
#elif _WIN32
#include "aqvm/base/file/file_id/windows/file_id.h"
#endif

#ifdef __unix__
typedef AqvmBaseFileFileIdUnix_FileId AqvmBaseFileFileId_FileId;
#elif _WIN32
typedef AqvmBaseFileFileIdWindows_FileId AqvmBaseFileFileId_FileId;
#else
typedef void AqvmBaseFileFileId_FileId;
#endif

int AqvmBaseFileFileId_GetFileId(const char* filename,
                                 AqvmBaseFileFileId_FileId* file_id);

uint32_t AqvmBaseFileFileId_GetFileIdHash(AqvmBaseFileFileId_FileId* file_id);

#endif