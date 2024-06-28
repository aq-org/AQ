// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/runtime/debugger/debugger.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

void AqvmRuntimeDebugger_OutputReport(
    struct AqvmRuntimeDebugger_DebugReport report) {
  char** type;

  AqvmRuntimeDebugger_FormatReport(&report, type, NULL);

  char time[26];
  AqvmRuntimeDebugger_GetCurrentTime(time);

  fprintf(stderr, "%s%s%s%s%s%s%s%s%s%s%s", "{ \"Time\":\"", time,
          "\", \"Type\":\"", *type, "\", \"Code\":\"", report.code,
          "\", \"Message\":\"", report.message, "\", \"OtherInfo\":\"",
          report.other_info, "\" }\n");
}

void AqvmRuntimeDebugger_FormatReport(
    struct AqvmRuntimeDebugger_DebugReport* report, char** type,
    const char* null_str) {
  if (report->code == NULL) {
    report->code = "NULL";
  }
  if (report->message == NULL) {
    report->message = "NULL";
  }
  if (report->other_info == NULL) {
    report->other_info = "NULL";
  }

  const char* type_str = NULL;

  switch (report->type) {
    case 0:
      type_str = "\"INFO\"";
      break;
    case 1:
      type_str = "\"WARNING\"";
      break;
    case 2:
      type_str = "\"ERROR\"";
      break;
    default:
      type_str = "NULL";
      break;
  }

  // *type = type_str;
}

void AqvmRuntimeDebugger_GetCurrentTime(char* return_time) {
  time_t current_time = time(NULL);
  strftime(return_time, 26, "%Y-%m-%dT%H:%M:%S%z", localtime(&current_time));
}