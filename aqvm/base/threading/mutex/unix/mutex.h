#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_THREADING_MUTEX_UNIX_MUTEX_H_
#define AQ_AQVM_BASE_THREADING_MUTEX_UNIX_MUTEX_H_

#include <pthread.h>

typedef pthread_mutex_t AqvmBaseThreadingMutexUnix_Mutex;

AqvmBaseThreadingMutexUnix_Mutex *AqvmBaseThreadingMutexUnix_CreateMutex();

int AqvmBaseThreadingMutexUnix_DestroyMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex);

int AqvmBaseThreadingMutexUnix_LockMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex);

int AqvmBaseThreadingMutexUnix_TryLockMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex);

int AqvmBaseThreadingMutexUnix_UnlockMutex(
    AqvmBaseThreadingMutexUnix_Mutex *mutex);

#endif
#endif