// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_TIME_TIME_H_
#define AQ_AQVM_BASE_TIME_TIME_H_

#include <time.h>

struct AqvmBaseTime_Time {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int millisecond;
  int weekday;
  int yearday;
  int isdst;
};

int AqvmBaseTime_localtime(const time_t timestamp, struct tm* result);

int AqvmBaseTime_GetCurrentTime(struct AqvmBaseTime_Time* result);

int AqvmBaseTime_ConvertTmToTime(char* result);

// Get the current time. The current time is then formatted as an ISO 8601
// compliant string and written to |result|. |result| must not be NULL and must
// be at least 28 characters in length. Returns 0 if successful.
// The format of the string is YYYY-MM-DDThh:mm:ss.mmm[Z/+hhmm/-hhmm]. Each part
// cannot be omitted. The final time offset is set according to the current
// system time zone.
int AqvmBaseTime_GetCurrentTimeString(char* result);

#endif