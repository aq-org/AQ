#ifndef AQ_AQVM_BASE_IO_IO_H_
#define AQ_AQVM_BASE_IO_IO_H_

#include <stdarg.h>
#include <stdio.h>

#include "aqvm/base/threading/mutex/mutex.h"

int AqvmBaseIo_InitializeIo();

int AqvmBaseIo_CloseIo();

int AqvmBaseIo_fprintf(FILE* stream, const char* format, ...);

int AqvmBaseIo_fscanf(FILE* stream, const char* format, ...);

int AqvmBaseIo_perror(const char* str);

int AqvmBaseIo_printf(const char* format, ...);

int AqvmBaseIo_scanf(const char* format, ...);

int AqvmBaseIo_snprintf(char* str, size_t size, const char* format, ...);

int AqvmBaseIo_sprintf(char* str, const char* format, ...);

int AqvmBaseIo_sscanf(const char* str, const char* format, ...);

int AqvmBaseIo_vfprintf(FILE* stream, const char* format, va_list arg);

int AqvmBaseIo_vprintf(const char* format, va_list arg);

int AqvmBaseIo_vsprintf(char* str, const char* format, va_list arg);

int AqvmBaseIo_fgetc(FILE* stream);

char* AqvmBaseIo_fgets(char* str, int n, FILE* stream);

int AqvmBaseIo_fputc(int character, FILE* stream);

int AqvmBaseIo_fputs(const char* str, FILE* stream);

int AqvmBaseIo_getc(FILE* stream);

int AqvmBaseIo_getchar(void);

char* AqvmBaseIo_gets(char* str);

int AqvmBaseIo_putc(int character, FILE* stream);

int AqvmBaseIo_putchar(int character);

int AqvmBaseIo_puts(const char* str);

int AqvmBaseIo_ungetc(int character, FILE* stream);

#endif