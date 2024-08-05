#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/time/windows/time.h"

#include <Windows.h>
#include <time.h>

#include "aqvm/base/logging/logging.h"
#include "aqvm/base/time/time.h"

int AqvmBaseTimeWindows_localtime(const time_t timestamp, struct tm* result) {
  if (localtime_s(result, &timestamp) != 0) {
    // TODO
    return -1;
  }
  return 0;
}

int AqvmBaseTimeWindows_GetCurrentTime(struct AqvmBaseTime_Time* result) {
  if (result == NULL) {
    // TODO
    return -1;
  }


  return 0;
}
#endif