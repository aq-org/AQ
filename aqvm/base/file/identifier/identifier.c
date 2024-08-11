// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/identifier/identifier.h"

#include <stdint.h>

#include "aqvm/base/file/file.h"

#ifdef __unix__
#include "aqvm/base/file/identifier/unix/identifier.h"
#elif _WIN32
#include "aqvm/base/file/identifier/windows/identifier.h"
#endif

int AqvmBaseFileIdentifier_GetIdentifier(
    const char* filename, AqvmBaseFileIdentifier_Identifier* identifier) {
  if (filename == NULL || identifier == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseFileIdentifierUnix_GetIdentifier(filename, identifier) != 0) {
    // TODO
    return -2;
  }
#elif _WIN32
  if (AqvmBaseFileIdentifierWindows_GetIdentifier(filename, identifier) != 0) {
    // TODO
    return -3;
  }
#else
  // TODO
  return -4;
#endif

  return 0;
}

uint32_t AqvmBaseFileIdentifier_GetIdentifierHash(
    const AqvmBaseFileIdentifier_Identifier* identifier) {
  if (identifier == NULL) {
    // TODO
    return 0;
  }

#ifdef __unix__
  return AqvmBaseFileIdentifierUnix_GetIdentifierHash(identifier);
#elif _WIN32
  return AqvmBaseFileIdentifierWindows_GetIdentifierHash(identifier);
#else
  // TODO
  return 0;
#endif
}

bool AqvmBaseFileIdentifier_IsEqual(
    const AqvmBaseFileIdentifier_Identifier* identifier1,
    const AqvmBaseFileIdentifier_Identifier* identifier2) {
  if (identifier1 == NULL || identifier2 == NULL) {
    // TODO
    return false;
  }

#ifdef __unix__
  return AqvmBaseFileIdentifierUnix_IsEqual(identifier1, identifier2);
#elif _WIN32
  return AqvmBaseFileIdentifierWindows_IsEqual(identifier1, identifier2);
#else
  // TODO
  return false;
#endif
}