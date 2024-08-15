#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/mutex/unix/mutex.h"

#include "aqvm/base/memory/memory.h"

AqvmBaseThreadingMutexUnix_Mutex *AqvmBaseThreadingMutexUnix_CreateMutex() {
  AqvmBaseThreadingMutexUnix_Mutex *mutex =
      (AqvmBaseThreadingMutexUnix_Mutex *)AqvmBaseMemory_malloc(
          sizeof(AqvmBaseThreadingMutexUnix_Mutex));
  if (mutex == NULL) {
    // TODO
    return NULL;
  }
  int result = pthread_mutex_init(mutex, NULL);
  if (result != 0) {
    // TODO
    return NULL;
  }
  return mutex;
}

int AqvmBaseThreadingMutexUnix_DestroyMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

  int result = pthread_mutex_destroy(mutex);
  AqvmBaseMemory_free(mutex);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseThreadingMutexUnix_LockMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

  int result = pthread_mutex_lock(mutex);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseThreadingMutexUnix_TryLockMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

  int result = pthread_mutex_trylock(mutex);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseThreadingMutexUnix_UnlockMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

  int result = pthread_mutex_unlock(mutex);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

#endif