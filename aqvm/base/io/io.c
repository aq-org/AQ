// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/io/io.h"

#include <stdarg.h>
#include <stdio.h>

#include "aqvm/base/process/file_lock/file_lock.h"
#include "aqvm/base/threading/mutex/mutex.h"

AqvmBaseThreadingMutex_Mutex AqvmBaseIo_printMutex;

int AqvmBaseIo_InitializeIo() {
  if (AqvmBaseThreadingMutex_InitializeMutex(&AqvmBaseIo_printMutex) != 0) {
    // TODO
    return -1;
  }
  return 0;
}

int AqvmBaseIo_CloseIo() {
  if (AqvmBaseThreadingMutex_CloseMutex(&AqvmBaseIo_printMutex) != 0) {
    // TODO
    return -1;
  }
  return 0;
}

int AqvmBaseIo_fprintf(FILE* const stream, const char* format, ...) {
  if (stream == NULL || format == NULL) {
    // TODO
    return -1;
  }
  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_LockMutex(&AqvmBaseIo_printMutex) != 0) {
      // TODO
      return -2;
    }
  } else {
    if (AqvmBaseProcessFileLock_LockFile(stream) != 0) {
      // TODO
      return -2;
    }
  }

  va_list arg;
  va_start(arg, format);
  int result = vfprintf(stream, format, arg);
  if (result < 0) {
    // TODO
    return -3;
  }
  va_end(arg);

  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBaseIo_printMutex)) {
      // TODO
      return -4;
    }
  } else {
    if (AqvmBaseProcessFileLock_UnlockFile(stream)) {
      // TODO
      return -4;
    }
  }

  return 0;
}

int AqvmBaseIo_fscanf(FILE* stream, const char* format, ...) {}

int AqvmBaseIo_perror(const char* str) {}

int AqvmBaseIo_printf(const char* format, ...) {
  if (format == NULL) {
    return -1;
  }
  if (AqvmBaseThreadingMutex_LockMutex(&AqvmBaseIo_printMutex) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vprintf(format, arg);
  if (result < 0) {
    // TODO
    return -3;
  }
  va_end(arg);

  if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBaseIo_printMutex)) {
    // TODO
    return -4;
  }

  return 0;
}

int AqvmBaseIo_scanf(const char* format, ...) {}

int AqvmBaseIo_snprintf(char* str, size_t size, const char* format, ...) {}

int AqvmBaseIo_sprintf(char* str, const char* format, ...) {}

int AqvmBaseIo_sscanf(const char* str, const char* format, ...) {}

int AqvmBaseIo_vfprintf(FILE* const stream, const char* format, va_list arg) {
  if (stream == NULL || format == NULL) {
    // TODO
    return -1;
  }
  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_LockMutex(&AqvmBaseIo_printMutex) != 0) {
      // TODO
      return -2;
    }
  } else {
    if (AqvmBaseProcessFileLock_LockFile(stream) != 0) {
      // TODO
      return -2;
    }
  }

  int result = vfprintf(stream, format, arg);
  if (result < 0) {
    // TODO
    return -3;
  }
  va_end(arg);

  if (stream == stdout || stream == stderr || stream == stdin) {
    if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBaseIo_printMutex)) {
      // TODO
      return -4;
    }
  } else {
    if (AqvmBaseProcessFileLock_UnlockFile(stream)) {
      // TODO
      return -4;
    }
  }

  return 0;
}

int AqvmBaseIo_vprintf(const char* format, va_list arg) {
  if (format == NULL) {
    return -1;
  }
  if (AqvmBaseThreadingMutex_LockMutex(&AqvmBaseIo_printMutex) != 0) {
    // TODO
    return -2;
  }

  int result = vprintf(format, arg);
  if (result < 0) {
    // TODO
    return -3;
  }
  va_end(arg);

  if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBaseIo_printMutex)) {
    // TODO
    return -4;
  }

  return 0;
}

int AqvmBaseIo_vsprintf(char* str, const char* format, va_list arg) {}

int AqvmBaseIo_fgetc(FILE* stream) {}

char* AqvmBaseIo_fgets(char* str, int n, FILE* stream) {}

int AqvmBaseIo_fputc(int character, FILE* stream) {}

int AqvmBaseIo_fputs(const char* str, FILE* stream) {}

int AqvmBaseIo_getc(FILE* stream) {}

int AqvmBaseIo_getchar(void) {}

char* AqvmBaseIo_gets(char* str) {}

int AqvmBaseIo_putc(int character, FILE* stream) {}

int AqvmBaseIo_putchar(int character) {}

int AqvmBaseIo_puts(const char* str) {}

int AqvmBaseIo_ungetc(int character, FILE* stream) {}





