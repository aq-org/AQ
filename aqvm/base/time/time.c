// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/time/time.h"

#include <time.h>

#include "aqvm/base/logging/logging.h"

#ifdef __unix__
#include "aqvm/base/time/unix/time.h"
#elif _WIN32
#include "aqvm/base/time/windows/time.h"
#endif

int AqvmBaseTime_localtime(const time_t timestamp, struct tm* result) {
#ifdef __unix__
  if (AqvmBaseTimeUnix_localtime(timestamp, result)) {
    // TODO
    return -1;
  }
#elif _WIN32
  if (AqvmBaseTimeWindows_localtime(timestamp, result)) {
    // TODO
    return -2;
  }
#else
  // TODO
  AqvmBaseLogging_OutputLog(
      "WARNING", "AqvmBaseTime_localtime_ThreadUnsafeWarning",
      "The localtime function may cause thread unsafety.", NULL);
  struct tm* local_time = localtime(&timestamp);
  if (local_time == NULL) {
    return -2;
  }
  *result = *local_time;
#endif

  return 0;
}

int AqvmBaseTime_GetCurrentTimeString(char* result) {
  if(result == NULL){
    // TODO
    return -1;
  }

  struct tm local_time;
  if(AqvmBaseTime_localtime(time(NULL), &local_time)){
    // TODO
    return -2;
  }
  strftime(result, 28, "%Y-%m-%dT%H:%M:%S%z", &local_time);
  return 0;
}