// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_FILE_IDENTIFIER_IDENTIFIER_H_
#define AQ_AQVM_BASE_FILE_IDENTIFIER_IDENTIFIER_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __unix__
#include "aqvm/base/file/identifier/unix/identifier.h"
#elif _WIN32
#include "aqvm/base/file/identifier/windows/identifier.h"
#endif

#ifdef __unix__
typedef AqvmBaseFileIdentifierUnix_Identifier AqvmBaseFileIdentifier_Identifier;
#elif _WIN32
typedef AqvmBaseFileIdentifierWindows_Identifier
    AqvmBaseFileIdentifier_Identifier;
#else
typedef void AqvmBaseFileIdentifier_Identifier;
#endif

int AqvmBaseFileIdentifier_GetIdentifier(
    const char* filename, AqvmBaseFileIdentifier_Identifier* identifier);

uint32_t AqvmBaseFileIdentifier_GetIdentifierHash(
    const AqvmBaseFileIdentifier_Identifier* identifier);

bool AqvmBaseFileIdentifier_IsEqual(
    const AqvmBaseFileIdentifier_Identifier* identifier1,
    const AqvmBaseFileIdentifier_Identifier* identifier2);

#endif