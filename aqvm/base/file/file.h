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
  AqvmBaseThreadingMutex_Mutex mutex;
};

int AqvmBaseFile_LockFile(struct AqvmBaseFile_File* file);

int AqvmBaseFile_UnlockFile(struct AqvmBaseFile_File* file);

void AqvmBaseFile_clearerr(struct AqvmBaseFile_File* stream);

int AqvmBaseFile_fclose(struct AqvmBaseFile_File* stream);

int AqvmBaseFile_feof(struct AqvmBaseFile_File* stream);

int AqvmBaseFile_ferror(struct AqvmBaseFile_File* stream);

int AqvmBaseFile_fflush(struct AqvmBaseFile_File* stream);

int AqvmBaseFile_fgetpos(struct AqvmBaseFile_File* stream, fpos_t* pos);

struct AqvmBaseFile_File* AqvmBaseFile_fopen(const char* filename,
                                           const char* mode);

size_t AqvmBaseFile_fread(void* ptr, size_t size, size_t nmemb,
                        struct AqvmBaseFile_File* stream);

struct AqvmBaseFile_File* AqvmBaseFile_freopen(const char* filename,
                                             const char* mode,
                                             struct AqvmBaseFile_File* stream);

int AqvmBaseFile_fseek(struct AqvmBaseFile_File* stream, long int offset,
                     int whence);

int AqvmBaseFile_fsetpos(struct AqvmBaseFile_File* stream, const fpos_t* pos);

long int AqvmBaseFile_ftell(struct AqvmBaseFile_File* stream);

size_t AqvmBaseFile_fwrite(const void* ptr, size_t size, size_t nmemb,
                         struct AqvmBaseFile_File* stream);

int AqvmBaseFile_remove(const char* filename);

int AqvmBaseFile_rename(const char* old_filename, const char* new_filename);

void AqvmBaseFile_rewind(struct AqvmBaseFile_File* stream);

void AqvmBaseFile_setbuf(struct AqvmBaseFile_File* stream, char* buffer);

int AqvmBaseFile_setvbuf(struct AqvmBaseFile_File* stream, char* buffer, int mode,
                       size_t size);

struct AqvmBaseFile_File* AqvmBaseFile_tmpfile(void);

char* AqvmBaseFile_tmpnam(char* str);

#endif