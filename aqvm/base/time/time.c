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
  if (timestamp == (time_t)-1 || result == NULL) {
    // TODO(logging)
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseTimeUnix_localtime(timestamp, result) != 0) {
    // TODO(logging)
    return -2;
  }
#elif _WIN32
  if (AqvmBaseTimeWindows_localtime(timestamp, result) != 0) {
    // TODO(logging)
    return -3;
  }
#else
  // TODO(logging)
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
    // TODO(logging)
    return -5;
  }
  int timezone_offset = (result->hour - utc_time.hour) * 3600 +
                        (result->minute - utc_time.minute) * 60 +
                        (result->second - utc_time.second);
  if (result->day != utc_time.day)
    timezone_offset += result->day > utc_time.day ? 24 * 3600 : -24 * 3600;
  if (timezone_offset == 0) {
    result->offset_sign = 0;
  } else {
    int offset_hour = timezone_offset / 3600;
    int offset_minute = timezone_offset % 3600 / 60;
    result->offset_sign = timezone_offset > 0 ? 1 : -1;
    result->offset_hour = offset_hour >= 0 ? offset_hour : -offset_hour;
    result->offset_minute = offset_minute >= 0 ? offset_minute : -offset_minute;
  }

  return 0;
}

int AqvmBaseTime_gmtime(const time_t timestamp,
                        struct AqvmBaseTime_Time* result) {
  if (timestamp == (time_t)-1 || result == NULL) {
    // TODO(logging)
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseTimeUnix_gmtime(timestamp, result) != 0) {
    // TODO(logging)
    return -2;
  }
#elif _WIN32
  if (AqvmBaseTimeWindows_gmtime(timestamp, result) != 0) {
    // TODO(logging)
    return -3;
  }
#else
  // TODO(logging)
  AqvmBaseLogging_OutputLog(
      "WARNING", "AqvmBaseTime_gmtime_ThreadUnsafeWarning",
      "The gmtime function may cause thread unsafety.", NULL);
  struct tm* gm_time = gmtime(&timestamp);
  if (gm_time == NULL) {
    // TODO(logging)
    return -4;
  }
  *result = *gm_time;
#endif

  return 0;
}

time_t AqvmBaseTime_mktime(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO(logging)
    return (time_t)-1;
  }

  struct tm tm;
  AqvmBaseTime_ConvertTimeToTm(time_info, &tm);
  return mktime(&tm);
}

int AqvmBaseTime_ConvertTmToTime(const struct tm* time_info,
                                 struct AqvmBaseTime_Time* result) {
  if (time_info == NULL || result == NULL) {
    // TODO(logging)
    return -1;
  }

  result->year = time_info->tm_year + 1900;
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
  if (!AqvmBaseTime_IsValidTime(time_info) || time_info->year < 1900 ||
      result == NULL) {
    // TODO(logging)
    return -1;
  }
  result->tm_year = time_info->year - 1900;
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

int AqvmBaseTime_GetCurrentTime(struct AqvmBaseTime_Time* result) {
  if (result == NULL) {
    // TODO(logging)
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseTimeUnix_GetCurrentTime(result) != 0) {
    // TODO(logging)
    return -2;
  }
#elif _WIN32
  if (AqvmBaseTimeWindows_GetCurrentTime(result) != 0) {
    // TODO(logging)
    return -3;
  }
#else
  if (AqvmBaseTime_localtime(time(NULL), result) != 0) {
    // TODO(logging)
    return -4;
  }
#endif

  return 0;
}

int AqvmBaseTime_GetCurrentTimeString(char* result) {
  if (result == NULL) {
    // TODO(logging)
    return -1;
  }

  struct AqvmBaseTime_Time current_time;
  if (AqvmBaseTime_GetCurrentTime(&current_time) != 0) {
    // TODO(logging)
    return -2;
  }
  if (!AqvmBaseTime_IsValidTime(&current_time)) {
    // TODO(logging)
    return -3;
  }
  char timezone_offset_string[7];
  if (AqvmBaseTime_GetTimezoneOffsetString(&current_time,
                                           timezone_offset_string) != 0) {
    // TODO(logging)
    // return -4;
  }
  if (current_time.year < 0) {
    if (AqvmBaseIo_snprintf(
            result, 31, "-%04d-%02d-%02dT%02d:%02d:%02d.%03d%s",
            current_time.year, current_time.month, current_time.day,
            current_time.hour, current_time.minute, current_time.second,
            current_time.millisecond, timezone_offset_string) < 0) {
      // TODO(logging)
      return -5;
    }
  } else {
    if (AqvmBaseIo_snprintf(
            result, 30, "%04d-%02d-%02dT%02d:%02d:%02d.%03d%s",
            current_time.year, current_time.month, current_time.day,
            current_time.hour, current_time.minute, current_time.second,
            current_time.millisecond, timezone_offset_string) < 0) {
      // TODO(logging)
      return -6;
    }
  }
  return 0;
}

int AqvmBaseTime_GetTimezoneOffsetString(
    const struct AqvmBaseTime_Time* time_info, char* result) {
  if (!AqvmBaseTime_IsValidTime(time_info) || result == NULL) {
    // TODO(logging)
    return -1;
  }

  if (time_info->offset_sign == 0 && AqvmBaseIo_snprintf(result, 2, "Z") < 0) {
    // TODO(logging)
    return -2;
  } else if (time_info->offset_sign > 0 &&
             AqvmBaseIo_snprintf(result, 7, "+%02d:%02d",
                                 time_info->offset_hour,
                                 time_info->offset_minute) < 0) {
    // TODO(logging)
    return -3;
  } else if (time_info->offset_sign < 0 &&
             AqvmBaseIo_snprintf(result, 7, "-%02d:%02d",
                                 time_info->offset_hour,
                                 time_info->offset_minute) < 0) {
    // TODO(logging)
    return -4;
  }
  return 0;
}

bool AqvmBaseTime_IsLeapYear(const struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO(logging)
    return false;
  }

  return ((time_info->year % 4 == 0 && time_info->year % 100 != 0) ||
          time_info->year % 400 == 0);
}

bool AqvmBaseTime_IsValidTime(const struct AqvmBaseTime_Time* time_info) {
  if (time_info == NULL) {
    // TODO(logging)
    return false;
  }
  if (time_info->year == 0) {
    // TODO(logging)
    return false;
  }
  if (time_info->month > 12) {
    // TODO(logging)
    return false;
  }
  if (time_info->day > 31) {
    // TODO(logging)
    return false;
  }
  if (time_info->hour > 23) {
    // TODO(logging)
    return false;
  }
  if (time_info->minute > 59) {
    // TODO(logging)
    return false;
  }
  if (time_info->second > 60) {
    // TODO(logging)
    return false;
  }
  if (time_info->millisecond > 999) {
    // TODO(logging)
    return false;
  }
  if (time_info->weekday > 6) {
    // TODO(logging)
    return false;
  }
  if (time_info->yearday > 366) {
    // TODO(logging)
    return false;
  }
  if (time_info->offset_hour > 23) {
    // TODO(logging)
    return false;
  }
  if (time_info->offset_minute > 59) {
    // TODO(logging)
    return false;
  }
  return true;
}

int AqvmBaseTime_SetIsdst(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO(logging)
    return -1;
  }

  time_t timestamp = AqvmBaseTime_mktime(time_info);
  if (timestamp == (time_t)-1) {
    // TODO(logging)
    return -2;
  }
  struct AqvmBaseTime_Time local_time;
  if (AqvmBaseTime_localtime(timestamp, &local_time) != 0) {
    // TODO(logging)
    return -3;
  }
  time_info->isdst = local_time.isdst;
  return 0;
}

int AqvmBaseTime_SetTimezoneOffset(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO(logging)
    return -1;
  }

  time_t timestamp = AqvmBaseTime_mktime(time_info);
  if (timestamp == (time_t)-1) {
    // TODO(logging)
    return -2;
  }
  struct AqvmBaseTime_Time local_time;
  if (AqvmBaseTime_localtime(timestamp, &local_time) != 0) {
    // TODO(logging)
    return -3;
  }
  time_info->offset_sign = local_time.offset_sign;
  time_info->offset_hour = local_time.offset_hour;
  time_info->offset_minute = local_time.offset_minute;
  return 0;
}

int AqvmBaseTime_SetWeekday(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO(logging)
    return -1;
  }

  int c = time_info->year / 100;
  int y = time_info->year % 100;
  int m = time_info->month;
  int d = time_info->day;

  if (time_info->year == 1582 && time_info->month == 10 && time_info->day > 4 &&
      time_info->day < 15) {
    // TODO(logging)
    return -2;
  }

  if (y < 0) ++y;
  if (m < 3) {
    m += 12;
    --y;
  }

  if (time_info->year < 1582 ||
      (time_info->year == 1582 && time_info->month < 10) ||
      (time_info->year == 1582 && time_info->month == 10 &&
       time_info->day <= 4))
    time_info->weekday = (-c + y + y / 4 + 13 * (m + 1) / 5 + d + 4) % 7;

  time_info->weekday =
      (c / 4 - 2 * c + y + y / 4 + 13 * (m + 1) / 5 + d - 1) % 7;

  return 0;
}

int AqvmBaseTime_SetYearday(struct AqvmBaseTime_Time* time_info) {
  if (!AqvmBaseTime_IsValidTime(time_info)) {
    // TODO(logging)
    return -1;
  }

  int days[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  time_info->yearday = days[time_info->month - 1] + time_info->day;
  if (time_info->month > 2 && AqvmBaseTime_IsLeapYear(time_info))
    ++time_info->yearday;
  return 0;
}