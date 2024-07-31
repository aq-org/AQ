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

void AqvmBaseLogging_OutputLog(const char* type, const char* code,
                               const char* message, const char* other_info) {
  char time_str[28];
  time_t current_time = time(NULL);
  strftime(time_str, 28, "%Y-%m-%dT%H:%M:%S%z", localtime(&current_time));

  const char* format = NULL;

  if (other_info != NULL) {
    format =
        "{\"Time\":\"%s\",\"Type\":\"%s\",\"Code\":\"%s\",\"Message\":\"%s\",%"
        "s}\n";
  }

  AqvmBaseLogging_ProcessLog(format, time_str, type, code, message, other_info);
}

void AqvmBaseLogging_ProcessLog(const char* format, const char* time,
                                const char* type, const char* code,
                                const char* message, const char* other_info,
                                ...) {
  if (format == NULL) {
    format =
        "{\"Time\":\"%s\",\"Type\":\"%s\",\"Code\":\"%s\",\"Message\":\"%s\"}"
        "\n";
  }
  if (time == NULL) {
    time = "NULL";
  }
  if (type == NULL) {
    type = "NULL";
  }
  if (code == NULL) {
    code = "NULL";
  }
  if (message == NULL) {
    message = "NULL";
  }

  AqvmBaseLogging_OutputLogToConsole(format, time, type, code, message,
                                     other_info);
  AqvmBaseLogging_OutputLogToFile(format, time, type, code, message,
                                  other_info);
}

int AqvmBaseLogging_OutputLogToConsole(const char* format, ...) {
  va_list args;
  va_start(args, format);

  int result = AqvmBaseIo_vfprintf(AqvmBaseIo_stderr, format, args);
  if (result != 0) {
    // TODO
    return -1;
  }

  va_end(args);
  return result;
}

int AqvmBaseLogging_OutputLogToFile(const char* format, ...) {
  struct AqvmBaseFile_File* log_ptr = AqvmBaseFile_fopen(".aqvm_log.log", "a");
  if (log_ptr == NULL) {
    return -1;
  }

  va_list args;
  va_start(args, format);

  int result = AqvmBaseIo_vfprintf(log_ptr, format, args);
  if (result != 0) {
    // TODO
    return -2;
  }

  va_end(args);
  fclose(log_ptr);
  return result;
}