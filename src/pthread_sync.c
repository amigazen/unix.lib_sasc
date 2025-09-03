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

/*
 * Initialize a semaphore
 */
int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (sem == NULL) {
        return -1;
    }
    
    /* Initialize the Amiga semaphore */
    InitSemaphore(&sem->sem_sem);
    sem->sem_sem.ss_NestCount = 0;  /* Reset nesting count */
    
    /* Initialize our own counter and condition variable */
    sem->count = value;
    NewList(&sem->waiters);
    
    return 0;
}

/*
 * Destroy a semaphore
 */
int sem_destroy(sem_t *sem)
{
    if (sem == NULL) {
        return -1;
    }
    
    /* Nothing to clean up */
    return 0;
}

/*
 * Wait on a semaphore
 * Blocks if count is zero, decrements count when available
 */
int sem_wait(sem_t *sem)
{
    struct Process *proc;
    struct Node *node;
    ULONG signal_mask;
    
    if (sem == NULL) {
        return -1;
    }
    
    /* Check for user abort */
    chkabort();
    
    /* Use the Amiga semaphore to protect our counter */
    ObtainSemaphore(&sem->sem_sem);
    
    /* Check if we can proceed immediately */
    if (sem->count > 0) {
        sem->count--;
        ReleaseSemaphore(&sem->sem_sem);
        return 0;
    }
    
    /* We need to block - add ourselves to the waiters list */
    proc = (struct Process *)FindTask(NULL);
    signal_mask = 1L << proc->pr_MsgPort.mp_SigBit;
    node = &proc->pr_MsgPort.mp_Node;
    AddTail(&sem->waiters, node);
    
    /* Release the semaphore and wait for a signal */
    ReleaseSemaphore(&sem->sem_sem);
    Wait(signal_mask);
    
    /* When we wake up, we need to re-acquire the semaphore and decrement count */
    ObtainSemaphore(&sem->sem_sem);
    sem->count--;
    ReleaseSemaphore(&sem->sem_sem);
    
    return 0;
}

/*
 * Try to wait on a semaphore
 */
int sem_trywait(sem_t *sem)
{
    if (sem == NULL) {
        return -1;
    }
    
    /* Check for user abort */
    chkabort();
    
    /* Try to obtain the semaphore without blocking */
    if (AttemptSemaphore(&sem->sem_sem)) {
        /* Check if we can proceed immediately */
        if (sem->count > 0) {
            sem->count--;
            ReleaseSemaphore(&sem->sem_sem);
            return 0;  /* Success */
        } else {
            ReleaseSemaphore(&sem->sem_sem);
            return -1;  /* Would block */
        }
    }
    
    return -1;  /* Would block */
}

/*
 * Post to a semaphore
 */
int sem_post(sem_t *sem)
{
    struct Node *node;
    
    if (sem == NULL) {
        return -1;
    }
    
    /* Check for user abort */
    chkabort();
    
    /* Use the Amiga semaphore to protect our counter */
    ObtainSemaphore(&sem->sem_sem);
    
    /* Increment the count */
    sem->count++;
    
    /* If there are waiting threads, wake up one of them */
    node = RemHead(&sem->waiters);
    if (node) {
        /* Calculate Process pointer from MsgPort node */
        struct Process *proc = (struct Process *)((char *)node - ((char *)&((struct Process *)0)->pr_MsgPort.mp_Node));
        Signal((struct Task *)proc, 1L << proc->pr_MsgPort.mp_SigBit);
    }
    
    ReleaseSemaphore(&sem->sem_sem);
    
    return 0;
}

/*
 * Get semaphore value
 * Note: This is not reliably implementable with Amiga semaphores
 * as the internal count is not accessible. We return -1 to indicate
 * this function is not supported.
 */
int sem_getvalue(sem_t *sem, int *sval)
{
    if (sem == NULL || sval == NULL) {
        return -1;
    }
    
    /* Check for user abort */
    chkabort();
    
    /* 
     * The Amiga semaphore's internal count is not accessible.
     * We could try to estimate it, but it's not reliable.
     * Return -1 to indicate this function is not supported.
     */
    *sval = -1;  /* Indicate unsupported */
    
    return 0;
}