// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_FILE_FILE_H_
#define AQ_AQVM_BASE_FILE_FILE_H_

#include <stdbool.h>
#include <stdio.h>

#include "aqvm/base/threading/mutex/mutex.h"

struct AqvmBaseFile_File {
  FILE* file;
  AqvmBaseThreadingMutex_Mutex* mutex;
};

int AqvmBaseFile_LockFile(struct AqvmBaseFile_File* file);

int AqvmBaseFile_UnlockFile(struct AqvmBaseFile_File* file);

void AqvmBaseIo_clearerr(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fclose(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_feof(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_ferror(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fflush(struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fgetpos(struct AqvmBaseFile_File* stream, fpos_t* pos);

struct AqvmBaseFile_File* AqvmBaseIo_fopen(const char* filename,
                                           const char* mode);

size_t AqvmBaseIo_fread(void* ptr, size_t size, size_t nmemb,
                        struct AqvmBaseFile_File* stream);

struct AqvmBaseFile_File* AqvmBaseIo_freopen(const char* filename,
                                             const char* mode,
                                             struct AqvmBaseFile_File* stream);

int AqvmBaseIo_fseek(struct AqvmBaseFile_File* stream, long int offset,
                     int whence);

int AqvmBaseIo_fsetpos(struct AqvmBaseFile_File* stream, const fpos_t* pos);

long int AqvmBaseIo_ftell(struct AqvmBaseFile_File* stream);

size_t AqvmBaseIo_fwrite(const void* ptr, size_t size, size_t nmemb,
                         struct AqvmBaseFile_File* stream);

int AqvmBaseIo_remove(const char* filename);

int AqvmBaseIo_rename(const char* old_filename, const char* new_filename);

void AqvmBaseIo_rewind(struct AqvmBaseFile_File* stream);

void AqvmBaseIo_setbuf(struct AqvmBaseFile_File* stream, char* buffer);

int AqvmBaseIo_setvbuf(struct AqvmBaseFile_File* stream, char* buffer, int mode,
                       size_t size);

struct AqvmBaseFile_File* AqvmBaseIo_tmpfile(void);

char* AqvmBaseIo_tmpnam(char* str);

#endif