// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/mutex/mutex.h"

int AqvmBaseThreadingMutex_InitializeMutex(
    AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_InitializeMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_InitializeMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#else
  // TODO
  return -3;
#endif
}

int AqvmBaseThreadingMutex_CloseMutex(AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_CloseMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_CloseMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#else
  // TODO
  return -3;
#endif
}

int AqvmBaseThreadingMutex_LockMutex(AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_LockMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_LockMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#else
  // TODO
  return -3;
#endif
}

int AqvmBaseThreadingMutex_TryLockMutex(AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_TryLockMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_TryLockMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#else
  // TODO
  return -3;
#endif
}

int AqvmBaseThreadingMutex_UnlockMutex(AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_UnlockMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_UnlockMutex(mutex) != 0) {
    return -2;
  }
  return 0;
#else
  // TODO
  return -3;
#endif
}