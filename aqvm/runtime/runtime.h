// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_RUNTIME_RUNTIME_H_
#define AQ_AQVM_RUNTIME_RUNTIME_H_

#include "aqvm/runtime/bytecode/bytecode.h"

// Initialize the runtime environment. Returns 0 for success, other values for
// errors.
int AqvmRuntime_InitRuntime();

#endif