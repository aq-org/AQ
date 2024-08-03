#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQVM_BASE_FILE_IDENTIFIER_UNIX_IDENTIFIER_H_
#define AQVM_BASE_FILE_IDENTIFIER_UNIX_IDENTIFIER_H_

#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "aqvm/base/file/file.h"

typedef struct {
  dev_t st_dev;
  ino_t st_ino;
} AqvmBaseFileIdentifierUnix_Identifier;

int AqvmBaseFileIdentifierUnix_GetIdentifier(const char* filename,
                                     AqvmBaseFileIdentifierUnix_Identifier* identifier);

uint32_t AqvmBaseFileIdentifierUnix_GetIdentifierHash(
    const AqvmBaseFileIdentifierUnix_Identifier* identifier);

#endif

#endif