// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/file.h"

#include <stdbool.h>
#include <stdio.h>

#include "aqvm/base/io/io.h"
#include "aqvm/base/memory/memory.h"
#include "aqvm/base/process/file_lock/file_lock.h"
#include "aqvm/base/threading/file_lock/file_lock.h"
#include "aqvm/base/threading/mutex/mutex.h"

int AqvmBaseFile_LockFile(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }

  /*int result = AqvmBaseThreadingMutex_LockMutex(&stream->mutex);
  if (result != 0) {
    // TODO(logging)
    return -2;
  }*/
  return 0;
}

int AqvmBaseFile_UnlockFile(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }

  /*int result = AqvmBaseThreadingMutex_UnlockMutex(&stream->mutex);
  if (result != 0) {
    // TODO(logging)
    return -2;
  }*/
  return 0;
}

int AqvmBaseFile_LockStream(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }
  if (stream->file == stdin) {
    if (AqvmBaseThreadingMutex_LockMutex(AqvmBaseIo_inputMutex) != 0) {
      // TODO(logging)
      return -2;
    }
  } else if (stream->file == stdout || stream->file == stderr) {
    if (AqvmBaseThreadingMutex_LockMutex(AqvmBaseIo_outputMutex) != 0) {
      // TODO(logging)
      return -3;
    }
  } else {
    if (AqvmBaseProcessFileLock_LockFile(stream) != 0) {
      // TODO(logging)
      return -4;
    }
  }

  return 0;
}

int AqvmBaseFile_UnlockStream(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }
  if (stream->file == stdin) {
    if (AqvmBaseThreadingMutex_UnlockMutex(AqvmBaseIo_inputMutex) != 0) {
      // TODO(logging)
      return -2;
    }
  } else if (stream->file == stdout || stream->file == stderr) {
    if (AqvmBaseThreadingMutex_UnlockMutex(AqvmBaseIo_outputMutex) != 0) {
      // TODO(logging)
      return -3;
    }
  } else {
    if (AqvmBaseProcessFileLock_UnlockFile(stream) != 0) {
      // TODO(logging)
      return -4;
    }
  }

  return 0;
}

int AqvmBaseFile_CheckStream(struct AqvmBaseFile_File* stream) {
  if (stream == NULL) {
    // TODO(logging)
    return -1;
  }
  if (stream->file == NULL) {
    // TODO(logging)
    return -2;
  }
  if (AqvmBaseFile_ferror(stream) != 0) {
    // TODO(logging)
    return -3;
  }

  return 0;
}

void AqvmBaseFile_clearerr(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return;
  }

  clearerr(stream->file);
}

int AqvmBaseFile_fclose(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }

  int result = fclose(stream->file);
  AqvmBaseMemory_free(stream->identifier);
  if (AqvmBaseThreadingFileLock_RemoveFileLock(stream) != 0) {
    // TODO(logging)
    AqvmBaseMemory_free(stream);
    return -2;
  }
  AqvmBaseMemory_free(stream);

  if (result != 0) {
    // TODO(logging)
    return -3;
  }

  return 0;
}

int AqvmBaseFile_feof(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }

  int result = feof(stream->file);
  return result;
}

int AqvmBaseFile_ferror(struct AqvmBaseFile_File* stream) {
  if (stream == NULL || stream->file == NULL) {
    // TODO(logging)
    return -1;
  }

  int result = ferror(stream->file);
  return result;
}

int AqvmBaseFile_fflush(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return -2;
  }

  int result = fflush(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return -3;
  }

  if (result < 0) {
    // TODO(logging)
    return -4;
  }

  return result;
}

int AqvmBaseFile_fgetpos(struct AqvmBaseFile_File* stream, fpos_t* pos) {
  if (AqvmBaseFile_CheckStream(stream) != 0 || pos == NULL) {
    // TODO(logging)
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return -2;
  }

  int result = fgetpos(stream->file, pos);
  if (result != 0) {
    // TODO(logging)
    return -3;
  }

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return -4;
  }

  return 0;
}

struct AqvmBaseFile_File* AqvmBaseFile_fopen(const char* filename,
                                             const char* mode) {
  if (filename == NULL || mode == NULL) {
    // TODO(logging)
    return NULL;
  }

  struct AqvmBaseFile_File* stream =
      (struct AqvmBaseFile_File*)AqvmBaseMemory_malloc(sizeof(struct AqvmBaseFile_File));
  if (stream == NULL) {
    // TODO(logging)
    return NULL;
  }
  stream->identifier = (AqvmBaseFileIdentifier_Identifier*)AqvmBaseMemory_malloc(
      sizeof(AqvmBaseFileIdentifier_Identifier));
  if (stream->identifier == NULL) {
    // TODO(logging)
    AqvmBaseMemory_free(stream);
    return NULL;
  }

  stream->file = fopen(filename, mode);
  if (stream->file == NULL || AqvmBaseFile_ferror(stream) != 0) {
    AqvmBaseMemory_free(stream->identifier);
    AqvmBaseMemory_free(stream);
    // TODO(logging)
    return NULL;
  }

  if (AqvmBaseFileIdentifier_GetIdentifier(filename, stream->identifier) !=
      0) {
    // TODO(logging)
    fclose(stream->file);
    AqvmBaseMemory_free(stream->identifier);
    AqvmBaseMemory_free(stream);
    return NULL;
  }

  if (AqvmBaseThreadingFileLock_AddFileLock(stream) != 0) {
    fclose(stream->file);
    AqvmBaseMemory_free(stream->identifier);
    AqvmBaseMemory_free(stream);
    return NULL;
  }

  return stream;
}

size_t AqvmBaseFile_fread(void* ptr, size_t size, size_t nmemb,
                          struct AqvmBaseFile_File* stream) {
  if (ptr == NULL || AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return 0;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return 0;
  }

  size_t result = fread(ptr, size, nmemb, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return 0;
  }

  return result;
}

struct AqvmBaseFile_File* AqvmBaseFile_freopen(
    const char* filename, const char* mode, struct AqvmBaseFile_File* stream) {
  if (filename == NULL || mode == NULL ||
      AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return NULL;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return NULL;
  }

  FILE* new_file = freopen(filename, mode, stream->file);
  if (new_file == NULL) {
    if (AqvmBaseFile_UnlockStream(stream) != 0) {
      // TODO(logging)
    }
    // TODO(logging)
    return NULL;
  }

  stream->file = new_file;

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return NULL;
  }

  return stream;
}

int AqvmBaseFile_fseek(struct AqvmBaseFile_File* stream, long int offset,
                       int whence) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return -2;
  }

  int result = fseek(stream->file, offset, whence);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return -3;
  }

  if (result != 0) {
    // TODO(logging)
    return -4;
  }

  return 0;
}

int AqvmBaseFile_fsetpos(struct AqvmBaseFile_File* stream, const fpos_t* pos) {
  if (AqvmBaseFile_CheckStream(stream) != 0 || pos == NULL) {
    // TODO(logging)
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return -2;
  }

  int result = fsetpos(stream->file, pos);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return -3;
  }

  if (result != 0) {
    // TODO(logging)
    return -4;
  }

  return 0;
}

long int AqvmBaseFile_ftell(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1L;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return -1L;
  }

  long int result = ftell(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return -1L;
  }

  if (result == -1L) {
    // TODO(logging)
    return -1L;
  }

  return result;
}

size_t AqvmBaseFile_fwrite(const void* ptr, size_t size, size_t nmemb,
                           struct AqvmBaseFile_File* stream) {
  if (ptr == NULL || AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return 0;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return 0;
  }

  size_t result = fwrite(ptr, size, nmemb, stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return 0;
  }

  return result;
}

int AqvmBaseFile_remove(const char* filename) {
  if (filename == NULL) {
    // TODO(logging)
    return -1;
  }

  int result = remove(filename);
  if (result != 0) {
    // TODO(logging)
    return -2;
  }
  return 0;
}

int AqvmBaseFile_rename(const char* old_filename, const char* new_filename) {
  if (old_filename == NULL || new_filename == NULL) {
    // TODO(logging)
    return -1;
  }

  int result = rename(old_filename, new_filename);
  if (result != 0) {
    // TODO(logging)
    return -2;
  }
  return 0;
}

void AqvmBaseFile_rewind(struct AqvmBaseFile_File* stream) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return;
  }
  rewind(stream->file);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return;
  }
}

void AqvmBaseFile_setbuf(struct AqvmBaseFile_File* stream, char* buffer) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return;
  }
  setbuf(stream->file, buffer);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return;
  }
}

int AqvmBaseFile_setvbuf(struct AqvmBaseFile_File* stream, char* buffer,
                         int mode, size_t size) {
  if (AqvmBaseFile_CheckStream(stream) != 0) {
    // TODO(logging)
    return -1;
  }

  if (AqvmBaseFile_LockStream(stream) != 0) {
    // TODO(logging)
    return -2;
  }

  int result = setvbuf(stream->file, buffer, mode, size);

  if (AqvmBaseFile_UnlockStream(stream) != 0) {
    // TODO(logging)
    return -3;
  }

  if (result != 0) {
    // TODO(logging)
    return -4;
  }

  return result;
}

struct AqvmBaseFile_File* AqvmBaseFile_tmpfile() {
  struct AqvmBaseFile_File* stream =
      (struct AqvmBaseFile_File*)AqvmBaseMemory_malloc(sizeof(struct AqvmBaseFile_File));
  if (stream == NULL) {
    // TODO(logging)
    return NULL;
  }

  stream->file = tmpfile();
  if (stream->file == NULL || AqvmBaseFile_ferror(stream) != 0) {
    AqvmBaseMemory_free(stream);
    // TODO(logging)
    return NULL;
  }
  stream->identifier = (AqvmBaseFileIdentifier_Identifier*)AqvmBaseMemory_malloc(
      sizeof(AqvmBaseFileIdentifier_Identifier));
  if (stream->identifier == NULL) {
    // TODO(logging)
    fclose(stream->file);
    AqvmBaseMemory_free(stream);
    return NULL;
  }
  if (AqvmBaseThreadingFileLock_AddFileLock(stream) != 0) {
    // TODO(logging)
    fclose(stream->file);
    AqvmBaseMemory_free(stream->identifier);
    AqvmBaseMemory_free(stream);
    return NULL;
  }
  return stream;
}

char* AqvmBaseFile_tmpnam(char* str) {
  // TODO(logging)
  char* result = tmpnam(str);
  if (result == NULL) {
    // TODO(logging)
    return NULL;
  }
  return result;
}