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