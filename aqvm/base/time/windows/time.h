#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_TIME_WINDOWS_TIME_H_
#define AQ_AQVM_BASE_TIME_WINDOWS_TIME_H_

#include <time.h>
#include <windows.h>

#include "aqvm/base/time/time.h"

int AqvmBaseTimeWindows_localtime(const time_t timestamp,
                                  struct AqvmBaseTime_Time* result);

int AqvmBaseTimeWindows_gmtime(const time_t timestamp,
                               struct AqvmBaseTime_Time* result);

int AqvmBaseTimeWindows_ConvertSystemtimeToTime(
    const SYSTEMTIME* time, struct AqvmBaseTime_Time* result);

int AqvmBaseTimeWindows_GetCurrentTime(struct AqvmBaseTime_Time* result);

#endif
#endif