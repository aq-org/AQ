#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_THREADING_MUTEX_WINDOWS_MUTEX_H_
#define AQ_AQVM_BASE_THREADING_MUTEX_WINDOWS_MUTEX_H_

#include <windows.h>

typedef HANDLE AqvmBaseThreadingMutexWindows_Mutex;

AqvmBaseThreadingMutexWindows_Mutex*
AqvmBaseThreadingMutexWindows_CreateMutex();

int AqvmBaseThreadingMutexWindows_DestroyMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex);

int AqvmBaseThreadingMutexWindows_LockMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex);

int AqvmBaseThreadingMutexWindows_TryLockMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex);

int AqvmBaseThreadingMutexWindows_UnlockMutex(
    AqvmBaseThreadingMutexWindows_Mutex* mutex);

#endif
#endif