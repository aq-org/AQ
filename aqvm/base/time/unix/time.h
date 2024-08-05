#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_TIME_UNIX_TIME_H_
#define AQ_AQVM_BASE_TIME_UNIX_TIME_H_

#include <time.h>

#include "aqvm/base/time/time.h"

int AqvmBaseTimeUnix_localtime(const time_t timestamp, struct tm* result);

int AqvmBaseTimeUnix_GetCurrentTime(struct AqvmBaseTime_Time* result);

#endif
#endif