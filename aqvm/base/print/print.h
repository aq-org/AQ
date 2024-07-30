// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_PRINT_PRINT_H_
#define AQ_AQVM_BASE_PRINT_PRINT_H_

#include <stdarg.h>
#include <stdio.h>

#include "aqvm/base/threading/mutex/mutex.h"

AqvmBaseThreadingMutex_Mutex AqvmBasePrint_printMutex;

int AqvmBasePrint_InitializePrint();

int AqvmBasePrint_ClosePrint();

int AqvmBasePrint_printf(const char* format, ...);

int AqvmBasePrint_fprintf(FILE* const stream, const char* format, ...);

int AqvmBasePrint_vfprintf(FILE* const stream, const char* format, va_list args);

#endif