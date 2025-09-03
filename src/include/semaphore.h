/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * POSIX semaphore implementation for Amiga
 */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>

/* Semaphore structure */
typedef struct {
    struct SignalSemaphore sem_sem;
    int count;              /* Our own counter */
    struct List waiters;    /* List of waiting processes */
} sem_t;

/* Semaphore functions */
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);

#endif /* _SEMAPHORE_H */
