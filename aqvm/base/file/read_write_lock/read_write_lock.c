// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/read_write_lock/read_write_lock.h"

struct AqvmBaseFileReadWriteLock_ReadWriteLock*
AqvmBaseFileReadWriteLock_CreateReadWriteLock() {
  struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock =
      (struct AqvmBaseFileReadWriteLock_ReadWriteLock*)AqvmBaseMemory_malloc(
          sizeof(struct AqvmBaseFileReadWriteLock_ReadWriteLock));
  if (read_write_lock == NULL) {
    // TODO
    return NULL;
  }
  if (AqvmBaseThreadingMutex_CreateMutex(&read_write_lock->mutex) == NULL) {
    // TODO
    return NULL;
  }
  read_write_lock->read_count = 0;
  return 0;
}

int AqvmBaseFileReadWriteLock_DestroyReadWriteLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock) {
  if (read_write_lock == NULL) {
    // TODO
    return -1;
  }

  int result = AqvmBaseThreadingMutex_DestoryMutex(&read_write_lock->mutex);
  free(read_write_lock);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseFileReadWriteLock_LockReadLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock) {
  if (read_write_lock == NULL) {
    // TODO
    return -1;
  }

  if (read_write_lock->read_count == 0 &&
      AqvmBaseThreadingMutex_LockMutex(&read_write_lock->mutex) != 0) {
    // TODO
    return -2;
  }
  ++read_write_lock->read_count;
  return 0;
}

int AqvmBaseFileReadWriteLock_LockWriteLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock) {
  if (read_write_lock == NULL) {
    // TODO
    return -1;
  }

  if (AqvmBaseThreadingMutex_LockMutex(&read_write_lock->mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseFileReadWriteLock_UnlockLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock) {
  if (read_write_lock == NULL) {
    // TODO
    return -1;
  }

  if (read_write_lock->read_count == 0 &&
      AqvmBaseThreadingMutex_UnlockMutex(&read_write_lock->mutex) != 0) {
    // TODO
    return -2;
  } else {
    --read_write_lock->read_count;
    if (read_write_lock->read_count == 0 &&
        AqvmBaseThreadingMutex_UnlockMutex(&read_write_lock->mutex) != 0) {
      // TODO
      return -3;
    }
  }
  return 0;
}