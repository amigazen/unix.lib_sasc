/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 * 
 * Private header for pthread implementation
 */

#ifndef _PTHREAD_PRIV_H
#define _PTHREAD_PRIV_H

#include "amigapthread.h"
#include "pthread.h"

/* Cancellation state machine */
typedef enum
{
    PTHREAD_STATE_RUNNING = 0,
    PTHREAD_STATE_CANCEL_REQUESTED = 1,
    PTHREAD_STATE_CANCELED = 2
} pthread_cancel_state_t;

/* Thread pair structure */
typedef struct ThreadPair {
    struct Node tp_Node;
    pthread_t tp_ThreadId;
    struct Process *tp_Parent;
    LONG tp_ParentSignal;
    struct Process *tp_Child;
    LONG tp_ChildSignal;
    void *(*tp_StartRoutine)(void *);
    void *tp_Arg;
    void *tp_Result;
    BOOL tp_Finished;
    BOOL tp_Detached;
    /* Cancellation support */
    int tp_CancelState;
    int tp_CancelType;
    pthread_cancel_state_t tp_CancelStateMachine;
    struct SignalSemaphore tp_CancelSem;
    BOOL tp_CancelInitialized;
} ThreadPair;

/* Function declarations */
struct ThreadPair *FindThreadPair(pthread_t thread_id);
void __saveds ThreadEntry(void);
int pthread_library_init(void);
void pthread_library_cleanup(void);

/* Assembly function */
extern void __saveds thread_entry(void);

/* Global variables - declared in pthread.c */

#endif /* _PTHREAD_PRIV_H */