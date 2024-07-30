#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/mutex/unix/mutex.h"

int AqvmBaseThreadingMutexUnix_InitializeMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

  int result = pthread_mutex_init(mutex, NULL);
  if (result != 0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseThreadingMutexUnix_CloseMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

  int result = pthread_mutex_destroy(mutex);
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