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
  if (result == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseTimeUnix_localtime(timestamp, result) != 0) {
    // TODO
    return -2;
  }
#elif _WIN32
  if (AqvmBaseTimeWindows_localtime(timestamp, result) != 0) {
    // TODO
    return -3;
  }
#else
  // TODO
  AqvmBaseLogging_OutputLog(
      "WARNING", "AqvmBaseTime_localtime_ThreadUnsafeWarning",
      "The localtime function may cause thread unsafety.", NULL);
  struct tm* local_time = localtime(&timestamp);
  if (local_time == NULL) {
    return -4;
  }
  *result = *local_time;
#endif

  return 0;
}

int AqvmBaseTime_GetCurrentTime(struct AqvmBaseTime_Time* result) {
  if (result == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseTimeUnix_GetCurrentTime(result) != 0) {
    // TODO
    return -2;
  }
#elif _WIN32
  if (AqvmBaseTimeWindows_GetCurrentTime(result) != 0) {
    // TODO
    return -3;
  }
#else
  struct tm local_time;
  if (AqvmBaseTime_localtime(time(NULL), &local_time) != 0) {
    // TODO
    return -4;
  }
  result->year = local_time.tm_year + 1900;
  result->month = local_time.tm_mon + 1;
  result->day = local_time.tm_mday;
  result->hour = local_time.tm_hour;
  result->minute = local_time.tm_min;
  result->second = local_time.tm_sec;
  result->millisecond = 0;
  result->weekday = local_time.tm_wday;
  result->yearday = local_time.tm_yday;
  result->isdst = local_time.tm_isdst;
#endif

  return 0;
}

int AqvmBaseTime_ConvertTmToTime(){

}

int AqvmBaseTime_GetCurrentTimeString(char* result) {
  if (result == NULL) {
    // TODO
    return -1;
  }

  struct tm local_time;
  if (AqvmBaseTime_localtime(time(NULL), &local_time)) {
    // TODO
    return -2;
  }
  strftime(result, 28, "%Y-%m-%dT%H:%M:%S%z", &local_time);
  return 0;
}