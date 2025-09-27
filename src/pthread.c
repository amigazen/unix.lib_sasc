/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * pthread implementation for Amiga
 */

#include "include/amigapthread.h"
#include "pthread_priv.h"

/* Forward declarations for cancellation functions */
static void pthread_cooperative_cancel_check(struct ThreadPair *tp);
static void pthread_cooperative_cleanup(struct ThreadPair *tp);

/* External function declarations */
extern void __regargs __chkabort(void);

static LONG CustomASyncRun(STRPTR name, STRPTR cmd, struct ProcessControlBlock *pcb)
{
    struct Process *process;
    LONG result = -1;
    /* Use CreateNewProcTags with variadic arguments */
    process = CreateNewProcTags(
        NP_Name, (ULONG)name,
        NP_Entry, (ULONG)pcb->pcb_Entry,
        NP_StackSize, pcb->pcb_StackSize,
        NP_Priority, pcb->pcb_Pri,
        TAG_END
    );
    
    if (process) {
        /* Store the process pointer in the PCB for later use */
        pcb->pcb_WBProcess = process;
        printf("DEBUG: CreateNewProc created process at %p\n", process);
        result = 0;  /* Success */
    } else {
        printf("DEBUG: CreateNewProc failed to create process\n");
    }
    
    /* Check for user abort */
    chkabort();
    
    return result;
}

/*
 * Cooperative cancellation check - called by threads to check for cancellation
 */
static void pthread_cooperative_cancel_check(struct ThreadPair *tp)
{
    if (!tp || !tp->tp_CancelInitialized)
        return;
        
    ObtainSemaphore(&tp->tp_CancelSem);
    
    if (tp->tp_CancelStateMachine == PTHREAD_STATE_CANCEL_REQUESTED)
    {
        tp->tp_CancelStateMachine = PTHREAD_STATE_CANCELED;
        ReleaseSemaphore(&tp->tp_CancelSem);
        
        /* Thread should exit now */
        pthread_exit(PTHREAD_CANCELED);
    }
    
    ReleaseSemaphore(&tp->tp_CancelSem);
}

/*
 * Cooperative cleanup - called when thread starts
 */
static void pthread_cooperative_cleanup(struct ThreadPair *tp)
{
    if (!tp)
        return;
        
    if (tp->tp_CancelInitialized)
    {
        ObtainSemaphore(&tp->tp_CancelSem);
        tp->tp_CancelStateMachine = PTHREAD_STATE_RUNNING;
        ReleaseSemaphore(&tp->tp_CancelSem);
    }
}

/* Global thread management */
struct List *ThreadList = NULL;
struct SignalSemaphore ThreadListSem;

/* Thread counter */
static pthread_t NextThreadId = 1;

/* Thread-specific storage key management */
typedef struct {
    void (*destructor)(void *);
    BOOL used;
} TLSKey;

static TLSKey tls_keys[PTHREAD_KEYS_MAX];
static struct SignalSemaphore tls_semaphore;

/*
 * Library constructor - initialize thread management
 */
int pthread_library_init(void)
{
    if (ThreadList == NULL) {
        ThreadList = (struct List *)AllocMem(sizeof(struct List), MEMF_CLEAR);
        if (ThreadList == NULL) {
            return -1;
        }
        NewList(ThreadList);
        InitSemaphore(&ThreadListSem);
        
        /* Initialize TLS system */
        InitSemaphore(&tls_semaphore);
        memset(tls_keys, 0, sizeof(tls_keys));
    }
    return 0;
}

/*
 * Library destructor - cleanup thread management
 */
void pthread_library_cleanup(void)
{
    if (ThreadList) {
        FreeMem(ThreadList, sizeof(struct List));
        ThreadList = NULL;
    }
}

/*
 * Find a thread pair by thread ID
 */
struct ThreadPair *FindThreadPair(pthread_t thread_id)
{
    struct ThreadPair *tp;
    
    Forbid();
    tp = (struct ThreadPair *)ThreadList->lh_Head;
    while (tp->tp_Node.ln_Succ) {
        if (tp->tp_ThreadId == thread_id) {
            Permit();
            return tp;
        }
        tp = (struct ThreadPair *)tp->tp_Node.ln_Succ;
    }
    Permit();
    return NULL;
}

/*
 * Thread entry point - called by assembly trampoline
 */
void __saveds ThreadEntry(void)
{
    struct ThreadPair *tp;
    struct Process *proc;
    
    proc = (struct Process *)FindTask(NULL);
    
    /* Wait for start signal */
    Wait(1L << proc->pr_MsgPort.mp_SigBit);
    
    Forbid();
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    Permit();
    
    if (tp) {
        /* Initialize cooperative cancellation */
        pthread_cooperative_cleanup(tp);
        
        /* Check for user abort before starting */
        chkabort();
        
        /* Call the user's thread function */
        tp->tp_Result = tp->tp_StartRoutine(tp->tp_Arg);
        
        /* Mark thread as finished */
        Forbid();
        tp->tp_Finished = TRUE;
        if (!tp->tp_Detached) {
            Signal((struct Task *)tp->tp_Parent, 1L << tp->tp_ParentSignal);
        }
        Permit();
    }
}

/*
 * Create a new thread
 */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, 
                   void *(*start_routine)(void *), void *arg)
{
    struct ThreadPair *tp;
    struct ProcessControlBlock PCB;
    char thread_name[32];
    
    /* Check for user abort */
    chkabort();
    
    /* Initialize library if needed */
    if (ThreadList == NULL) {
        if (pthread_library_init() != 0) {
            return EAGAIN;
        }
    }
    
    /* Allocate thread pair structure */
    tp = (struct ThreadPair *)AllocMem(sizeof(struct ThreadPair), MEMF_CLEAR);
    if (tp == NULL) {
        return EAGAIN;
    }
    
    /* Initialize thread pair */
    tp->tp_Node.ln_Type = 0;
    tp->tp_Node.ln_Pri = 0;
    tp->tp_ThreadId = NextThreadId++;
    tp->tp_Parent = (struct Process *)FindTask(NULL);
    tp->tp_StartRoutine = start_routine;
    tp->tp_Arg = arg;
    tp->tp_Result = NULL;
    tp->tp_Finished = FALSE;
    tp->tp_Detached = FALSE;
    
    /* Initialize cancellation support */
    tp->tp_CancelState = PTHREAD_CANCEL_ENABLE;
    tp->tp_CancelType = PTHREAD_CANCEL_DEFERRED;
    tp->tp_CancelStateMachine = PTHREAD_STATE_RUNNING;
    InitSemaphore(&tp->tp_CancelSem);
    tp->tp_CancelInitialized = TRUE;
    
    /* Allocate parent signal */
    tp->tp_ParentSignal = AllocSignal(-1);
    if (tp->tp_ParentSignal == -1) {
        FreeMem(tp, sizeof(struct ThreadPair));
        return EAGAIN;
    }
    
    /* Set up ProcessControlBlock */
    PCB.pcb_StackSize = 8192L;
    PCB.pcb_Pri = 0;
    PCB.pcb_Flags = PRF_CODE | PRF_NOCLI | PRF_SAVEIO;
    PCB.pcb_Reserved1 = 0;
    PCB.pcb_Reserved2 = 0;
    PCB.pcb_Reserved3 = 0;
    PCB.pcb_Reserved4 = 0;
    PCB.pcb_Entry = (APTR)thread_entry;
    PCB.pcb_Reserved5 = 0;
    PCB.pcb_WBProcess = NULL;
    
    ObtainSemaphore(&ThreadListSem);
    
    /* Create thread name */
    sprintf(thread_name, "pthread_%ld", tp->tp_ThreadId);
    
    /* Start the process */
    printf("DEBUG: About to call CustomASyncRun for thread %ld\n", tp->tp_ThreadId);
    if (CustomASyncRun(thread_name, NOCMD, &PCB) >= 0L) {
        printf("DEBUG: CustomASyncRun succeeded for thread %ld\n", tp->tp_ThreadId);
        tp->tp_Child = (struct Process *)PCB.pcb_WBProcess;
        tp->tp_Child->pr_Task.tc_UserData = tp;
        AddHead(ThreadList, &tp->tp_Node);
        
        /* Signal the child to start */
        printf("DEBUG: Signaling child process for thread %ld, signal bit %ld\n", tp->tp_ThreadId, tp->tp_Child->pr_MsgPort.mp_SigBit);
        printf("DEBUG: Child process pointer: %p\n", tp->tp_Child);
        printf("DEBUG: Child task pointer: %p\n", (struct Task *)tp->tp_Child);
        Signal((struct Task *)tp->tp_Child, 1L << tp->tp_Child->pr_MsgPort.mp_SigBit);
        printf("DEBUG: Signal sent to thread %ld\n", tp->tp_ThreadId);
        
        *thread = tp->tp_ThreadId;
        ReleaseSemaphore(&ThreadListSem);
        printf("DEBUG: Thread %ld created successfully\n", tp->tp_ThreadId);
        
        /* Check for user abort */
        chkabort();
        return 0;
    } else {
        printf("DEBUG: CustomASyncRun failed for thread %ld\n", tp->tp_ThreadId);
    }
    
    /* Error cleanup */
    FreeSignal(tp->tp_ParentSignal);
    FreeMem(tp, sizeof(struct ThreadPair));
    ReleaseSemaphore(&ThreadListSem);
    return EAGAIN;
}

/*
 * Wait for thread to complete
 */
int pthread_join(pthread_t thread, void **retval)
{
    struct ThreadPair *tp;
    
    /* Check for user abort */
    chkabort();
    
    tp = FindThreadPair(thread);
    if (tp == NULL) {
        return ESRCH;
    }
    
    if (tp->tp_Detached) {
        return EINVAL;
    }
    
    /* Wait for thread to finish */
    Wait(1L << tp->tp_ParentSignal);
    
    /* Check for user abort after waiting */
    chkabort();
    
    if (retval) {
        *retval = tp->tp_Result;
    }
    
    /* Clean up thread pair */
    Forbid();
    Remove(&tp->tp_Node);
    FreeSignal(tp->tp_ParentSignal);
    FreeMem(tp, sizeof(struct ThreadPair));
    Permit();
    
    return 0;
}

/*
 * Detach a thread
 */
int pthread_detach(pthread_t thread)
{
    struct ThreadPair *tp;
    
    /* Check for user abort */
    chkabort();
    
    tp = FindThreadPair(thread);
    if (tp == NULL) {
        return ESRCH;
    }
    
    if (tp->tp_Detached) {
        return EINVAL;
    }
    
    tp->tp_Detached = TRUE;
    
    /* If thread is already finished, clean it up */
    if (tp->tp_Finished) {
        Forbid();
        Remove(&tp->tp_Node);
        FreeSignal(tp->tp_ParentSignal);
        FreeMem(tp, sizeof(struct ThreadPair));
        Permit();
    }
    
    return 0;
}

/*
 * Get current thread ID
 */
pthread_t pthread_self(void)
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct ThreadPair *tp;
    
    /* Check for user abort */
    chkabort();
    
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    if (tp) {
        return tp->tp_ThreadId;
    }
    
    return 0; /* Main thread */
}

/*
 * Exit current thread
 */
void pthread_exit(void *retval)
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct ThreadPair *tp;
    
    /* Check for user abort before exiting */
    chkabort();
    
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    if (tp) {
        tp->tp_Result = retval;
        tp->tp_Finished = TRUE;
        
        if (!tp->tp_Detached) {
            Signal((struct Task *)tp->tp_Parent, 1L << tp->tp_ParentSignal);
        }
    }
    
    /* Exit the process */
    Exit(0);
}

/*
 * Cancel a thread using cooperative cancellation
 */
int pthread_cancel(pthread_t thread)
{
    struct ThreadPair *tp;
    
    tp = FindThreadPair(thread);
    if (tp == NULL) {
        return ESRCH;
    }
    
    if (tp->tp_CancelInitialized) {
        ObtainSemaphore(&tp->tp_CancelSem);
        if (tp->tp_CancelStateMachine == PTHREAD_STATE_RUNNING) {
            tp->tp_CancelStateMachine = PTHREAD_STATE_CANCEL_REQUESTED;
        }
        ReleaseSemaphore(&tp->tp_CancelSem);
    }
    
    return 0;
}

/*
 * Test if thread is cancelled - cooperative cancellation check
 */
void pthread_testcancel(void)
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct ThreadPair *tp;
    
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    if (tp && tp->tp_CancelState == PTHREAD_CANCEL_ENABLE) {
        pthread_cooperative_cancel_check(tp);
    }
}

/*
 * Set thread cancellation state
 */
int pthread_setcancelstate(int state, int *oldstate)
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct ThreadPair *tp;
    
    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE) {
        return EINVAL;
    }
    
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    if (tp == NULL) {
        return EINVAL;
    }
    
    if (oldstate) {
        *oldstate = tp->tp_CancelState;
    }
    tp->tp_CancelState = state;
    
    return 0;
}

/*
 * Set thread cancellation type
 */
int pthread_setcanceltype(int type, int *oldtype)
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct ThreadPair *tp;
    
    if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS) {
        return EINVAL;
    }
    
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    if (tp == NULL) {
        return EINVAL;
    }
    
    if (oldtype) {
        *oldtype = tp->tp_CancelType;
    }
    tp->tp_CancelType = type;
    
    return 0;
}

/*
 * Initialize thread attributes
 */
int pthread_attr_init(pthread_attr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    attr->stacksize = 0; /* Use default */
    attr->stackaddr = NULL;
    attr->scope = PTHREAD_SCOPE_SYSTEM;
    attr->inheritsched = PTHREAD_INHERIT_SCHED;
    attr->schedpolicy = SCHED_OTHER;
    attr->schedparam.sched_priority = 0;
    
    return 0;
}

/*
 * Destroy thread attributes
 */
int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    /* Nothing to clean up */
    return 0;
}

/*
 * Get detach state attribute
 */
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
    if (attr == NULL || detachstate == NULL) {
        return EINVAL;
    }
    
    *detachstate = attr->detachstate;
    return 0;
}

/*
 * Set detach state attribute
 */
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    if (detachstate != PTHREAD_CREATE_JOINABLE && 
        detachstate != PTHREAD_CREATE_DETACHED) {
        return EINVAL;
    }
    
    attr->detachstate = detachstate;
    return 0;
}

/*
 * Get stack size attribute
 */
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    if (attr == NULL || stacksize == NULL) {
        return EINVAL;
    }
    
    *stacksize = attr->stacksize;
    return 0;
}

/*
 * Set stack size attribute
 */
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    if (stacksize < PTHREAD_STACK_MIN) {
        return EINVAL;
    }
    
    attr->stacksize = stacksize;
    return 0;
}

/*
 * Get stack address attribute
 */
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
    if (attr == NULL || stackaddr == NULL) {
        return EINVAL;
    }
    
    *stackaddr = attr->stackaddr;
    return 0;
}

/*
 * Set stack address attribute
 */
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    attr->stackaddr = stackaddr;
    return 0;
}

/*
 * Get scope attribute
 */
int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
    if (attr == NULL || scope == NULL) {
        return EINVAL;
    }
    
    *scope = attr->scope;
    return 0;
}

/*
 * Set scope attribute
 */
int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    if (scope != PTHREAD_SCOPE_SYSTEM && scope != PTHREAD_SCOPE_PROCESS) {
        return EINVAL;
    }
    
    attr->scope = scope;
    return 0;
}

/*
 * Get inherit scheduler attribute
 */
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    if (attr == NULL || inheritsched == NULL) {
        return EINVAL;
    }
    
    *inheritsched = attr->inheritsched;
    return 0;
}

/*
 * Set inherit scheduler attribute
 */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    if (inheritsched != PTHREAD_INHERIT_SCHED && 
        inheritsched != PTHREAD_EXPLICIT_SCHED) {
        return EINVAL;
    }
    
    attr->inheritsched = inheritsched;
    return 0;
}

/*
 * Get scheduling policy attribute
 */
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
    if (attr == NULL || policy == NULL) {
        return EINVAL;
    }
    
    *policy = attr->schedpolicy;
    return 0;
}

/*
 * Set scheduling policy attribute
 */
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    if (attr == NULL) {
        return EINVAL;
    }
    
    if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR) {
        return EINVAL;
    }
    
    attr->schedpolicy = policy;
    return 0;
}

/*
 * Get scheduling parameters attribute
 */
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
    if (attr == NULL || param == NULL) {
        return EINVAL;
    }
    
    *param = attr->schedparam;
    return 0;
}

/*
 * Set scheduling parameters attribute
 */
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
    if (attr == NULL || param == NULL) {
        return EINVAL;
    }
    
    attr->schedparam = *param;
    return 0;
}

/*
 * Thread-specific storage functions
 */
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    int i;
    
    if (key == NULL) {
        return EINVAL;
    }
    
    ObtainSemaphore(&tls_semaphore);
    
    for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if (!tls_keys[i].used) {
            tls_keys[i].used = TRUE;
            tls_keys[i].destructor = destructor;
            *key = i;
            ReleaseSemaphore(&tls_semaphore);
            return 0;
        }
    }
    
    ReleaseSemaphore(&tls_semaphore);
    return EAGAIN;
}

int pthread_key_delete(pthread_key_t key)
{
    if (key >= PTHREAD_KEYS_MAX) {
        return EINVAL;
    }
    
    ObtainSemaphore(&tls_semaphore);
    
    if (!tls_keys[key].used) {
        ReleaseSemaphore(&tls_semaphore);
        return EINVAL;
    }
    
    tls_keys[key].used = FALSE;
    tls_keys[key].destructor = NULL;
    
    ReleaseSemaphore(&tls_semaphore);
    return 0;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct ThreadPair *tp;
    
    if (key >= PTHREAD_KEYS_MAX) {
        return EINVAL;
    }
    
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    if (tp == NULL) {
        return EINVAL;
    }
    
    tp->tp_TLSValues[key] = (void *)value;
    return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct ThreadPair *tp;
    
    if (key >= PTHREAD_KEYS_MAX) {
        return NULL;
    }
    
    tp = (struct ThreadPair *)proc->pr_Task.tc_UserData;
    if (tp == NULL) {
        return NULL;
    }
    
    return tp->tp_TLSValues[key];
}

/*
 * Once control function
 */
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    if (once_control == NULL || init_routine == NULL) {
        return EINVAL;
    }
    
    if (once_control->done) {
        return 0;
    }
    
    ObtainSemaphore(&once_control->semaphore);
    
    if (!once_control->done) {
        init_routine();
        once_control->done = 1;
    }
    
    ReleaseSemaphore(&once_control->semaphore);
    return 0;
}

/*
 * Library constructor
 */
void __saveds pthread_constructor(void)
{
    pthread_library_init();
}

/*
 * Library destructor
 */
void __saveds pthread_destructor(void)
{
    pthread_library_cleanup();
}

/*
 * pthread-aware chkabort() override
 * This overrides sc.lib's chkabort() when pthread.lib is linked first
 * 
 * Call chain: chkabort() -> pthread_testcancel() -> __chkabort()
 */
void chkabort(void)
{
    /* Check for pthread cancellation first */
    pthread_testcancel();
    
    /* Then call the internal break checking function */
    __chkabort();
}