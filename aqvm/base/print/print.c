// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/print/print.h"

#include <stdarg.h>
#include <stdio.h>

#include "aqvm/base/threading/file_lock/file_lock.h"
#include "aqvm/base/threading/mutex/mutex.h"

AqvmBaseThreadingMutex_Mutex AqvmBasePrint_printMutex;

int AqvmBasePrint_InitializePrint() {
  if (AqvmBaseThreadingMutex_InitializeMutex(&AqvmBasePrint_printMutex) != 0) {
    // TODO
    return -1;
  }
  return 0;
}

int AqvmBasePrint_ClosePrint() {
  if (AqvmBaseThreadingMutex_CloseMutex(&AqvmBasePrint_printMutex) != 0) {
    // TODO
    printf("ERROR IN PRINT CLOSE\n %i",AqvmBaseThreadingMutex_CloseMutex(&AqvmBasePrint_printMutex));
    return -1;
  }
  return 0;
}

int AqvmBasePrint_printf(const char* format, ...) {
  if (AqvmBaseThreadingMutex_LockMutex(&AqvmBasePrint_printMutex) != 0) {
    // TODO
    return -1;
  }

  va_list args;
  va_start(args, format);
  int result = vprintf(format, args);
  if (result < 0) {
    // TODO
    return -2;
  }
  va_end(args);

  if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBasePrint_printMutex)) {
    // TODO
    return -3;
  }

  return 0;
}

int AqvmBasePrint_fprintf(FILE* const stream, const char* format, ...) {
  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_LockMutex(&AqvmBasePrint_printMutex) != 0) {
      // TODO
      return -1;
    }
  } else {
    if (AqvmBaseThreadingFileLock_LockFile(stream) != 0) {
      // TODO
      return -1;
    }
  }

  va_list args;
  va_start(args, format);
  int result = vfprintf(stream, format, args);
  if (result < 0) {
    // TODO
    return -2;
  }
  va_end(args);

  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBasePrint_printMutex)) {
      // TODO
      return -3;
    }
  } else {
    if (AqvmBaseThreadingFileLock_UnlockFile(stream)) {
      // TODO
      return -3;
    }
  }

  return 0;
}

int AqvmBasePrint_vfprintf(FILE* const stream, const char* format,
                           va_list args) {
  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_LockMutex(&AqvmBasePrint_printMutex) != 0) {
      // TODO
      return -1;
    }
  } else {
    if (AqvmBaseThreadingFileLock_LockFile(stream) != 0) {
      // TODO
      return -1;
    }
  }

  int result = vfprintf(stream, format, args);
  if (result < 0) {
    // TODO
    return -2;
  }
  va_end(args);

  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBasePrint_printMutex)) {
      // TODO
      return -3;
    }
  } else {
    if (AqvmBaseThreadingFileLock_UnlockFile(stream)) {
      // TODO
      return -3;
    }
  }

  return 0;
}