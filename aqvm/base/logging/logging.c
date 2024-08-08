// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/logging/logging.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "aqvm/base/file/file.h"
#include "aqvm/base/io/io.h"
#include "aqvm/base/time/time.h"

void AqvmBaseLogging_OutputLog(const char* type, const char* code,
                               const char* message, ...) {
  char time_str[31];
  if (AqvmBaseTime_GetCurrentTimeString(time_str) != 0) {
    // TODO
    return;
  }

  va_list other_info;
  va_start(other_info, message);
  AqvmBaseLogging_ProcessLog(time_str, type, code, message, other_info, NULL);
  va_end(other_info);
}

void AqvmBaseLogging_ProcessLog(const char* time, const char* type,
                                const char* code, const char* message,
                                va_list other_info, ...) {
  if (time == NULL) time = "NULL";
  if (type == NULL) type = "NULL";
  if (code == NULL) code = "NULL";
  if (message == NULL) message = "NULL";

  va_list system_info;
  va_start(system_info, other_info);

  va_list system_info_to_console;
  va_list other_info_to_console;
  va_copy(system_info_to_console, system_info);
  va_copy(other_info_to_console, other_info);

  va_list system_info_to_file;
  va_list other_info_to_file;
  va_copy(system_info_to_file, system_info);
  va_copy(other_info_to_file, other_info);

  AqvmBaseLogging_OutputLogToConsole(
      time, type, code, message, other_info_to_console, system_info_to_console);
  AqvmBaseLogging_OutputLogToFile(time, type, code, message, other_info_to_file,
                                  system_info_to_file);

  va_end(system_info);
}

int AqvmBaseLogging_OutputLogToConsole(const char* time, const char* type,
                                       const char* code, const char* message,
                                       va_list system_info,
                                       va_list other_info) {
  int result = AqvmBaseIo_OutputLog(AqvmBaseIo_stderr, time, type, code,
                                    message, system_info, other_info);
  if (result != 0) {
    // TODO
    return -1;
  }
  return 0;
}

int AqvmBaseLogging_OutputLogToFile(const char* time, const char* type,
                                    const char* code, const char* message,
                                    va_list system_info, va_list other_info) {
  struct AqvmBaseFile_File* log_ptr = AqvmBaseFile_fopen(".aqvm_log.log", "a");
  if (log_ptr == NULL) {
    // TODO
    return -1;
  }

  int result = AqvmBaseIo_OutputLog(log_ptr, time, type, code, message,
                                    system_info, other_info);
  if (result != 0) {
    // TODO
    return -2;
  }

  AqvmBaseFile_fclose(log_ptr);

  return 0;
}