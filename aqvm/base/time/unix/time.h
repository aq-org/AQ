#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_TIME_POSIX_TIME_H_
#define AQ_AQVM_BASE_TIME_POSIX_TIME_H_

#include <time.h>

int AqvmBaseTimePosix_localtime(const time_t timestamp, struct tm* result);

#endif
#endif