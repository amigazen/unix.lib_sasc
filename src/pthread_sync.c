/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * 
 * Synchronization primitives for pthread implementation
 */

#include "include/amigapthread.h"
#include "include/pthread.h"
#include "include/semaphore.h"

/* Forward declaration for condition variable waiters */
typedef struct CondWaiter {
    struct Node node;
    struct Task *task;
    ULONG sigmask;
} CondWaiter;

/*
 * Initialize a mutex
 */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    if (mutex == NULL) {
        return EINVAL;
    }
    
    /* Initialize the semaphore */
    InitSemaphore(&mutex->semaphore);
    mutex->kind = PTHREAD_MUTEX_DEFAULT;
    mutex->incond = 0;
    NewList(&mutex->waiters);
    
    return 0;
}

/*
 * Destroy a mutex
 */
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if (mutex == NULL) {
        return EINVAL;
    }
    
    /* Nothing to clean up for Amiga semaphores */
    return 0;
}

/*
 * Lock a mutex
 */
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    /* Check for user abort */
    chkabort();
    
    if (mutex == NULL) {
        return EINVAL;
    }
    
    ObtainSemaphore(&mutex->semaphore);
    
    return 0;
}

/*
 * Try to lock a mutex (non-blocking)
 */
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    if (mutex == NULL) {
        return EINVAL;
    }
    
    if (AttemptSemaphore(&mutex->semaphore)) {
        return 0;
    }
    
    return EBUSY;
}

/*
 * Unlock a mutex
 */
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    /* Check for user abort */
    chkabort();
    
    if (mutex == NULL) {
        return EINVAL;
    }
    
    ReleaseSemaphore(&mutex->semaphore);
    
    return 0;
}

/*
 * Initialize a condition variable
 */
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    if (cond == NULL) {
        return EINVAL;
    }
    
    NewList(&cond->waiters);
    InitSemaphore(&cond->semaphore);
    
    return 0;
}

/*
 * Destroy a condition variable
 */
int pthread_cond_destroy(pthread_cond_t *cond)
{
    if (cond == NULL) {
        return EINVAL;
    }
    
    /* Nothing to clean up */
    return 0;
}

/*
 * Wait on a condition variable
 * CRITICAL: This must be atomic - unlock mutex and wait in one operation
 * 
 * This implementation uses a signal-based approach with proper atomicity
 */
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    struct Process *proc;
    struct Node *node;
    ULONG signal_mask;
    
    if (cond == NULL || mutex == NULL) {
        return EINVAL;
    }
    
    proc = (struct Process *)FindTask(NULL);
    signal_mask = 1L << proc->pr_MsgPort.mp_SigBit;
    
    /* 
     * ATOMIC OPERATION: Add to waiters, unlock mutex, and wait
     * We use Forbid() to prevent context switches during the critical section
     */
    Forbid();
    
    /* Add ourselves to the wait list */
    node = &proc->pr_MsgPort.mp_Node;
    AddTail(&cond->waiters, node);
    
    /* Unlock the mutex while still in Forbid() state */
    ReleaseSemaphore(&mutex->semaphore);
    
    /* 
     * CRITICAL: We must wait while still in Forbid() state to prevent
     * the race condition. We'll use Wait() but keep multitasking disabled
     * until we're actually waiting.
     */
    Wait(signal_mask);
    
    /* When we wake up, we're still in Forbid() state */
    Permit();
    
    /* Relock the mutex when we wake up */
    pthread_mutex_lock(mutex);
    
    return 0;
}

/*
 * Signal a condition variable
 * Wakes up one waiting thread using signals
 */
int pthread_cond_signal(pthread_cond_t *cond)
{
    struct Node *node;
    
    if (cond == NULL) {
        return EINVAL;
    }
    
    /* Check for user abort */
    chkabort();
    
    Forbid();
    node = RemHead(&cond->waiters);
    if (node) {
        /* Calculate Process pointer from MsgPort node */
        struct Process *proc = (struct Process *)((char *)node - ((char *)&((struct Process *)0)->pr_MsgPort.mp_Node));
        Signal((struct Task *)proc, 1L << proc->pr_MsgPort.mp_SigBit);
    }
    Permit();
    
    return 0;
}

/*
 * Broadcast to a condition variable
 * Wakes up all waiting threads using signals
 */
int pthread_cond_broadcast(pthread_cond_t *cond)
{
    struct Node *node;
    
    if (cond == NULL) {
        return EINVAL;
    }
    
    /* Check for user abort */
    chkabort();
    
    Forbid();
    while ((node = RemHead(&cond->waiters))) {
        /* Calculate Process pointer from MsgPort node */
        struct Process *proc = (struct Process *)((char *)node - ((char *)&((struct Process *)0)->pr_MsgPort.mp_Node));
        Signal((struct Task *)proc, 1L << proc->pr_MsgPort.mp_SigBit);
    }
    Permit();
    
    return 0;
}

/*
 * Initialize a read-write lock
 * Uses a proper algorithm with mutex and condition variables
 */
int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
    if (rwlock == NULL) {
        return EINVAL;
    }
    
    /* Initialize the main mutex */
    InitSemaphore(&rwlock->lock.semaphore);
    rwlock->lock.kind = PTHREAD_MUTEX_DEFAULT;
    rwlock->lock.incond = 0;
    NewList(&rwlock->lock.waiters);
    
    /* Initialize condition variables */
    NewList(&rwlock->readers_cond.waiters);
    InitSemaphore(&rwlock->readers_cond.semaphore);
    NewList(&rwlock->writers_cond.waiters);
    InitSemaphore(&rwlock->writers_cond.semaphore);
    
    /* Initialize counters */
    rwlock->readers_active = 0;
    rwlock->writers_active = 0;
    rwlock->writers_waiting = 0;
    
    return 0;
}

/*
 * Destroy a read-write lock
 */
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        return EINVAL;
    }
    
    /* Nothing to clean up */
    return 0;
}

/*
 * Lock a read-write lock for reading
 * Readers can proceed if no writers are active or waiting
 */
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        return EINVAL;
    }
    
    /* Check for user abort */
    chkabort();
    
    pthread_mutex_lock(&rwlock->lock);
    
    /* Wait while writers are active or waiting */
    while (rwlock->writers_active > 0 || rwlock->writers_waiting > 0) {
        pthread_cond_wait(&rwlock->readers_cond, &rwlock->lock);
    }
    
    rwlock->readers_active++;
    pthread_mutex_unlock(&rwlock->lock);
    
    return 0;
}

/*
 * Try to lock a read-write lock for reading
 */
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        return EINVAL;
    }
    
    /* Check for user abort */
    chkabort();
    
    if (pthread_mutex_trylock(&rwlock->lock) != 0) {
        return EBUSY;
    }
    
    /* Check if we can proceed immediately */
    if (rwlock->writers_active > 0 || rwlock->writers_waiting > 0) {
        pthread_mutex_unlock(&rwlock->lock);
        return EBUSY;
    }
    
    rwlock->readers_active++;
    pthread_mutex_unlock(&rwlock->lock);
    
    return 0;
}

/*
 * Lock a read-write lock for writing
 * Writers must wait for all readers and other writers to finish
 */
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        return EINVAL;
    }
    
    /* Check for user abort */
    chkabort();
    
    pthread_mutex_lock(&rwlock->lock);
    
    /* Increment waiting writers count */
    rwlock->writers_waiting++;
    
    /* Wait while readers or other writers are active */
    while (rwlock->readers_active > 0 || rwlock->writers_active > 0) {
        pthread_cond_wait(&rwlock->writers_cond, &rwlock->lock);
    }
    
    /* We can now write */
    rwlock->writers_waiting--;
    rwlock->writers_active++;
    pthread_mutex_unlock(&rwlock->lock);
    
    return 0;
}

/*
 * Try to lock a read-write lock for writing
 */
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        return EINVAL;
    }
    
    /* Check for user abort */
    chkabort();
    
    if (pthread_mutex_trylock(&rwlock->lock) != 0) {
        return EBUSY;
    }
    
    /* Check if we can proceed immediately */
    if (rwlock->readers_active > 0 || rwlock->writers_active > 0) {
        pthread_mutex_unlock(&rwlock->lock);
        return EBUSY;
    }
    
    rwlock->writers_active++;
    pthread_mutex_unlock(&rwlock->lock);
    
    return 0;
}

/*
 * Unlock a read-write lock
 * Must determine if we're releasing a reader or writer and signal appropriately
 */
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    if (rwlock == NULL) {
        return EINVAL;
    }
    
    /* Check for user abort */
    chkabort();
    
    pthread_mutex_lock(&rwlock->lock);
    
    if (rwlock->writers_active > 0) {
        /* Releasing a writer */
        rwlock->writers_active--;
        
        /* Signal waiting writers first, then readers */
        if (rwlock->writers_waiting > 0) {
            pthread_cond_signal(&rwlock->writers_cond);
        } else {
            pthread_cond_broadcast(&rwlock->readers_cond);
        }
    } else if (rwlock->readers_active > 0) {
        /* Releasing a reader */
        rwlock->readers_active--;
        
        /* If this was the last reader, signal waiting writers */
        if (rwlock->readers_active == 0 && rwlock->writers_waiting > 0) {
            pthread_cond_signal(&rwlock->writers_cond);
        }
    }
    
    pthread_mutex_unlock(&rwlock->lock);
    
    return 0;
}

/* Semaphore functions are now implemented in semaphore_impl.c */

/*
 * Lock a mutex with timeout
 */
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
    struct MsgPort *timermp;
    struct TimeRequest *timerio;
    BYTE timersignal;
    ULONG timer_sig, break_sigs, signals;
    struct Task *task;
    CondWaiter waiter;
    int result;
    
    /* Initialize variables */
    timermp = NULL;
    timerio = NULL;
    result = 0;
    
    /* Check for user abort */
    chkabort();
    
    if (mutex == NULL || abstime == NULL) {
        return EINVAL;
    }
    
    task = FindTask(NULL);
    
    /* Try to acquire the mutex immediately first */
    if (AttemptSemaphore(&mutex->semaphore) == TRUE) {
        return 0;
    }
    
    /* Set up timer for timeout */
    timermp = AllocVec(sizeof(struct MsgPort), MEMF_CLEAR | MEMF_PUBLIC);
    timerio = AllocVec(sizeof(struct TimeRequest), MEMF_CLEAR | MEMF_PUBLIC);
    timersignal = AllocSignal(-1);
    
    if (!timermp || !timerio || timersignal == -1) {
        if (timersignal != -1) FreeSignal(timersignal);
        if (timerio) FreeVec(timerio);
        if (timermp) FreeVec(timermp);
        return EAGAIN;
    }
    
    /* Initialize timer message port */
    timermp->mp_Node.ln_Type = NT_MSGPORT;
    timermp->mp_Flags = PA_SIGNAL;
    timermp->mp_SigBit = timersignal;
    timermp->mp_SigTask = task;
    
    /* Initialize timer request */
    timerio->tr_node.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    timerio->tr_node.io_Message.mn_ReplyPort = timermp;
    
    /* Open timer device */
    if (OpenDevice((STRPTR)TIMERNAME, UNIT_MICROHZ, (struct IORequest *)timerio, 0) != 0) {
        FreeSignal(timersignal);
        FreeVec(timerio);
        FreeVec(timermp);
        return EINVAL;
    }
    
    /* Set up timer request */
    timerio->tr_node.io_Command = TR_ADDREQUEST;
    timerio->tr_time.tv_secs = abstime->tv_sec;
    timerio->tr_time.tv_micro = abstime->tv_nsec / 1000;
    
    /* Set up waiter for mutex availability notification */
    waiter.task = task;
    waiter.sigmask = 1L << AllocSignal(-1);
    if (waiter.sigmask == 0) {
        CloseDevice((struct IORequest *)timerio);
        FreeSignal(timersignal);
        FreeVec(timerio);
        FreeVec(timermp);
        return EAGAIN;
    }
    
    /* Add ourselves to the mutex waiters list */
    ObtainSemaphore(&mutex->semaphore);
    AddTail((struct List *)&mutex->waiters, (struct Node *)&waiter);
    ReleaseSemaphore(&mutex->semaphore);
    
    /* Set up signals */
    timer_sig = 1L << timersignal;
    break_sigs = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;
    signals = waiter.sigmask | timer_sig | break_sigs;
    
    /* Start timer and wait */
    SendIO((struct IORequest *)timerio);
    signals = Wait(signals);
    
    /* Remove ourselves from the waiters list */
    ObtainSemaphore(&mutex->semaphore);
    Remove((struct Node *)&waiter);
    ReleaseSemaphore(&mutex->semaphore);
    
    FreeSignal(waiter.sigmask >> 1); /* Convert back to signal number */
    
    /* Check for user abort */
    if (signals & break_sigs) {
        chkabort();
    }
    
    /* Check if timer expired */
    if (signals & timer_sig) {
        result = ETIMEDOUT;
    } else {
        /* Try to acquire the mutex one more time */
        if (AttemptSemaphore(&mutex->semaphore) == FALSE) {
            result = ETIMEDOUT; /* Should not happen, but be safe */
        }
    }
    
    /* Clean up timer resources */
    if (!CheckIO((struct IORequest *)timerio)) {
        AbortIO((struct IORequest *)timerio);
    }
    WaitIO((struct IORequest *)timerio);
    CloseDevice((struct IORequest *)timerio);
    FreeSignal(timersignal);
    FreeVec(timerio);
    FreeVec(timermp);
    
    return result;
}

/*
 * Wait on condition variable with timeout
 */
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    struct MsgPort *timermp;
    struct TimeRequest *timerio;
    BYTE timersignal;
    ULONG timer_sig, break_sigs, signals;
    struct Task *task;
    CondWaiter waiter;
    int result;
    
    /* Initialize variables */
    timermp = NULL;
    timerio = NULL;
    result = 0;
    
    /* Check for user abort */
    chkabort();
    
    if (cond == NULL || mutex == NULL || abstime == NULL) {
        return EINVAL;
    }
    
    task = FindTask(NULL);
    
    /* Set up timer for timeout */
    timermp = AllocVec(sizeof(struct MsgPort), MEMF_CLEAR | MEMF_PUBLIC);
    timerio = AllocVec(sizeof(struct TimeRequest), MEMF_CLEAR | MEMF_PUBLIC);
    timersignal = AllocSignal(-1);
    
    if (!timermp || !timerio || timersignal == -1) {
        if (timersignal != -1) FreeSignal(timersignal);
        if (timerio) FreeVec(timerio);
        if (timermp) FreeVec(timermp);
        return EAGAIN;
    }
    
    /* Initialize timer message port */
    timermp->mp_Node.ln_Type = NT_MSGPORT;
    timermp->mp_Flags = PA_SIGNAL;
    timermp->mp_SigBit = timersignal;
    timermp->mp_SigTask = task;
    
    /* Initialize timer request */
    timerio->tr_node.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    timerio->tr_node.io_Message.mn_ReplyPort = timermp;
    
    /* Open timer device */
    if (OpenDevice((STRPTR)TIMERNAME, UNIT_MICROHZ, (struct IORequest *)timerio, 0) != 0) {
        FreeSignal(timersignal);
        FreeVec(timerio);
        FreeVec(timermp);
        return EINVAL;
    }
    
    /* Set up timer request */
    timerio->tr_node.io_Command = TR_ADDREQUEST;
    timerio->tr_time.tv_secs = abstime->tv_sec;
    timerio->tr_time.tv_micro = abstime->tv_nsec / 1000;
    
    /* Set up waiter for condition variable notification */
    waiter.task = task;
    waiter.sigmask = 1L << AllocSignal(-1);
    if (waiter.sigmask == 0) {
        CloseDevice((struct IORequest *)timerio);
        FreeSignal(timersignal);
        FreeVec(timerio);
        FreeVec(timermp);
        return EAGAIN;
    }
    
    /* Add ourselves to the condition variable waiters list */
    ObtainSemaphore(&cond->semaphore);
    AddTail((struct List *)&cond->waiters, (struct Node *)&waiter);
    ReleaseSemaphore(&cond->semaphore);
    
    /* Mark mutex as being used in condition wait */
    mutex->incond++;
    
    /* Unlock the mutex */
    ReleaseSemaphore(&mutex->semaphore);
    
    /* Set up signals */
    timer_sig = 1L << timersignal;
    break_sigs = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D;
    signals = waiter.sigmask | timer_sig | break_sigs;
    
    /* Start timer and wait */
    SendIO((struct IORequest *)timerio);
    signals = Wait(signals);
    
    /* Remove ourselves from the waiters list */
    ObtainSemaphore(&cond->semaphore);
    Remove((struct Node *)&waiter);
    ReleaseSemaphore(&cond->semaphore);
    
    FreeSignal(waiter.sigmask >> 1); /* Convert back to signal number */
    
    /* Re-lock the mutex */
    ObtainSemaphore(&mutex->semaphore);
    mutex->incond--;
    
    /* Check for user abort */
    if (signals & break_sigs) {
        chkabort();
    }
    
    /* Check if timer expired */
    if (signals & timer_sig) {
        result = ETIMEDOUT;
    }
    
    /* Clean up timer resources */
    if (!CheckIO((struct IORequest *)timerio)) {
        AbortIO((struct IORequest *)timerio);
    }
    WaitIO((struct IORequest *)timerio);
    CloseDevice((struct IORequest *)timerio);
    FreeSignal(timersignal);
    FreeVec(timerio);
    FreeVec(timermp);
    
    return result;
}