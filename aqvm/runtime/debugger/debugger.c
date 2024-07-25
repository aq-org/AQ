// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/runtime/debugger/debugger.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void AqvmRuntimeDebugger_OutputLog(const char* type, const char* code,
                                   const char* message,
                                   const char* other_info) {
  if (type == NULL) {
    type = "NULL";
  }
  if (code == NULL) {
    code = "NULL";
  }
  if (message == NULL) {
    message = "NULL";
  }
  if (other_info == NULL) {
    other_info = "NULL";
  }

  char time_str[28];
  time_t current_time = time(NULL);
  strftime(time_str, 28, "\"%Y-%m-%dT%H:%M:%S%z\"", localtime(&current_time));

  fprintf(stderr,
          "{\"Time\":%s,\"Type\":%s,\"Code\":%s,\"Message\":%s,\"ErrnoInfo\":{"
          "\"Errno\":%d,\"Message\":\"%s\"},\"OtherInfo\":%s}\n",
          time_str, type, code, message, errno, strerror(errno), other_info);

  FILE* log_ptr = fopen(".aqvm_debug_report.log", "a");
  if (log_ptr == NULL) {
    fprintf(
        stderr,
        "{\"Time\":%s,\"Type\":%s,\"Code\":%s,\"Message\":%s,\"ErrnoInfo\":{"
        "\"Errno\":%d,\"Message\":\"%s\"},\"OtherInfo\":%s}\n",
        time_str, "ERROR", "AqvmRuntimeDebugger_OutputLog_OutputToFileError",
        "Failed to open log file", errno, strerror(errno), "NULL");
    return;
  }
  fprintf(log_ptr,
          "{\"Time\":%s,\"Type\":%s,\"Code\":%s,\"Message\":%s,\"ErrnoInfo\":{"
          "\"Errno\":%d,\"Message\":\"%s\"},\"OtherInfo\":%s}\n",
          time_str, type, code, message, errno, strerror(errno), other_info);
  fclose(log_ptr);
}