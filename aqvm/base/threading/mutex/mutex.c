// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/mutex/mutex.h"

AqvmBaseThreadingMutex_Mutex* AqvmBaseThreadingMutex_CreateMutex() {
#ifdef __unix__
  return AqvmBaseThreadingMutexUnix_CreateMutex();
#elif _WIN32
  return AqvmBaseThreadingMutexWindows_CreateMutex();
#else
  // TODO(Threading): When Threading is developed, rewrite that code.
  AqvmBaseThreadingMutex_Mutex* mutex = (AqvmBaseThreadingMutex_Mutex*)malloc(
      sizeof(AqvmBaseThreadingMutex_Mutex));
  if (mutex == NULL) {
    // TODO
    return NULL;
  }
  *mutex = false;
  return mutex;
#endif
}

int AqvmBaseThreadingMutex_DestroyMutex(AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_DestroyMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_DestroyMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#else
  // TODO(Threading): When Threading is developed, rewrite that code.
  free(mutex);
  return 0;
#endif
}

int AqvmBaseThreadingMutex_LockMutex(AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_LockMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_LockMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#else
  // TODO(Threading): When Threading is developed, rewrite that code.
  while (*mutex);
  *mutex = true;
  return 0;
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
  // TODO(Threading): When Threading is developed, rewrite that code.
  if (*mutex) {
    return -3;
  }
  *mutex = true;
  return 0;
#endif
}

int AqvmBaseThreadingMutex_UnlockMutex(AqvmBaseThreadingMutex_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

#ifdef __unix__
  if (AqvmBaseThreadingMutexUnix_UnlockMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#elif _WIN32
  if (AqvmBaseThreadingMutexWindows_UnlockMutex(mutex) != 0) {
    // TODO
    return -2;
  }
  return 0;
#else
  // TODO(Threading): When Threading is developed, rewrite that code.
  if (!*mutex) {
    return -3;
  }
  *mutex = false;
  return 0;
#endif
}