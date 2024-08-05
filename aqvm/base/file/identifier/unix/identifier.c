#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/identifier/unix/identifier.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "aqvm/base/hash/hash.h"

int AqvmBaseFileIdentifierUnix_GetIdentifier(const char* filename,
                                     AqvmBaseFileIdentifierUnix_Identifier* identifier) {
  if (filename == NULL || identifier == NULL) {
    // TODO
    return -1;
  }

  struct stat st;

  if (stat(filename, &st) != 0) {
    // TODO
    return -2;
  }

  identifier->st_dev = st.st_dev;
  identifier->st_ino = st.st_ino;

  return 0;
}

uint32_t AqvmBaseFileIdentifierUnix_GetIdentifierHash(
    const AqvmBaseFileIdentifierUnix_Identifier* identifier) {
  if (identifier == NULL) {
    // TODO
    return 0;
  }
  uint32_t hash[2];

  hash[0] = identifier->st_dev;
  hash[1] = identifier->st_ino;

  return AqvmBaseHash_HashUnsignedIntArray(hash, 2);
}

#endif