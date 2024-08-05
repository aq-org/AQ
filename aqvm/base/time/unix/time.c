#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/time/unix/time.h"

#include <sys/time.h>
#include <time.h>

#include "aqvm/base/logging/logging.h"
#include "aqvm/base/time/time.h"

int AqvmBaseTimeUnix_localtime(const time_t timestamp, struct tm* result) {
  if (localtime_r(&timestamp, result) == NULL) {
    // TODO
    return -1;
  }
  return 0;
}

int AqvmBaseTimeUnix_GetCurrentTime(struct AqvmBaseTime_Time* result) {
  if (result == NULL) {
    // TODO
    return -1;
  }
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
    // TODO
    return -2;
  }
#else
  struct timeval tv;
  if (gettimeofday(&tv, NULL) != 0) {
    // TODO
    return -3;
  }
#endif
  return 0;
}
#endif