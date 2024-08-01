// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_HASH_HASH_H_
#define AQ_AQVM_BASE_HASH_HASH_H_

#include <stdint.h>
#include <stdlib.h>

int AqvmBaseHash_Hash(int data);

uint32_t AqvmBaseHash_HashString(const char* str);

uint32_t AqvmBaseHash_HashUnsignedIntArray(const uint32_t* data, size_t size);

#endif