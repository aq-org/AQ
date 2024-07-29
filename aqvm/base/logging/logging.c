// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/logging/logging.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void AqvmBaseLogging_OutputLog(const char* type, const char* code,
                               const char* message, const char* other_info) {
  /*if (type == NULL) {
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
  }*/

  char time_str[28];
  time_t current_time = time(NULL);
  strftime(time_str, 28, "%Y-%m-%dT%H:%M:%S%z", localtime(&current_time));

  AqvmBaseLogging_ProcessLog(time_str, type, code, message, errno,
                             strerror(errno), other_info);

  /*fprintf(stderr,
          "{Time:%s,Type:%s,Code:%s,Message:%s,ErrnoInfo:{Errno:%d,Message:%s},"
          "OtherInfo:%s}\n",
          time_str, type, code, message, errno, strerror(errno), other_info);

  FILE* log_ptr = fopen(".aqvm_log.log", "a");
  if (log_ptr == NULL) {
    fprintf(stderr,
            "{Time:%s,Type:%s,Code:%s,Message:%s,ErrnoInfo:{Errno:%d,Message:%"
            "s},OtherInfo:%s}\n",
            time_str, "ERROR", "AqvmBaseLogging_OutputLog_OutputToFileError",
            "Failed to open log file", errno, strerror(errno), "NULL");
    return;
  }
  fprintf(log_ptr,
          "{Time:%s,Type:%s,Code:%s,Message:%s,ErrnoInfo:{"
          "Errno:%d,Message:%s},OtherInfo:%s}\n",
          time_str, type, code, message, errno, strerror(errno), other_info);
  fclose(log_ptr);*/
}

void AqvmBaseLogging_ProcessLog(const char* time, const char* type,
                                const char* code, const char* message,
                                int error_number, const char* errno_message,
                                const char* other_info) {
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
  if (errno_message == NULL) {
    errno_message = "NULL";
  }
  if (other_info == NULL) {
    other_info = "NULL";
  }

  AqvmBaseLogging_OutputLogToConsole(time, type, code, message, error_number,
                                     errno_message, other_info);
  AqvmBaseLogging_OutputLogToFile(time, type, code, message, error_number,
                                  errno_message, other_info);
}

int AqvmBaseLogging_OutputLogToConsole(const char* time, const char* type,
                                       const char* code, const char* message,
                                       int error_number,
                                       const char* errno_message,
                                       const char* other_info) {
  fprintf(stderr,
          "{\"Time\":\"%s\",\"Type\":\"%s\",\"Code\":\"%s\",\"Message\":\"%s\","
          "\"ErrnoInfo\":{\"Errno\":%d,\"Message\":\"%s\"},"
          "\"OtherInfo\":\"%s\"}\n",
          time, type, code, message, error_number, errno_message, other_info);
}

/*int AqvmBaseLogging_OutputLogToConsole(const char* format, ...) {
    va_list args;
    va_start(args, format);

    int result = vfprintf(stderr, format, args);

    va_end(args);
    return result;
}*/

int AqvmBaseLogging_OutputLogToFile(const char* time, const char* type,
                                    const char* code, const char* message,
                                    int error_number, const char* errno_message,
                                    const char* other_info) {
  FILE* log_ptr = fopen(".aqvm_log.log", "a");
  if (log_ptr == NULL) {
    fprintf(stderr,
            "{Time:%s,Type:%s,Code:%s,Message:%s,ErrnoInfo:{Errno:%d,Message:%"
            "s},OtherInfo:%s}\n",
            time, "ERROR", "AqvmBaseLogging_OutputLog_OutputToFileError",
            "Failed to open log file", errno, strerror(errno), "NULL");
    return;
  }
  fprintf(log_ptr,
          "{\"Time\":\"%s\",\"Type\":\"%s\",\"Code\":\"%s\",\"Message\":\"%s\","
          "\"ErrnoInfo\":{\"Errno\":%d,\"Message\":\"%s\"},"
          "\"OtherInfo\":\"%s\"}\n",
          time, type, code, message, error_number, errno_message, other_info);
  fclose(log_ptr);
}