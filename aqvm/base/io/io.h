// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_IO_IO_H_
#define AQ_AQVM_BASE_IO_IO_H_

#include <stdarg.h>
#include <stdio.h>

#include "aqvm/base/file/file.h"
#include "aqvm/base/threading/mutex/mutex.h"

extern AqvmBaseThreadingMutex_Mutex AqvmBaseIo_inputMutex;
extern AqvmBaseThreadingMutex_Mutex AqvmBaseIo_outputMutex;

extern struct AqvmBaseFile_File* AqvmBaseIo_stdout;
extern struct AqvmBaseFile_File* AqvmBaseIo_stdin;
extern struct AqvmBaseFile_File* AqvmBaseIo_stderr;

int AqvmBaseIo_InitializeIo();

int AqvmBaseIo_CloseIo();

int AqvmBaseIo_OutputLog(struct AqvmBaseFile_File* stream, const char* time,
                         const char* type, const char* code,
                         const char* message, va_list system_info,
                         va_list other_info);

int AqvmBaseIo_fgetc(struct AqvmBaseFile_File* stream);

char* AqvmBaseIo_fgets(char* str, int n, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fprintf(struct AqvmBaseFile_File* stream, const char* format,
                       ...);

int AqvmBaseIo_fputc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fputs(const char* str, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fscanf(struct AqvmBaseFile_File* stream, const char* format,
                      ...);

int AqvmBaseIo_getc(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_getchar(void);

int AqvmBaseIo_perror(const char* str);

int AqvmBaseIo_printf(const char* format, ...);

int AqvmBaseIo_putc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_putchar(int character);

int AqvmBaseIo_puts(const char* str);

int AqvmBaseIo_scanf(const char* format, ...);

int AqvmBaseIo_snprintf(char* str, size_t size, const char* format, ...);

int AqvmBaseIo_sprintf(char* str, const char* format, ...);

int AqvmBaseIo_sscanf(const char* str, const char* format, ...);

int AqvmBaseIo_ungetc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_vfprintf(struct AqvmBaseFile_File* stream, const char* format,
                        va_list arg);

int AqvmBaseIo_vprintf(const char* format, va_list arg);

int AqvmBaseIo_vsprintf(char* str, const char* format, va_list arg);

#endif