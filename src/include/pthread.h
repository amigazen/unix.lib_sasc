/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * Hybrid pthread implementation for Amiga
 */

#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <setjmp.h>
#include <time.h>

/* POSIX timespec structure - only define if not already defined */
#ifndef _TIMESPEC_DEFINED
struct timespec {
    time_t tv_sec;   /* seconds */
    long tv_nsec;    /* nanoseconds */
};
#define _TIMESPEC_DEFINED
#endif

/* POSIX thread types and constants */
typedef ULONG pthread_t;
typedef struct pthread_attr pthread_attr_t;
typedef struct pthread_mutex pthread_mutex_t;
typedef struct pthread_mutexattr pthread_mutexattr_t;
typedef struct pthread_cond pthread_cond_t;
typedef struct pthread_condattr pthread_condattr_t;
typedef struct pthread_rwlock pthread_rwlock_t;
typedef struct pthread_rwlockattr pthread_rwlockattr_t;
typedef ULONG pthread_key_t;
typedef struct pthread_once pthread_once_t;

/* Thread attribute constants */
#define PTHREAD_CREATE_JOINABLE    0
#define PTHREAD_CREATE_DETACHED    1

/* Mutex types */
#define PTHREAD_MUTEX_NORMAL       0
#define PTHREAD_MUTEX_RECURSIVE    1
#define PTHREAD_MUTEX_ERRORCHECK   2
#define PTHREAD_MUTEX_DEFAULT      PTHREAD_MUTEX_NORMAL

/* Cancellation types */
#define PTHREAD_CANCEL_ENABLE      0
#define PTHREAD_CANCEL_DISABLE     1
#define PTHREAD_CANCEL_DEFERRED    0
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

/* Return values */
#define PTHREAD_CANCELED           ((void *)-1)
#define PTHREAD_BARRIER_SERIAL_THREAD 1

/* Limits */
#define PTHREAD_THREADS_MAX        64
#define PTHREAD_KEYS_MAX           32

/* Error codes - only define if not already defined */
#ifndef EINVAL
#define EINVAL     22
#endif
#ifndef EAGAIN
#define EAGAIN     11
#endif
#ifndef ESRCH
#define ESRCH      3
#endif
#ifndef EBUSY
#define EBUSY      16
#endif
#ifndef EDEADLK
#define EDEADLK    45
#endif
#ifndef EPERM
#define EPERM      1
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT  60
#endif
#ifndef ENOSYS
#define ENOSYS     78
#endif

/* Additional constants */
#define PTHREAD_STACK_MIN 8192
#define PTHREAD_SCOPE_SYSTEM 0
#define PTHREAD_SCOPE_PROCESS 1
#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_EXPLICIT_SCHED 1
#define SCHED_OTHER 0
#define SCHED_FIFO 1
#define SCHED_RR 2

/* Scheduler parameter structure - must be defined before pthread_attr */
struct sched_param {
    int sched_priority;
};

/* Thread attributes structure */
struct pthread_attr {
    int detachstate;
    void *stackaddr;
    size_t stacksize;
    int scope;
    int inheritsched;
    int schedpolicy;
    struct sched_param schedparam;
};

/* Mutex structure */
struct pthread_mutex {
    struct SignalSemaphore semaphore;
    int kind;
    int incond;
    struct List waiters;
};

/* Mutex initializer macro */
#define PTHREAD_MUTEX_INITIALIZER { {0}, PTHREAD_MUTEX_DEFAULT, 0, {0} }

/* Condition variable initializer macro */
#define PTHREAD_COND_INITIALIZER { {0}, {0} }

/* Read-write lock initializer macro */
#define PTHREAD_RWLOCK_INITIALIZER { {0}, 0, 0, {0}, {0} }

/* Mutex attributes */
struct pthread_mutexattr {
    int kind;
};

/* Condition variable structure */
struct pthread_cond {
    struct SignalSemaphore semaphore;
    struct List waiters;
};

/* Condition variable attributes */
struct pthread_condattr {
    int dummy; /* Placeholder for future attributes */
};

/* Read-write lock structure */
struct pthread_rwlock {
    pthread_mutex_t lock;
    pthread_cond_t readers_cond;
    pthread_cond_t writers_cond;
    int readers_active;
    int writers_active;
    int writers_waiting;
};

/* Read-write lock attributes */
struct pthread_rwlockattr {
    int dummy; /* Placeholder for future attributes */
};

/* Once control structure */
struct pthread_once {
    int done;
    struct SignalSemaphore semaphore;
};

/* Thread management functions */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
                   void *(*start_routine)(void *), void *arg);
void pthread_exit(void *value_ptr);
int pthread_join(pthread_t thread, void **value_ptr);
int pthread_detach(pthread_t thread);
pthread_t pthread_self(void);
int pthread_equal(pthread_t t1, pthread_t t2);

/* Thread attributes */
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
/* TODO: Implement these functions */
/* int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, size_t *stacksize); */
/* int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize); */
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);

/* Mutex functions */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime);

/* Mutex attributes */
/* TODO: Implement mutex attributes */
/* int pthread_mutexattr_init(pthread_mutexattr_t *attr); */
/* int pthread_mutexattr_destroy(pthread_mutexattr_t *attr); */
/* int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type); */
/* int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type); */

/* Condition variable functions */
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, 
                          const struct timespec *abstime);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

/* Condition variable attributes */
/* TODO: Implement condition variable attributes */
/* int pthread_condattr_init(pthread_condattr_t *attr); */
/* int pthread_condattr_destroy(pthread_condattr_t *attr); */

/* Read-write lock functions */
int pthread_rwlock_init(pthread_rwlock_t *lock, const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *lock);
int pthread_rwlock_rdlock(pthread_rwlock_t *lock);
int pthread_rwlock_wrlock(pthread_rwlock_t *lock);
int pthread_rwlock_unlock(pthread_rwlock_t *lock);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *lock);
int pthread_rwlock_trywrlock(pthread_rwlock_t *lock);
/* TODO: Implement timed functions */
/* int pthread_rwlock_timedrdlock(pthread_rwlock_t *lock, const struct timespec *abstime); */
/* int pthread_rwlock_timedwrlock(pthread_rwlock_t *lock, const struct timespec *abstime); */

/* Read-write lock attributes */
/* TODO: Implement read-write lock attributes */
/* int pthread_rwlockattr_init(pthread_rwlockattr_t *attr); */
/* int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr); */

/* Thread-specific storage */
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t key);

/* Cancellation */
int pthread_cancel(pthread_t thread);
void pthread_testcancel(void);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);

/* Cleanup */
/* TODO: Implement cleanup handlers */
/* void pthread_cleanup_push(void (*routine)(void *), void *arg); */
/* void pthread_cleanup_pop(int execute); */

/* Once */
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

/* Thread naming (non-standard extension) */
/* TODO: Implement thread naming */
/* int pthread_setname_np(pthread_t thread, const char *name); */

#endif /* _PTHREAD_H */
