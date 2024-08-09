// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/file/read_write_lock/read_write_lock.h"

int AqvmBaseFileReadWriteLock_InitializeReadWriteLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock) {
  if (read_write_lock == NULL) {
    // TODO
    return -1;
  }

  if (AqvmBaseThreadingMutex_InitializeMutex(&read_write_lock->mutex) != 0) {
    // TODO
    return -2;
  }
  read_write_lock->read_count = 0;
  return 0;
}

int AqvmBaseFileReadWriteLock_CloseReadWriteLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock) {
  if (read_write_lock == NULL) {
    // TODO
    return -1;
  }

  if (AqvmBaseThreadingMutex_CloseMutex(&read_write_lock->mutex) != 0) {
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