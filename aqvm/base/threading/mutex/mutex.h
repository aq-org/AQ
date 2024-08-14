// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_THREADING_MUTEX_MUTEX_H_
#define AQ_AQVM_BASE_THREADING_MUTEX_MUTEX_H_

#ifdef __unix__
#include "aqvm/base/threading/mutex/unix/mutex.h"
#elif _WIN32
#include "aqvm/base/threading/mutex/windows/mutex.h"
#endif

#ifdef __unix__
typedef AqvmBaseThreadingMutexUnix_Mutex AqvmBaseThreadingMutex_Mutex;
#elif _WIN32
typedef AqvmBaseThreadingMutexWindows_Mutex AqvmBaseThreadingMutex_Mutex;
#else
// TODO(Threading): When Threading is developed, rewrite that code.
typedef bool AqvmBaseThreadingMutex_Mutex;
#endif

AqvmBaseThreadingMutex_Mutex* AqvmBaseThreadingMutex_CreateMutex();

int AqvmBaseThreadingMutex_DestroyMutex(AqvmBaseThreadingMutex_Mutex* mutex);

int AqvmBaseThreadingMutex_LockMutex(AqvmBaseThreadingMutex_Mutex* mutex);

int AqvmBaseThreadingMutex_TryLockMutex(AqvmBaseThreadingMutex_Mutex* mutex);

int AqvmBaseThreadingMutex_UnlockMutex(AqvmBaseThreadingMutex_Mutex* mutex);

#endif