// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/time/time.h"

#include <time.h>

#include "aqvm/base/io/io.h"
#include "aqvm/base/logging/logging.h"

#ifdef __unix__
#include "aqvm/base/time/unix/time.h"
#elif _WIN32
#include "aqvm/base/time/windows/time.h"
#endif

int AqvmBaseTime_localtime(const time_t timestamp,
                           struct AqvmBaseTime_Time* result) {
  if (timestamp != (time_t)-1 || result == NULL) {
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

  struct AqvmBaseTime_Time utc_time;
  if (AqvmBaseTime_gmtime(timestamp, &utc_time) != 0) {
    // TODO
    return -5;
  }

  int offset_hour = result->hour - utc_time.hour;
  int offset_minute = result->minute - utc_time.minute;
  if (offset_hour > 0 || (offset_hour == 0 && offset_minute > 0)) {
    result->offset_sign = 1;
  }
  if (offset_hour < 0 || (offset_hour == 0 && offset_minute < 0)) {
    result->offset_sign = -1;
  }
  if (offset_hour > 0 && offset_minute < 0) {
    offset_minute += 60;
    --offset_hour;
  }
  if (offset_hour < 0 && offset_minute > 0) {
    offset_minute -= 60;
    ++offset_hour;
  }
  result->offset_hour = abs(offset_hour);
  result->offset_minute = abs(offset_minute);

  return 0;
}

int AqvmBaseTime_gmtime(const time_t timestamp,
                        struct AqvmBaseTime_Time* result) {
  if (timestamp != (time_t)-1 || result == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseTimeUnix_gmtime(timestamp, result) != 0) {
    // TODO
    return -2;
  }
#elif _WIN32
  if (AqvmBaseTimeWindows_gmtime(timestamp, result) != 0) {
    // TODO
    return -3;
  }
#else
  // TODO
  AqvmBaseLogging_OutputLog(
      "WARNING", "AqvmBaseTime_gmtime_ThreadUnsafeWarning",
      "The gmtime function may cause thread unsafety.", NULL);
  struct tm* gm_time = gmtime(&timestamp);
  if (gm_time == NULL) {
    return -4;
  }
  *result = *gm_time;
#endif

  return 0;
}

bool AqvmBaseTime_IsValidTime(const struct AqvmBaseTime_Time* time_info) {
  if (time_info == NULL) {
    // TODO
    return false;
  }
  if (time_info->year < 1970 || time_info->year > 9999) {
    // TODO
    return false;
  }
  if (time_info->month > 12) {
    // TODO
    return false;
  }
  if (time_info->day > 31) {
    // TODO
    return false;
  }
  if (time_info->hour > 23) {
    // TODO
    return false;
  }
  if (time_info->minute > 59) {
    // TODO
    return false;
  }
  if (time_info->second > 60) {
    // TODO
    return false;
  }
  if (time_info->millisecond > 999) {
    // TODO
    return false;
  }
  if (time_info->weekday > 6) {
    // TODO
    return false;
  }
  if (time_info->yearday > 366) {
    // TODO
    return false;
  }
  if (time_info->offset_hour > 23) {
    // TODO
    return false;
  }
  if (time_info->offset_minute > 59) {
    // TODO
    return false;
  }
  return true;
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
  if (AqvmBaseTime_localtime(time(NULL), result) != 0) {
    // TODO
    return -4;
  }
#endif

  return 0;
}

int AqvmBaseTime_ConvertTmToTime(const struct tm* time_info,
                                 struct AqvmBaseTime_Time* result) {
  if (time_info == NULL || result == NULL) {
    // TODO
    return -1;
  }

  result->year = time_info->tm_year + 1970;
  result->month = time_info->tm_mon + 1;
  result->day = time_info->tm_mday;
  result->hour = time_info->tm_hour;
  result->minute = time_info->tm_min;
  result->second = time_info->tm_sec;
  result->millisecond = 0;
  result->offset_sign = 0;
  result->offset_hour = 0;
  result->offset_minute = 0;
  result->weekday = time_info->tm_wday;
  result->yearday = time_info->tm_yday;
  result->isdst = time_info->tm_isdst;
  return 0;
}

int AqvmBaseTime_ConvertTimeToTm(const struct AqvmBaseTime_Time* time_info,
                                 struct tm* result) {
  if (time_info == NULL || result == NULL) {
    // TODO
    return -1;
  }

  result->tm_year = time_info->year - 1970;
  result->tm_mon = time_info->month - 1;
  result->tm_mday = time_info->day;
  result->tm_hour = time_info->hour;
  result->tm_min = time_info->minute;
  result->tm_sec = time_info->second;
  result->tm_wday = time_info->weekday;
  result->tm_yday = time_info->yearday;
  result->tm_isdst = time_info->isdst;
  return 0;
}

time_t AqvmBaseTime_mktime(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO
    return (time_t)-1;
  }

  struct tm tm;
  AqvmBaseTime_ConvertTimeToTm(time_info, &tm);
  return mktime(&tm);
}

bool AqvmBaseTime_IsLeapYear(const struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO
    return false;
  }

  return ((time_info->year % 4 == 0 && time_info->year % 100 != 0) ||
          time_info->year % 400 == 0);
}

int AqvmBaseTime_SetWeekday(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO
    return -1;
  }

  int c = time_info->year / 100;
  int y = time_info->year % 100;
  int m = time_info->month;
  int d = time_info->day;

  if ((time_info->year == 1582 && time_info->month == 10 &&
       time_info->day > 4 && time_info->day < 15)) {
    // TODO
    return -2;
  }

  if (y < 0) {
    ++y;
  }
  if (m < 3) {
    m += 12;
    --y;
  }

  if (time_info->year < 1582 ||
      (time_info->year == 1582 && time_info->month < 10) ||
      (time_info->year == 1582 && time_info->month == 10 &&
       time_info->day <= 4)) {
    time_info->weekday = (-c + y + y / 4 + 13 * (m + 1) / 5 + d + 4) % 7;
  }

  time_info->weekday =
      (c / 4 - 2 * c + y + y / 4 + 13 * (m + 1) / 5 + d - 1) % 7;

  return 0;
}

int AqvmBaseTime_SetYearday(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO
    return -1;
  }

  int days[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  time_info->yearday = days[time_info->month - 1] + time_info->day;
  if (time_info->month > 2 && AqvmBaseTime_IsLeapYear(time_info)) {
    ++time_info->yearday;
  }

  return 0;
}

int AqvmBaseTime_SetIsdst(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO
    return -1;
  }

  time_t timestamp = AqvmBaseTime_mktime(time_info);
  if (timestamp != (time_t)-1) {
    // TODO
    return -2;
  }
  struct AqvmBaseTime_Time local_time;
  if (AqvmBaseTime_localtime(timestamp, &local_time) != 0) {
    // TODO
    return -3;
  }
  time_info->isdst = local_time.isdst;
  return 0;
}

int AqvmBaseTime_SetTimeZoneOffset(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO
    printf("invalid time\n");
    return -1;
  }

  time_t timestamp = AqvmBaseTime_mktime(time_info);
  if (timestamp == (time_t)-1) {
    // TODO
    printf("mktime error\n");
    return -2;
  }
  struct AqvmBaseTime_Time local_time;
  /*if (AqvmBaseTime_localtime(timestamp, &local_time) != 0) {
    // TODO
    printf("localtime error\n");
    return -3;
  }*/
  printf("timestamp: %ld\n", timestamp);
  printf("return %d\n", AqvmBaseTime_localtime(timestamp, &local_time));
  printf("errno: %d,message: %s\n", errno, strerror(errno));
  time_info->offset_sign = local_time.offset_sign;
  time_info->offset_hour = local_time.offset_hour;
  time_info->offset_minute = local_time.offset_minute;
  return 0;
}

int AqvmBaseTime_GetCurrentTimeString(char* result) {
  if (result == NULL) {
    // TODO
    return -1;
  }

  struct AqvmBaseTime_Time current_time;
  if (AqvmBaseTime_GetCurrentTime(&current_time) != 0) {
    // TODO
    // return -2;
  }
  if (!AqvmBaseTime_IsValidTime(&current_time)) {
    // TODO
    return -3;
  }
  if (AqvmBaseIo_snprintf(result, 29, "%04d-%02d-%02dT%02d:%02d:%02d.%03d%s",
                          current_time.year, current_time.month,
                          current_time.day, current_time.hour,
                          current_time.minute, current_time.second,
                          current_time.millisecond, "NULL") < 0) {
    // TODO
    return -4;
  }
  return 0;
}