#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQVM_BASE_FILE_FILE_ID_UNIX_FILE_ID_H_
#define AQVM_BASE_FILE_FILE_ID_UNIX_FILE_ID_H_

#include <stdint.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "aqvm/base/file/file.h"

typedef struct {
  dev_t st_dev;
  ino_t st_ino;
} AqvmBaseFileFileIdUnix_FileId;

int AqvmBaseFileFileIdUnix_GetFileId(const char* filename,
                                     AqvmBaseFileFileIdUnix_FileId* file_id);

uint32_t AqvmBaseFileFileIdUnix_GetFileIdHash(
    AqvmBaseFileFileIdUnix_FileId* file_id);

#endif

#endif