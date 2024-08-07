// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_TIME_TIME_H_
#define AQ_AQVM_BASE_TIME_TIME_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

struct AqvmBaseTime_Time {
  int year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint16_t millisecond;
  int8_t offset_sign;
  uint8_t offset_hour;
  uint8_t offset_minute;
  uint8_t weekday;
  uint16_t yearday;
  int8_t isdst;
};

int AqvmBaseTime_localtime(const time_t timestamp,
                           struct AqvmBaseTime_Time* result);

int AqvmBaseTime_gmtime(const time_t timestamp,
                        struct AqvmBaseTime_Time* result);

time_t AqvmBaseTime_mktime(struct AqvmBaseTime_Time* time_info);

int AqvmBaseTime_ConvertTmToTime(const struct tm* time_info,
                                 struct AqvmBaseTime_Time* result);

int AqvmBaseTime_ConvertTimeToTm(const struct AqvmBaseTime_Time* time_info,
                                 struct tm* result);

int AqvmBaseTime_GetCurrentTime(struct AqvmBaseTime_Time* result);

int AqvmBaseTime_GetCurrentTimeString(char* result);

int AqvmBaseTime_GetTimezoneOffsetString(
    const struct AqvmBaseTime_Time* time_info, char* result);

bool AqvmBaseTime_IsLeapYear(const struct AqvmBaseTime_Time* time_info);

bool AqvmBaseTime_IsValidTime(const struct AqvmBaseTime_Time* time_info);

int AqvmBaseTime_SetIsdst(struct AqvmBaseTime_Time* time_info);

int AqvmBaseTime_SetTimezoneOffset(struct AqvmBaseTime_Time* time_info);

int AqvmBaseTime_SetWeekday(struct AqvmBaseTime_Time* time_info);

int AqvmBaseTime_SetYearday(struct AqvmBaseTime_Time* time_info);

#endif