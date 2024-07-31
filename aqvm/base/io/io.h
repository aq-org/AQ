#ifndef AQ_AQVM_BASE_IO_IO_H_
#define AQ_AQVM_BASE_IO_IO_H_

#include <stdarg.h>
#include <stdio.h>

#include "aqvm/base/file/file.h"
#include "aqvm/base/threading/mutex/mutex.h"

extern struct AqvmBaseFile_File* AqvmBaseIo_stdout;
extern struct AqvmBaseFile_File* AqvmBaseIo_stdin;
extern struct AqvmBaseFile_File* AqvmBaseIo_stderr;

int AqvmBaseIo_InitializeIo();

int AqvmBaseIo_CloseIo();

int AqvmBaseIo_fprintf(struct AqvmBaseFile_File* stream, const char* format, ...);

int AqvmBaseIo_fscanf(struct AqvmBaseFile_File* stream, const char* format, ...);

int AqvmBaseIo_perror(const char* str);

int AqvmBaseIo_printf(const char* format, ...);

int AqvmBaseIo_scanf(const char* format, ...);

int AqvmBaseIo_snprintf(char* str, size_t size, const char* format, ...);

int AqvmBaseIo_sprintf(char* str, const char* format, ...);

int AqvmBaseIo_sscanf(const char* str, const char* format, ...);

int AqvmBaseIo_vfprintf(struct AqvmBaseFile_File* stream, const char* format, va_list arg);

int AqvmBaseIo_vprintf(const char* format, va_list arg);

int AqvmBaseIo_vsprintf(char* str, const char* format, va_list arg);

int AqvmBaseIo_fgetc(struct AqvmBaseFile_File* stream);

char* AqvmBaseIo_fgets(char* str, int n, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fputc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fputs(const char* str, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_getc(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_getchar(void);

char* AqvmBaseIo_gets(char* str);

int AqvmBaseIo_putc(int character, struct AqvmBaseFile_File* stream);

int AqvmBaseIo_putchar(int character);

int AqvmBaseIo_puts(const char* str);

int AqvmBaseIo_ungetc(int character, struct AqvmBaseFile_File* stream);

#endif