#ifndef FAKE86_MUTEX_H_INCLUDED
#define FAKE86_MUTEX_H_INCLUDED

#if 0
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define MutexLock(mutex) EnterCriticalSection(&mutex)
#define MutexUnlock(mutex) LeaveCriticalSection(&mutex)
#else
#include <pthread.h>
#define MutexLock(mutex) pthread_mutex_lock(&mutex)
#define MutexUnlock(mutex) pthread_mutex_unlock(&mutex)
#endif
#endif

#endif