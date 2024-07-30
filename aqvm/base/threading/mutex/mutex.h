// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_THREADING_MUTEX_MUTEX_H_
#define AQ_AQVM_BASE_THREADING_MUTEX_MUTEX_H_

#include "aqvm/base/threading/mutex/unix/mutex.h"
#include "aqvm/base/threading/mutex/windows/mutex.h"

#ifdef __unix__
typedef AqvmBaseThreadingMutexUnix_Mutex AqvmBaseThreadingMutex_Mutex;
#elif _WIN32
typedef AqvmBaseThreadingMutexWindows_Mutex AqvmBaseThreadingMutex_Mutex;
#else
// TODO(Threading): When Threading is developed, rewrite that code.
typedef bool AqvmBaseThreadingMutex_Mutex;
#endif

int AqvmBaseThreadingMutex_InitializeMutex(AqvmBaseThreadingMutex_Mutex *mutex);

int AqvmBaseThreadingMutex_CloseMutex(AqvmBaseThreadingMutex_Mutex *mutex);

int AqvmBaseThreadingMutex_LockMutex(AqvmBaseThreadingMutex_Mutex *mutex);

int AqvmBaseThreadingMutex_TryLockMutex(AqvmBaseThreadingMutex_Mutex *mutex);

int AqvmBaseThreadingMutex_UnlockMutex(AqvmBaseThreadingMutex_Mutex *mutex);

#endif