// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/io/io.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "aqvm/base/file/file.h"
#include "aqvm/base/process/file_lock/file_lock.h"
#include "aqvm/base/threading/mutex/mutex.h"

AqvmBaseThreadingMutex_Mutex AqvmBaseIo_printMutex;

struct AqvmBaseFile_File AqvmBaseIo_stdoutStream;
extern struct AqvmBaseFile_File* AqvmBaseIo_stdout = &AqvmBaseIo_stdoutStream;

struct AqvmBaseFile_File AqvmBaseIo_stdinStream;
extern struct AqvmBaseFile_File* AqvmBaseIo_stdin = &AqvmBaseIo_stdinStream;

struct AqvmBaseFile_File AqvmBaseIo_stderrStream;
extern struct AqvmBaseFile_File* AqvmBaseIo_stderr = &AqvmBaseIo_stderrStream;

int AqvmBaseIo_InitializeIo() {
  AqvmBaseIo_stdoutStream.file = stdout;
  AqvmBaseIo_stdinStream.file = stdin;
  AqvmBaseIo_stderrStream.file = stderr;

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

int AqvmBaseIo_fgetc(struct AqvmBaseFile_File* stream);

char* AqvmBaseIo_fgets(char* str, int n, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fprintf(struct AqvmBaseFile_File* stream, const char* format,
                       ...) {
  if (stream == NULL || stream->file == NULL || format == NULL) {
    // TODO
    return -1;
  }
  if (stream->file == stdout || stream->file == stderr ||
      stream->file == stdin) {
    if (AqvmBaseThreadingMutex_LockMutex(&AqvmBaseIo_printMutex) != 0) {
      // TODO
      return -2;
    }
  } else {
    if (AqvmBaseProcessFileLock_LockFile(stream) != 0) {
      // TODO
      return -3;
    }
    if (AqvmBaseFile_LockFile(stream) != 0) {
      // TODO
      return -4;
    }
  }

  va_list arg;
  va_start(arg, format);
  int result = vfprintf(stream->file, format, arg);
  if (result < 0) {
    // TODO
    return -5;
  }
  va_end(arg);

  if (stream->file == stdout || stream->file == stderr ||
      stream->file == stdin) {
    if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBaseIo_printMutex)) {
      // TODO
      return -6;
    }
  } else {
    if (AqvmBaseProcessFileLock_UnlockFile(stream)) {
      // TODO
      return -7;
    }
    if (AqvmBaseFile_UnlockFile(stream)) {
      // TODO
      return -8;
    }
  }

  return 0;
}

int AqvmBaseIo_fputc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fputs(const char* str, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fscanf(struct AqvmBaseFile_File* stream, const char* format, ...);

int AqvmBaseIo_getc(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_getchar(void);

char* AqvmBaseIo_gets(char* str);

int AqvmBaseIo_perror(const char* str);

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

int AqvmBaseIo_putc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_putchar(int character);

int AqvmBaseIo_puts(const char* str);

int AqvmBaseIo_scanf(const char* format, ...);

int AqvmBaseIo_snprintf(char* str, size_t size, const char* format, ...);

int AqvmBaseIo_sprintf(char* str, const char* format, ...);

int AqvmBaseIo_sscanf(const char* str, const char* format, ...);

int AqvmBaseIo_ungetc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_vfprintf(struct AqvmBaseFile_File* stream, const char* format,
                        va_list arg) {
  if (stream == NULL || stream->file == NULL || format == NULL) {
    // TODO
    return -1;
  }
  if (stream->file == stdout || stream->file == stderr ||
      stream->file == stdin) {
    if (AqvmBaseThreadingMutex_LockMutex(&AqvmBaseIo_printMutex) != 0) {
      // TODO
      return -2;
    }
  } else {
    if (AqvmBaseProcessFileLock_LockFile(stream) != 0) {
      // TODO
      return -3;
    }
    if (AqvmBaseFile_LockFile(stream) != 0) {
      // TODO
      return -4;
    }
  }

  int result = vfprintf(stream->file, format, arg);
  if (result < 0) {
    // TODO
    return -5;
  }
  va_end(arg);

  if (stream->file == stdout || stream->file == stderr ||
      stream->file == stdin) {
    if (AqvmBaseThreadingMutex_UnlockMutex(&AqvmBaseIo_printMutex)) {
      // TODO
      return -6;
    }
  } else {
    if (AqvmBaseProcessFileLock_UnlockFile(stream)) {
      // TODO
      return -7;
    }
    if (AqvmBaseFile_UnlockFile(stream)) {
      // TODO
      return -8;
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

int AqvmBaseIo_vsprintf(char* str, const char* format, va_list arg);
