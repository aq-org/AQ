#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/time/unix/time.h"

#include <sys/time.h>
#include <time.h>

#include "aqvm/base/logging/logging.h"
#include "aqvm/base/time/time.h"

int AqvmBaseTimeUnix_localtime(const time_t timestamp,
                               struct AqvmBaseTime_Time* result) {
  if (timestamp == (time_t)-1 || result == NULL) {
    // TODO(logging)
    return -1;
  }

  struct tm current_time;
  if (localtime_r(&timestamp, &current_time) == NULL) {
    // TODO(logging)
    return -2;
  }
  if (AqvmBaseTime_ConvertTmToTime(&current_time, result) != 0) {
    // TODO(logging)
    return -3;
  }

  return 0;
}

int AqvmBaseTimeUnix_gmtime(const time_t timestamp,
                            struct AqvmBaseTime_Time* result) {
  if (timestamp == (time_t)-1 || result == NULL) {
    // TODO(logging)
    return -1;
  }

  struct tm current_time;
  if (gmtime_r(&timestamp, &current_time) == NULL) {
    // TODO(logging)
    return -2;
  }
  if (AqvmBaseTime_ConvertTmToTime(&current_time, result) != 0) {
    // TODO(logging)
    return -3;
  }

  return 0;
}

int AqvmBaseTimeUnix_ConvertTimespecToTime(const struct timespec* time_info,
                                           struct AqvmBaseTime_Time* result) {
  if (result == NULL || time_info == NULL) {
    // TODO(logging)
    return -1;
  }

  if (AqvmBaseTime_localtime(time_info->tv_sec, result) != 0) {
    // TODO(logging)
    return -2;
  }
  result->millisecond = time_info->tv_nsec / 1000000;

  return 0;
}

int AqvmBaseTimeUnix_ConvertTimevalToTime(const struct timeval* time_info,
                                          struct AqvmBaseTime_Time* result) {
  if (result == NULL || time_info == NULL) {
    // TODO(logging)
    return -1;
  }

  if (AqvmBaseTime_localtime(time_info->tv_sec, result) != 0) {
    // TODO(logging)
    return -2;
  }
  result->millisecond = time_info->tv_usec / 1000;

  return 0;
}

int AqvmBaseTimeUnix_GetCurrentTime(struct AqvmBaseTime_Time* result) {
  if (result == NULL) {
    // TODO(logging)
    return -1;
  }
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
    // TODO(logging)
    return -2;
  }
  if (AqvmBaseTimeUnix_ConvertTimespecToTime(&ts, result) != 0) {
    // TODO(logging)
    return -3;
  }
#else
  struct timeval tv;
  if (gettimeofday(&tv, NULL) != 0) {
    // TODO(logging)
    return -4;
  }
  if (AqvmBaseTimeUnix_ConvertTimevalToTime(&tv, result) != 0) {
    // TODO(logging)
    return -5;
  }
#endif
  return 0;
}
#endif