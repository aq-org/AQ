// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/time/time.h"

#include <time.h>

#include "aqvm/base/logging/logging.h"

int AqvmBaseTime_localtime(const time_t timestamp, struct tm* result) {
  // TODO
  AqvmBaseLogging_OutputLog(
      "WARNING", "AqvmBaseTime_localtime_ThreadUnsafeWarning",
      "The localtime function may cause thread unsafety.", NULL);
  struct tm* local_time = localtime(&timestamp);
  if (local_time == NULL) {
    return -2;
  }
  *result = *local_time;
  return 0;
}

int AqvmBaseTime_GetCurrentTimeString(char* result) {
  time_t current_time = time(NULL);
  strftime(result, 28, "%Y-%m-%dT%H:%M:%S%z", localtime(&current_time));
  return 0;
}