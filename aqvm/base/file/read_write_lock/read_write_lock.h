// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_FILE_READ_WRITE_LOCK_READ_WRITE_LOCK_H_
#define AQ_AQVM_BASE_FILE_READ_WRITE_LOCK_READ_WRITE_LOCK_H_

#include "aqvm/base/threading/mutex/mutex.h"

struct AqvmBaseFileReadWriteLock_ReadWriteLock {
  AqvmBaseThreadingMutex_Mutex mutex;
  int lock_count;
  int read_count;
};

struct AqvmBaseFileReadWriteLock_ReadWriteLock*
AqvmBaseFileReadWriteLock_CreateReadWriteLock();

int AqvmBaseFileReadWriteLock_DestroyReadWriteLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock);

int AqvmBaseFileReadWriteLock_LockReadLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock);

int AqvmBaseFileReadWriteLock_LockWriteLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock);

int AqvmBaseFileReadWriteLock_UnlockLock(
    struct AqvmBaseFileReadWriteLock_ReadWriteLock* read_write_lock);

#endif