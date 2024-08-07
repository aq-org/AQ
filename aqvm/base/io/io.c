// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/io/io.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "aqvm/base/file/file.h"

AqvmBaseThreadingMutex_Mutex AqvmBaseIo_inputMutex;
AqvmBaseThreadingMutex_Mutex AqvmBaseIo_outputMutex;

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

  if (AqvmBaseThreadingMutex_InitializeMutex(&AqvmBaseIo_inputMutex) != 0) {
    // TODO
    return -1;
  }
  if (AqvmBaseThreadingMutex_InitializeMutex(&AqvmBaseIo_outputMutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseIo_CloseIo() {
  if (AqvmBaseThreadingMutex_CloseMutex(&AqvmBaseIo_inputMutex) != 0) {
    // TODO
    return -1;
  }
  if (AqvmBaseThreadingMutex_CloseMutex(&AqvmBaseIo_outputMutex) != 0) {
    // TODO
    return -1;
  }
  return 0;
}

int AqvmBaseIo_OutputLog(struct AqvmBaseFile_File* stream, const char* time,
                         const char* type, const char* code,
                         const char* message, va_list system_info,
                         va_list other_info) {
  if (AqvmBaseFile_CheckStream(stream) != 0 || time == NULL || type == NULL ||
      code == NULL || message == NULL) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -2;
  }

  if (fprintf(
          stream->file,
          "{\"Time\":\"%s\",\"Type\":\"%s\",\"Code\":\"%s\",\"Message\":\"%s\"",
          time, type, code, message) < 0) {
    // TODO
    return -3;
  }

  char* name = va_arg(system_info, char*);
  char* value = NULL;
  if (name != NULL) {
    value = va_arg(system_info, char*);
  }
  while (name != NULL && value != NULL) {
    if (fprintf(stream->file, ",\"%s\":\"%s\"", name, value) != 0) {
      // TODO
      return -4;
    }
    name = va_arg(system_info, char*);
    if (name != NULL) {
      value = va_arg(system_info, char*);
    }
  }

  name = va_arg(other_info, char*);
  value = NULL;
  if (name != NULL) {
    value = va_arg(system_info, char*);
  }
  while (name != NULL && value != NULL) {
    if (fprintf(stream->file, ",\"%s\":\"%s\"", name, value) != 0) {
      // TODO
      return -5;
    }
    name = va_arg(other_info, char*);
    if (name != NULL) {
      value = va_arg(system_info, char*);
    }
  }

  if (fprintf(stream->file, "}\n") < 0) {
    // TODO
    return -6;
  }

  if (AqvmBaseFile_UnlockStream(stream) < 0) {
    // TODO
    return -7;
  }

  return 0;
}

int AqvmBaseIo_fgetc(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -2;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return -3;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  int result = fgetc(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (str == NULL || n <= 0 || AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return NULL;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return NULL;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return NULL;
  }

  char* result = fgets(str, n, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (format == NULL || AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  va_list arg;
  va_start(arg, format);
  int result = vfprintf(stream->file, format, arg);
  va_end(arg);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -2;
  }
  if (stream->file == stdin) {
    // TODO
    return -3;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  int result = fputc(character, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (str == NULL || AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  int result = fputs(str, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (format == NULL || AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -2;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return -3;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  va_list arg;
  va_start(arg, format);
  int result = vfscanf(stream->file, format, arg);
  va_end(arg);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    return -2;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return -3;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -4;
  }

  int result = getc(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (AqvmBaseFile_CheckStream(AqvmBaseIo_stdin) != 0) {
    // TODO
    return -2;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdin) != 0) {
    // TODO
    return -3;
  }

  int result = getchar();

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdin) != 0) {
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
  if (str == NULL || AqvmBaseFile_CheckStream(AqvmBaseIo_stderr) != 0) {
    return -1;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stderr) != 0) {
    // TODO
    return -2;
  }

  perror(str);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stderr) != 0) {
    // TODO
    return -3;
  }

  return 0;
}

int AqvmBaseIo_printf(const char* format, ...) {
  if (format == NULL || AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    return -1;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vprintf(format, arg);
  va_end(arg);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
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
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  int result = putc(character, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
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
  if (AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  int result = putchar(character);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
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
  if (str == NULL || AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  int result = puts(str);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
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
  if (format == NULL || AqvmBaseFile_CheckStream(AqvmBaseIo_stdin) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  va_list arg;
  va_start(arg, format);
  int result = vfscanf(stdin, format, arg);
  va_end(arg);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
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
      AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  va_list arg;
  va_start(arg, format);
  int result = vsnprintf(str, size, format, arg);
  va_end(arg);

  if (result < 0) {
    // TODO
    return -2;
  }

  return result;
}

int AqvmBaseIo_sprintf(char* str, const char* format, ...) {
  if (str == NULL || format == NULL ||
      AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  va_list arg;
  va_start(arg, format);
  int result = vsprintf(str, format, arg);
  va_end(arg);

  if (result < 0) {
    // TODO
    return -2;
  }

  return result;
}

int AqvmBaseIo_sscanf(const char* str, const char* format, ...) {
  if (str == NULL || format == NULL ||
      AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  va_list arg;
  va_start(arg, format);
  int result = vsscanf(str, format, arg);
  va_end(arg);

  if (result == EOF) {
    // TODO
    return -2;
  }

  return result;
}

int AqvmBaseIo_ungetc(int character, struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -1;
  }
  if (stream->file == stdout || stream->file == stderr) {
    // TODO
    return -2;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -3;
  }

  int result = ungetc(character, stream->file);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -4;
  }

  if (result == EOF) {
    // TODO
    return -5;
  }

  return result;
}

int AqvmBaseIo_vfprintf(struct AqvmBaseFile_File* stream, const char* format,
                        va_list arg) {
  if (format == NULL || AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO
    return -1;
  }
  if (stream->file == stdin) {
    // TODO
    return -2;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -3;
  }

  int result = vfprintf(stream->file, format, arg);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
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
  if (format == NULL || AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    return -1;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  int result = vprintf(format, arg);
  if (result < 0) {
    // TODO
    return -3;
  }
  va_end(arg);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -4;
  }

  return 0;
}

int AqvmBaseIo_vsprintf(char* str, const char* format, va_list arg) {
  if (str == NULL || format == NULL ||
      AqvmBaseFile_CheckStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -2;
  }

  int result = vsprintf(str, format, arg);

  if (AqvmBaseFile_UnlockStream(AqvmBaseIo_stdout) != 0) {
    // TODO
    return -4;
  }

  if (result < 0) {
    // TODO
    return -5;
  }

  return 0;
}
