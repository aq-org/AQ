// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/file.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "aqvm/base/io/io.h"
#include "aqvm/base/process/file_lock/file_lock.h"
#include "aqvm/base/threading/mutex/mutex.h"

int AqvmBaseFile_LockFile(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1;
  }

  int result = AqvmBaseThreadingMutex_LockMutex(&stream->mutex);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseFile_UnlockFile(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1;
  }

  int result = AqvmBaseThreadingMutex_UnlockMutex(&stream->mutex);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseFile_LockStream(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
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
    if (AqvmBaseFile_LockStream(stream) != 0) {
      // TODO
      return -4;
    }
  }

  return 0;
}

int AqvmBaseFile_UnlockStream(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
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
    if (AqvmBaseFile_LockStream(stream) != 0) {
      // TODO
      return -4;
    }
  }

  return 0;
}

int AqvmBaseFile_CheckStream(struct AqvmBaseFile_File* stream) {
  if (stream == NULL) {
    // TODO
    return -1;
  }
  if (stream->file == NULL) {
    // TODO
    return -2;
  }
  if (AqvmBaseFile_ferror(stream) != 0) {
    // TODO
    return -3;
  }

  return 0;
}

void AqvmBaseFile_clearerr(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return;
  }

  clearerr(stream->file);
}

int AqvmBaseFile_fclose(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1;
  }

  int result = fclose(stream->file);
  if (AqvmBaseThreadingMutex_CloseMutex(&stream->mutex)) {
    // TODO
    return -2;
  }

  if (result != 0) {
    // TODO
    return -3;
  }

  free(stream);

  return 0;
}

int AqvmBaseFile_feof(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1;
  }

  int result = feof(stream->file);
  return result;
}

int AqvmBaseFile_ferror(struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL) {
    // TODO
    return -1;
  }

  int result = ferror(stream->file);
  return result;
}

int AqvmBaseFile_fflush(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -2;
  }

  int result = fflush(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return -3;
  }

  if (result < 0) {
    // TODO
    return -4;
  }

  return result;
}

int AqvmBaseFile_fgetpos(struct AqvmBaseFile_File* stream, fpos_t* pos) {
  if (AqvmBaseFile_CheckStream(stream) || pos == NULL) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -2;
  }

  int result = fgetpos(stream->file, pos);
  if (result != 0) {
    // TODO
    return -3;
  }

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return -4;
  }

  return 0;
}

struct AqvmBaseFile_File* AqvmBaseFile_fopen(const char* filename,
                                             const char* mode) {
  if (filename == NULL || mode == NULL) {
    // TODO
    return NULL;
  }

  struct AqvmBaseFile_File* stream =
      (struct AqvmBaseFile_File*)malloc(sizeof(struct AqvmBaseFile_File));
  if (stream == NULL) {
    // TODO
    return NULL;
  }

  stream->file = fopen(filename, mode);
  if (stream->file == NULL || AqvmBaseFile_ferror(stream) != 0) {
    free(stream);
    // TODO
    return NULL;
  }

  if (AqvmBaseThreadingMutex_InitializeMutex(&stream->mutex) != 0) {
    fclose(stream->file);
    free(stream);
    return NULL;
  }

  return stream;
}

size_t AqvmBaseFile_fread(void* ptr, size_t size, size_t nmemb,
                          struct AqvmBaseFile_File* stream) {
  if (ptr == NULL || AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return 0;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return 0;
  }

  size_t result = fread(ptr, size, nmemb, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return 0;
  }

  return result;
}

struct AqvmBaseFile_File* AqvmBaseFile_freopen(
    const char* filename, const char* mode, struct AqvmBaseFile_File* stream) {
  if (filename == NULL || mode == NULL || AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return NULL;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return NULL;
  }

  FILE* new_file = freopen(filename, mode, stream->file);
  if (new_file == NULL) {
    if (AqvmBaseFile_UnlockStream(stream) != 0) {
      // TODO
    }
    // TODO
    return NULL;
  }

  stream->file = new_file;

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return NULL;
  }

  return stream;
}

int AqvmBaseFile_fseek(struct AqvmBaseFile_File* stream, long int offset,
                       int whence) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -2;
  }

  int result = fseek(stream->file, offset, whence);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return -3;
  }

  if (result != 0) {
    // TODO
    return -4;
  }

  return 0;
}

int AqvmBaseFile_fsetpos(struct AqvmBaseFile_File* stream, const fpos_t* pos) {
  if (AqvmBaseFile_CheckStream(stream) || pos == NULL) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -2;
  }

  int result = fsetpos(stream->file, pos);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return -3;
  }

  if (result != 0) {
    // TODO
    return -4;
  }

  return 0;
}

long int AqvmBaseFile_ftell(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1L;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -1L;
  }

  long int result = ftell(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return -1L;
  }

  if (result == -1L) {
    // TODO
    return -1L;
  }

  return result;
}

size_t AqvmBaseFile_fwrite(const void* ptr, size_t size, size_t nmemb,
                           struct AqvmBaseFile_File* stream) {
  if (ptr == NULL || AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return 0;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return 0;
  }

  size_t result = fwrite(ptr, size, nmemb, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return 0;
  }

  return result;
}

int AqvmBaseFile_remove(const char* filename) {
  if (filename == NULL) {
    // TODO
    return -1;
  }

  int result = remove(filename);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseFile_rename(const char* old_filename, const char* new_filename) {
  if (old_filename == NULL || new_filename == NULL) {
    // TODO
    return -1;
  }

  int result = rename(old_filename, new_filename);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

void AqvmBaseFile_rewind(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return;
  }
  rewind(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return;
  }
}

void AqvmBaseFile_setbuf(struct AqvmBaseFile_File* stream, char* buffer) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return;
  }
  setbuf(stream->file, buffer);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return;
  }
}

int AqvmBaseFile_setvbuf(struct AqvmBaseFile_File* stream, char* buffer,
                         int mode, size_t size) {
  if (AqvmBaseFile_CheckStream(stream)) {
    // TODO
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO
    return -2;
  }

  int result = setvbuf(stream->file, buffer, mode, size);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO
    return -3;
  }

  if (result != 0) {
    // TODO
    return -4;
  }

  return result;
}

struct AqvmBaseFile_File* AqvmBaseFile_tmpfile(void) {
  struct AqvmBaseFile_File* stream =
      (struct AqvmBaseFile_File*)malloc(sizeof(struct AqvmBaseFile_File));
  if (stream == NULL) {
    // TODO
    return NULL;
  }

  stream->file = tmpfile();
  if (stream->file == NULL || AqvmBaseFile_ferror(stream) != 0) {
    free(stream);
    // TODO
    return NULL;
  }

  AqvmBaseThreadingMutex_InitializeMutex(&stream->mutex);

  return stream;
}

char* AqvmBaseFile_tmpnam(char* str) {  // TODO
  char* result = tmpnam(str);
  if (result == NULL) {
    // TODO
    return NULL;
  }
  return result;
}