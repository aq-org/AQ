// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/runtime/debugger/debugger.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

void AqvmRuntimeDebugger_ReportError(const char* error_code,
                                     const char* error_message, ...) {
  size_t message_size = 0;
  message_size = strlen(error_code) + strlen(error_message);
  va_list args;
  va_start(args, error_message);
  char* message = malloc(message_size + 1);
  vsnprintf(message, message_size + 1, error_message, args);
  va_end(args);
  fprintf(stderr(), "%s\n", message);
  free(message);
}

void AqvmRuntimeDebugger_ReportWarning(const char* warning_code,
                                       const char* warning_message, ...) {}

void AqvmRuntimeDebugger_ReportInfo(const char* info_code,
                                    const char* info_message, ...) {}

void AqvmRuntimeDebugger_Output(const char* debug_message) {}