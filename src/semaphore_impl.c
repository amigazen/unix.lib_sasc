/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on libsem implementation by Diego Casorran (Public Domain)
 *
 * POSIX semaphore API implementation
 */

#include "include/semaphore.h"
#include "include/pthread.h"
#include "debug.h"
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <proto/dos.h>

/*
 * Initialize an unnamed semaphore
 */
int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    psem_t *psem;
    
    if ((sem == NULL) || (value > SEM_VALUE_MAX)) {
        errno = EINVAL;
        return -1;
    }
    
    psem = psem_init(NULL, pshared, value);
    
    memcpy(sem, psem, sizeof(psem_t));
    
    return 0;
}

/*
 * Destroy an unnamed semaphore
 */
int sem_destroy(sem_t *sem)
{
    psem_t *psem = sem2psem(sem);
    
    ENTER();
    SHOWPOINTER(psem);
    
    if (_PSEM_INVALID(psem)) {
        D(("Invalid sem_t provided, magic=%lx\n", ((psem_t *)sem)->magic));
        errno = EINVAL;
        return -1;
    }
    
    psem_destroy(psem);
    memset(sem, _PSEM_ERASEDBIT, sizeof(*sem));
    
    LEAVE();
    return 0;
}

/*
 * Wait on a semaphore
 */
int sem_wait(sem_t *sem)
{
    psem_t *psem = sem2psem(sem);
    
    if (_PSEM_INVALID(psem)) {
        D(("Invalid sem_t provided, magic=%lx\n", ((psem_t *)sem)->magic));
        errno = EINVAL;
        return -1;
    }
    
    return (psem_wait(psem));
}

/*
 * Try to wait on a semaphore (non-blocking)
 */
int sem_trywait(sem_t *sem)
{
    psem_t *psem = sem2psem(sem);
    
    if (_PSEM_INVALID(psem)) {
        D(("Invalid sem_t provided, magic=%lx\n", ((psem_t *)sem)->magic));
        errno = EINVAL;
        return -1;
    }
    
    return (psem_trywait(psem));
}

/*
 * Post to a semaphore
 */
int sem_post(sem_t *sem)
{
    psem_t *psem = sem2psem(sem);
    
    ENTER();
    SHOWPOINTER(psem);
    
    if (_PSEM_INVALID(psem)) {
        D(("Invalid sem_t provided, magic=%lx\n", ((psem_t *)sem)->magic));
        errno = EINVAL;
        return -1;
    }
    
    Forbid();
    if ((_SSEM(psem)->ss_QueueCount != -1)
        && ((_SSEM(psem)->ss_Owner == NULL) || (_SSEM(psem)->ss_Owner == FindTask(NULL)))) {
        ReleaseSemaphore(_SSEM(psem));
    }
    if ((psem->flags & SEMF_EXPUNGE) && (_SSEM(psem)->ss_QueueCount == -1)) {
        D(("Delayed expunge will take action now...\n"));
        psem_destroy(psem);
        memset(sem, _PSEM_ERASEDBIT, sizeof(*sem));
    }
    Permit();
    
    LEAVE();
    return 0;
}

/*
 * Get semaphore value
 */
int sem_getvalue(sem_t *sem, int *sval)
{
    psem_t *psem = sem2psem(sem);
    
    if (_PSEM_INVALID(psem) || (sval == NULL)) {
        D(("Invalid sem_t provided, magic=%lx\n", ((psem_t *)sem)->magic));
        errno = EINVAL;
        return -1;
    }
    
    if (AttemptSemaphore(_SSEM(psem))) {
        (*sval) = 1 + psem->value;
        ReleaseSemaphore(_SSEM(psem));
    } else {
        (*sval) = -_SSEM(psem)->ss_QueueCount;
    }
    
    return 0;
}

/*
 * Wait on a semaphore with timeout
 */
int sem_timedwait(sem_t *sem, const struct timespec *abstime)
{
    psem_t *psem = sem2psem(sem);
    time_t sec;
    
    if (_PSEM_INVALID(psem)) {
        D(("Invalid sem_t provided, magic=%lx\n", ((psem_t *)sem)->magic));
        errno = EINVAL;
        return -1;
    }
    
    if (psem_trywait(psem) == 0) {
        return 0;
    }
    
    sec = (time(NULL) - abstime->tv_sec);
    
    while (sec > 0) {
        Delay(TICKS_PER_SECOND);
        
        if (psem_trywait(psem) == 0) {
            return 0;
        }
        
        sec--;
    }
    
    errno = ETIMEDOUT;
    return -1;
}

/*
 * Open a named semaphore
 */
sem_t *sem_open(const char *name, int oflag, ...)
{
    unsigned char sem_name[_PSEM_NAME_MAXLENGTH];
    struct SignalSemaphore *ssem;
    psem_t *psem = NULL;
    mode_t mode = 0;
    unsigned value = 0;
    sem_t *rc = SEM_FAILED;
    
    if (strlen(name) > (_PSEM_NAME_MAXLENGTH - 20)) {
        errno = ENAMETOOLONG;
        return (SEM_FAILED);
    }
    
    errno = 0;
    psem_name(name, sem_name, sizeof(sem_name) - 1);
    
    Forbid();
    if ((ssem = FindSemaphore((STRPTR)sem_name))) {
        if (((psem_t *)ssem)->magic == _PSEM_MAGIC) {
            psem = (psem_t *)ssem;
        }
    }
    Permit();
    
    if ((ssem != NULL) && (psem == NULL)) {
        errno = ENOSPC;
    } else if (oflag & O_CREAT) {
        if ((oflag & O_EXCL) && (psem != NULL)) {
            errno = EEXIST;
        } else if (psem == NULL) {
            va_list ap;
            
            va_start(ap, oflag);
            mode = va_arg(ap, int);
            value = va_arg(ap, unsigned int);
            va_end(ap);
            
            if (value > SEM_VALUE_MAX) {
                errno = EINVAL;
            } else if ((psem = psem_init(name, 0, value)) == (psem_t *)SEM_FAILED) {
                errno = ENOMEM;
            }
        } else if (psem->owner != (unsigned)FindTask(NULL)) {
            /* Make a dup of this psem to track it down and be sure
             * it's properly released on exit. */
            
            psem_t *semdup;
            
            if ((semdup = AllocVec(sizeof(psem_t), MEMF_PUBLIC))) {
                memcpy(semdup, psem, sizeof(psem_t));
                semdup->flags |= SEMF_OPENDUP;
                
                Forbid();
                psem_list_add(semdup);
                Permit();
            }
        }
    } else if (psem == NULL) {
        errno = ENOENT;
    }
    
    if (errno == 0) {
        rc = (sem_t *)psem;
    }
    
    return (rc);
}

/*
 * Close a named semaphore
 */
int sem_close(sem_t *sem)
{
    psem_t *psem = sem2psem(sem);
    
    if (_PSEM_INVALID(psem)) {
        D(("Invalid sem_t provided, magic=%lx\n", ((psem_t *)sem)->magic));
        errno = EINVAL;
        return -1;
    }
    
    /* For named semaphores opened with sem_open(), we just mark for cleanup */
    if (psem->flags & SEMF_OPENDUP) {
        psem_destroy(psem);
        memset(sem, _PSEM_ERASEDBIT, sizeof(*sem));
    }
    
    return 0;
}

/*
 * Unlink a named semaphore
 */
int sem_unlink(const char *name)
{
    unsigned char sem_name[_PSEM_NAME_MAXLENGTH];
    struct SignalSemaphore *ssem;
    psem_t *psem = NULL;
    
    if (strlen(name) > (_PSEM_NAME_MAXLENGTH - 20)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    
    psem_name(name, sem_name, sizeof(sem_name) - 1);
    
    Forbid();
    if ((ssem = FindSemaphore((STRPTR)sem_name))) {
        if (((psem_t *)ssem)->magic == _PSEM_MAGIC) {
            psem = (psem_t *)ssem;
        }
    }
    Permit();
    
    if (psem == NULL) {
        errno = ENOENT;
        return -1;
    }
    
    /* Mark for delayed expunge */
    psem->flags |= SEMF_EXPUNGE;
    
    return 0;
}
