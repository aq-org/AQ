#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/time/windows/time.h"

#include <stdio.h>
#include <time.h>
#include <windows.h>

#include "aqvm/base/logging/logging.h"
#include "aqvm/base/time/time.h"

int AqvmBaseTimeWindows_localtime(const time_t timestamp,
                                  struct AqvmBaseTime_Time* result) {
  if (timestamp == (time_t)-1 || result == NULL) {
    // TODO
    return -1;
  }

  struct tm current_time;
  if (localtime_s(&current_time, &timestamp) != 0) {
    // TODO
    return -2;
  }
  if (AqvmBaseTime_ConvertTmToTime(&current_time, result) != 0) {
    // TODO
    return -3;
  }

  return 0;
}

int AqvmBaseTimeWindows_gmtime(const time_t timestamp,
                               struct AqvmBaseTime_Time* result) {
  if (timestamp == (time_t)-1 || result == NULL) {
    // TODO
    return -1;
  }

  struct tm current_time;
  if (gmtime_s(&current_time, &timestamp) != 0) {
    // TODO
    return -2;
  }
  if (AqvmBaseTime_ConvertTmToTime(&current_time, result) != 0) {
    // TODO
    return -3;
  }

  return 0;
}

int AqvmBaseTimeWindows_ConvertSystemtimeToTime(
    const SYSTEMTIME* time, struct AqvmBaseTime_Time* result) {
  if (time == NULL || result == NULL) {
    // TODO
    return -1;
  }

  result->year = (int)time->wYear;
  result->month = (uint8_t)time->wMonth;
  result->day = (uint8_t)time->wDay;
  result->hour = (uint8_t)time->wHour;
  result->minute = (uint8_t)time->wMinute;
  result->second = (uint8_t)time->wSecond;
  result->millisecond = time->wMilliseconds;
  result->weekday = (uint8_t)time->wDayOfWeek;
  result->offset_sign = 0;
  result->offset_hour = 0;
  result->offset_minute = 0;
  result->yearday = 0;
  result->isdst = -1;
  return 0;
}

int AqvmBaseTimeWindows_GetCurrentTime(struct AqvmBaseTime_Time* result) {
  if (result == NULL) {
    // TODO
    return -1;
  }

  SYSTEMTIME st;
  GetLocalTime(&st);
  if (AqvmBaseTimeWindows_ConvertSystemtimeToTime(&st, result) != 0) {
    // TODO
    return -2;
  }

  if (AqvmBaseTime_SetWeekday(result) != 0) {
    // TODO
    return -3;
  }
  if (AqvmBaseTime_SetYearday(result) != 0) {
    // TODO
    return -4;
  }
  if (AqvmBaseTime_SetIsdst(result) != 0) {
    // TODO
    // return -5;
  }
  if (AqvmBaseTime_SetTimezoneOffset(result) != 0) {
    // TODO
    // return -6;
  }

  return 0;
}
#endif