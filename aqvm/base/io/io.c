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
struct AqvmBaseFile_File* AqvmBaseIo_stdout = &AqvmBaseIo_stdoutStream;

struct AqvmBaseFile_File AqvmBaseIo_stdinStream;
struct AqvmBaseFile_File* AqvmBaseIo_stdin = &AqvmBaseIo_stdinStream;

struct AqvmBaseFile_File AqvmBaseIo_stderrStream;
struct AqvmBaseFile_File* AqvmBaseIo_stderr = &AqvmBaseIo_stderrStream;

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

int AqvmBaseIo_LockStream(struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
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
}

int AqvmBaseIo_UnlockStream(struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
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
}

int AqvmBaseIo_OutputToStream(struct AqvmBaseFile_File* stream, ...) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, stream);
  char* str = va_arg(arg, char*);
  while (str != NULL) {
    int result = vfprintf(stream->file, "%s", arg);
    if (result < 0) {
      // TODO
      return -3;
    }
    str = va_arg(arg, char*);
  }
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -4;
  }

  return 0;
}

int AqvmBaseIo_fgetc(struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
    // TODO
    return -2;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return -3;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  int result = fgetc(stream->file);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -5;
  }

  if (result == EOF) {
    if (AqvmBaseFile_feof(stream)) {
      return EOF;
    }
    // TODO
    return -6;
  }

  return result;
}

char* AqvmBaseIo_fgets(char* str, int n, struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0 || str == NULL || n <= 0) {
    // TODO
    return NULL;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return NULL;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return NULL;
  }

  char* result = fgets(str, n, stream->file);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return NULL;
  }

  if (result == NULL) {
    if (AqvmBaseFile_feof(stream)) {
      return NULL;
    }
    // TODO
    return NULL;
  }

  return result;
}

int AqvmBaseIo_fprintf(struct AqvmBaseFile_File* stream, const char* format,
                       ...) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0 || format == NULL) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  va_list arg;
  va_start(arg, format);
  int result = vfprintf(stream->file, format, arg);
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -4;
  }

  if (result < 0) {
    // TODO
    return -5;
  }

  return 0;
}

int AqvmBaseIo_fputc(int character, struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
    // TODO
    return -2;
  }
  if (stream->file == stdin) {
    // TODO
    return -3;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  int result = fputc(character, stream->file);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -5;
  }

  if (result == EOF) {
    // TODO
    return -6;
  }

  return 0;
}

int AqvmBaseIo_fputs(const char* str, struct AqvmBaseFile_File* stream) {
  if (str == NULL || stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  int result = fputs(str, stream->file);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -4;
  }

  if (result == EOF) {
    // TODO
    return -5;
  }

  return 0;
}

int AqvmBaseIo_fscanf(struct AqvmBaseFile_File* stream, const char* format,
                      ...) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0 || format == NULL) {
    // TODO
    return -2;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return -3;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  va_list arg;
  va_start(arg, format);
  int result = vfscanf(stream->file, format, arg);
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -5;
  }

  if (result == EOF) {
    // TODO
    return -6;
  }

  return result;
}

int AqvmBaseIo_getc(struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
    return -2;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return -3;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  int result = getc(stream->file);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -5;
  }

  if (result == EOF) {
    if (AqvmBaseFile_feof(stream)) {
      return EOF;
    } else {
      // TODO
      return -6;
    }
  }

  return result;
}

int AqvmBaseIo_getchar(void) {
  if (AqvmBaseFile_ferror(AqvmBaseIo_stdin) != 0) {
    // TODO
    return -2;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdin) != 0) {
    // TODO
    return -3;
  }

  int result = getchar();

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdin) != 0) {
    // TODO
    return -4;
  }

  if (result == EOF) {
    if (AqvmBaseFile_feof(AqvmBaseIo_stdin)) {
      return EOF;
    } else {
      // TODO
      return -5;
    }
  }

  return result;
}

int AqvmBaseIo_perror(const char* str) {
  if (str == NULL||AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stderr) != 0) {
    // TODO
    return -2;
  }

  perror(str);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stderr) != 0) {
    // TODO
    return -3;
  }

  return 0;
}

int AqvmBaseIo_printf(const char* format, ...) {
  if (format == NULL||AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vprintf(format, arg);
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  if (result < 0) {
    // TODO
    return -4;
  }

  return 0;
}

int AqvmBaseIo_putc(int character, struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  int result = putc(character, stream->file);

  if (AqvmBaseIo_UnlockStream(stream) != 0) {
    // TODO
    return -4;
  }

  if (result == EOF) {
    // TODO
    return -5;
  }

  return result;
}

int AqvmBaseIo_putchar(int character) {
  if (AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  int result = putchar(character);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -4;
  }

  if (result == EOF) {
    // TODO
    return -5;
  }

  return result;
}

int AqvmBaseIo_puts(const char* str) {
  if (str == NULL||AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  int result = puts(str);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  if (result == EOF) {
    // TODO
    return -4;
  }

  return result;
}

int AqvmBaseIo_scanf(const char* format, ...) {
  if (format == NULL || AqvmBaseFile_ferror(AqvmBaseIo_stdin) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vfscanf(stdin, format, arg);
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  if (result == EOF) {
    // TODO
    return -4;
  }

  return result;
}

int AqvmBaseIo_snprintf(char* str, size_t size, const char* format, ...) {
  if (str == NULL || format == NULL ||
      AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vsnprintf(str, size, format, arg);
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  if (result < 0) {
    // TODO
    return -4;
  }

  return result;
}

int AqvmBaseIo_sprintf(char* str, const char* format, ...) {
  if (str == NULL || format == NULL ||
      AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vsprintf(str, format, arg);
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  if (result < 0) {
    // TODO
    return -4;
  }

  return result;
}

int AqvmBaseIo_sscanf(const char* str, const char* format, ...) {
  if (str == NULL || format == NULL ||
      AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vsscanf(str, format, arg);
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  if (result == EOF) {
    // TODO
    return -4;
  }

  return result;
}

int AqvmBaseIo_ungetc(int character, struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  int result = ungetc(character, stream->file);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  if (result == EOF) {
    // TODO
    return -4;
  }

  return result;
}

int AqvmBaseIo_vfprintf(struct AqvmBaseFile_File* stream, const char* format,
                        va_list arg) {
  if (stream == NULL || stream->file == NULL ||
      AqvmBaseFile_ferror(stream) != 0 || format == NULL) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseIo_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  int result = vfprintf(stream->file, format, arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -4;
  }

  if (result < 0) {
    // TODO
    return -5;
  }

  return 0;
}

int AqvmBaseIo_vprintf(const char* format, va_list arg) {
  if (format == NULL || AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  int result = vprintf(format, arg);
  if (result < 0) {
    // TODO
    return -3;
  }
  va_end(arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -4;
  }

  return 0;
}

int AqvmBaseIo_vsprintf(char* str, const char* format, va_list arg) {
  if (str == NULL || format == NULL ||
      AqvmBaseFile_ferror(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseIo_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  int result = vsprintf(str, format, arg);

  if (AqvmBaseIo_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -4;
  }

  if (result < 0) {
    // TODO
    return -5;
  }

  return 0;
}
