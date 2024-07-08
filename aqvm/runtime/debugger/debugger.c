// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/runtime/debugger/debugger.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void AqvmRuntimeDebugger_OutputReport(uint8_t type, const char* code,
                                      const char* message,
                                      const char* other_info) {
  const char* type =
      AqvmRuntimeDebugger_FormatReport(type, code, message, other_info);

  char time[28];
  AqvmRuntimeDebugger_GetCurrentTime(time);

  fprintf(stderr,
          "{\"Time\":%s,\"Type\":%s,\"Code\":%s,\"Message\":%s,\"ErrnoInfo\":{"
          "\"Errno\":%d,\"Message\":\"%s\"},\"OtherInfo\":%s}\n",
          time, type, code, message, errno, strerror(errno), other_info);
}

const char* AqvmRuntimeDebugger_FormatReport(uint8_t type, const char* code,
                                             const char* message,
                                             const char* other_info) {
  if (code == NULL) {
    code = "NULL";
  }
  if (message == NULL) {
    message = "NULL";
  }
  if (other_info == NULL) {
    other_info = "NULL";
  }

  switch (type) {
    case 0:
      return "\"INFO\"";
    case 1:
      return "\"WARNING\"";
    case 2:
      return "\"ERROR\"";
    default:
      return "NULL";
  }
}

void AqvmRuntimeDebugger_GetCurrentTime(char* return_time) {
  time_t current_time = time(NULL);
  strftime(return_time, 28, "\"%Y-%m-%dT%H:%M:%S%z\"",
           localtime(&current_time));
}