// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/runtime/debugger/debugger.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void AqvmRuntimeDebugger_OutputReport(
    struct AqvmRuntimeDebugger_DebugReport report) {
  const char* type = AqvmRuntimeDebugger_FormatReport(&report);

  char time[28];
  AqvmRuntimeDebugger_GetCurrentTime(time);

  fprintf(stderr,
          "{\"Time\":%s,\"Type\":%s,\"Code\":%s,\"Message\":%s,\"ErrnoInfo\":{\"Errno\":%d,\"Message\":\"%s\"},\"OtherInfo\":%s}\n",
          time, type, report.code, report.message, errno, strerror(errno),
          report.other_info);
}

const char* AqvmRuntimeDebugger_FormatReport(
    struct AqvmRuntimeDebugger_DebugReport* report) {
  if (report->code == NULL) {
    report->code = "NULL";
  }
  if (report->message == NULL) {
    report->message = "NULL";
  }
  if (report->other_info == NULL) {
    report->other_info = "NULL";
  }

  switch (report->type) {
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