/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * vfork implementation for Amiga using pthread infrastructure
 */

#include "include/unistd.h"
#include "include/amigapthread.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Forward declarations to avoid header conflicts */
typedef ULONG pthread_t;

/* pthread function declarations */
extern int pthread_create(pthread_t *thread, const void *attr, 
                         void *(*start_routine)(void *), void *arg);
extern int pthread_join(pthread_t thread, void **retval);
extern void pthread_exit(void *retval);

/* Forward declarations */
static void *vfork_thread_entry(void *arg);
static void vfork_cleanup_thread(void *arg);

/* vfork state structure */
typedef struct {
    pthread_t thread_id;
    pid_t child_pid;
    BOOL is_child;
    BOOL thread_created;
    struct Process *parent_process;
    struct Process *child_process;
    BPTR parent_input;
    BPTR parent_output;
    BPTR parent_error;
    BPTR child_input;
    BPTR child_output;
    BPTR child_error;
} VForkState;

/* Global state for vfork communication */
static VForkState *current_vfork_state = NULL;
static struct SignalSemaphore vfork_semaphore;

/* Initialize vfork system */
static void vfork_init(void)
{
    static BOOL initialized = FALSE;
    if (!initialized) {
        InitSemaphore(&vfork_semaphore);
        initialized = TRUE;
    }
}

/*
 * vfork - Create a child process using pthread infrastructure
 * 
 * This implementation uses pthread to create a child process that
 * simulates vfork() behavior on AmigaOS.
 */
pid_t vfork(void)
{
    VForkState *state;
    pthread_t thread;
    int result;
    struct Process *parent;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Initialize vfork system */
    vfork_init();
    
    /* Get current process */
    parent = (struct Process *)FindTask(NULL);
    if (parent == NULL) {
        errno = EAGAIN;
        return -1;
    }
    
    /* Allocate vfork state */
    state = (VForkState *)AllocMem(sizeof(VForkState), MEMF_CLEAR);
    if (state == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Initialize state */
    state->parent_process = parent;
    state->is_child = FALSE;
    state->thread_created = FALSE;
    
    /* Save current I/O handles */
    state->parent_input = Input();
    state->parent_output = Output();
    state->parent_error = ErrorOutput();
    
    /* Set up child I/O handles (inherit from parent) */
    state->child_input = state->parent_input;
    state->child_output = state->parent_output;
    state->child_error = state->parent_error;
    
    /* Create pthread that will become the child process */
    result = pthread_create(&thread, NULL, vfork_thread_entry, state);
    if (result != 0) {
        FreeMem(state, sizeof(VForkState));
        errno = result;
        return -1;
    }
    
    state->thread_id = thread;
    state->thread_created = TRUE;
    
    /* Wait for child thread to initialize */
    /* In a real vfork(), the child would return 0 here */
    /* The parent continues and returns the child's PID */
    
    /* Store state for cleanup */
    current_vfork_state = state;
    
    /* Return child process ID (using thread ID as PID) */
    return (pid_t)thread;
}

/*
 * vfork_thread_entry - Entry point for child thread/process
 * 
 * This function runs in the child thread and simulates vfork() behavior.
 */
static void *vfork_thread_entry(void *arg)
{
    VForkState *state = (VForkState *)arg;
    struct Process *child;
    
    /* Get current process (child) */
    child = (struct Process *)FindTask(NULL);
    if (child == NULL) {
        return (void *)-1;
    }
    
    /* Set up child process state */
    state->child_process = child;
    state->child_pid = (pid_t)((ULONG)child & 0xFFFF); /* Use process address as PID */
    state->is_child = TRUE;
    
    /* In a real vfork(), the child would:
     * 1. Share the parent's memory space
     * 2. Have the same file descriptors
     * 3. Share the same stack (temporarily)
     * 4. Return 0 to indicate it's the child
     * 
     * On Amiga, we simulate this by having the child thread
     * wait for the parent to call exec() or exit().
     */
    
    /* Child process waits for parent to call exec() */
    /* This simulates the vfork() behavior where the child
     * shares the parent's memory space until exec() is called
     */
    while (1) {
        /* Wait for parent to signal us to continue */
        Wait(1L << child->pr_MsgPort.mp_SigBit);
        
        /* Check if we should exit */
        if (state->thread_created == FALSE) {
            break;
        }
    }
    
    /* Child process exits */
    pthread_exit((void *)0);
    return (void *)0;
}

/*
 * vfork_cleanup_thread - Cleanup function for vfork state
 */
static void vfork_cleanup_thread(void *arg)
{
    VForkState *state = (VForkState *)arg;
    
    if (state == NULL) {
        return;
    }
    
    /* Clean up I/O handles if they were duplicated */
    if (state->child_input != state->parent_input && state->child_input != NULL) {
        Close(state->child_input);
    }
    if (state->child_output != state->parent_output && state->child_output != NULL) {
        Close(state->child_output);
    }
    if (state->child_error != state->parent_error && state->child_error != NULL) {
        Close(state->child_error);
    }
    
    /* Free state structure */
    FreeMem(state, sizeof(VForkState));
}

/*
 * vfork_wait - Wait for child process to complete
 * 
 * This function waits for the child process created by vfork() to complete.
 */
int vfork_wait(pid_t pid, int *status)
{
    VForkState *state;
    void *retval;
    int result;
    
    /* Check for abort signal */
    __chkabort();
    
    if (current_vfork_state == NULL) {
        errno = ECHILD;
        return -1;
    }
    
    state = current_vfork_state;
    
    /* Wait for child thread to complete */
    result = pthread_join(state->thread_id, &retval);
    if (result != 0) {
        errno = result;
        return -1;
    }
    
    /* Set exit status if provided */
    if (status != NULL) {
        *status = (int)(long)retval;
    }
    
    /* Clean up state */
    vfork_cleanup_thread(state);
    current_vfork_state = NULL;
    
    return 0;
}

/*
 * vfork_cleanup - Clean up vfork state
 * 
 * This function cleans up the vfork state and should be called
 * when the parent process is done with the child.
 */
void vfork_cleanup(void)
{
    if (current_vfork_state != NULL) {
        vfork_cleanup_thread(current_vfork_state);
        current_vfork_state = NULL;
    }
}