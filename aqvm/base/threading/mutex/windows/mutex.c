#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/threading/mutex/windows/mutex.h"

#include <stdio.h>
#include <windows.h>

AqvmBaseThreadingMutexWindows_Mutex*
AqvmBaseThreadingMutexWindows_CreateMutex() {
  AqvmBaseThreadingMutexWindows_Mutex* mutex =
      (AqvmBaseThreadingMutexWindows_Mutex*)malloc(
          sizeof(AqvmBaseThreadingMutexWindows_Mutex));
  if (mutex == NULL) {
    // TODO
    return NULL;
  }
  *mutex = CreateMutex(NULL, FALSE, NULL);
  if (*mutex == NULL) {
    // TODO
    return NULL;
  }
  return 0;
}

int AqvmBaseThreadingMutexWindows_DestroyMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex) {
  if (mutex == NULL) {
    // TODO
    return -1;
  }

  if (*mutex != NULL && !CloseHandle(*mutex)) {
    // TODO
    return -2;
  }
  free(mutex);
  return 0;
}

int AqvmBaseThreadingMutexWindows_LockMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex) {
  if (mutex == NULL || *mutex == NULL) {
    // TODO
    return -1;
  }

  DWORD result = WaitForSingleObject(*mutex, INFINITE);
  if (result != WAIT_OBJECT_0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseThreadingMutexWindows_TryLockMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex) {
  if (mutex == NULL || *mutex == NULL) {
    // TODO
    return -1;
  }

  DWORD result = WaitForSingleObject(*mutex, 0);
  if (result != WAIT_OBJECT_0) {
    // TODO
    return -2;
  }
  return 0;
}

int AqvmBaseThreadingMutexWindows_UnlockMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex) {
  if (mutex == NULL || *mutex == NULL) {
    // TODO
    return -1;
  }

  if (!ReleaseMutex(*mutex)) {
    // TODO
    return -2;
  }
  return 0;
}

#endif