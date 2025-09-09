/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on sysvipc implementation by Peter Bengtsson
 *
 * Core IPC management functions
 */

#include "include/internal/ipc_internal.h"
#include "debug.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>

/* Global IPC contexts */
struct IPCIdKeyMap shm_keymap;
struct IPCIdKeyMap msg_keymap;
struct IPCIdKeyMap sem_keymap;
struct UndoInfo *undo_list = NULL;

/* Initialize IPC map */
void ipc_map_init(struct IPCIdKeyMap *m)
{
    if (!m->Lock) {
        m->nobj = 0;
        m->vlen = 0;
        m->idx = 0;
        m->nused = 0;
        m->objv = NULL;
        m->Lock = AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC | MEMF_CLEAR);
        if (m->Lock) {
            InitSemaphore(m->Lock);
        }
    }
}

/* Cleanup IPC map */
void ipc_map_uninit(struct IPCIdKeyMap *m)
{
    if (m->Lock) {
        FreeMem(m->Lock, sizeof(struct SignalSemaphore));
        m->Lock = NULL;
    }
    if (m->objv) {
        FreeVec(m->objv);
        m->objv = NULL;
    }
}

/* Get IPC key ID */
int get_ipc_key_id(struct IPCIdKeyMap *m, key_t key, int flags, 
                   void *(*construct)(int, int))
{
    int id = -1, newidx;
    int i;
    struct IPCGeneric *o;
    
    newidx = m->nobj;
    
    if (key != IPC_PRIVATE) {
        for (i = 0; i < m->nobj; i++) {
            if (m->objv[i] != NULL && m->objv[i]->perm.key == key) {
                id = i;  /* Object already exists */
                break;
            } else if (m->objv[i] == NULL && newidx == m->nobj) {
                newidx = i;  /* First free slot */
            }
        }
    } else {
        flags |= IPC_CREAT;
    }
    
    if (flags & IPC_CREAT) {  /* Will attempt to create resource */
        if (id >= 0 && (flags & IPC_EXCL)) {  /* Exclusive resource requested, but it already exists */
            id = -EEXIST;
        } else if (id == -1) {  /* Create a new resource */
            o = (struct IPCGeneric *)construct(key, flags);
            if (o) {
                /* Use default values for UID/GID since Task doesn't have these fields */
                o->perm.uid = 0;  /* Default user ID */
                o->perm.gid = 0;  /* Default group ID */
                o->perm.seq = m->idx++;
                
                if (m->nobj == m->vlen) {  /* Add more free slots if needed */
                    struct IPCGeneric **tmp;
                    tmp = AllocVec(sizeof(struct IPCGeneric *) * (m->vlen + VEXTEND), 
                                        MEMF_ANY);
                    if (tmp) {
                        if (m->objv) {
                            CopyMem(m->objv, tmp, sizeof(struct IPCGeneric *) * m->vlen);
                            FreeVec(m->objv);
                        }
                        m->objv = tmp;
                        m->vlen += VEXTEND;
                    } else {
                        FreeVec(o);
                        id = -ENOMEM;
                        return id;
                    }
                }
                
                m->objv[newidx] = o;
                if (newidx == m->nobj) {
                    m->nobj++;
                }
                id = newidx;
                m->nused++;
            } else {
                id = -ENOMEM;
            }
        }
    } else if (id == -1) {
        id = -ENOENT;
    }
    
    return id;
}

/* Get IPC object by ID */
void *get_ipc_by_id(struct IPCIdKeyMap *m, int id)
{
    if (id >= 0 && id < m->nobj && m->objv[id]) {
        return m->objv[id];
    }
    return NULL;
}

/* Lock IPC map */
void ipc_lock(struct IPCIdKeyMap *m)
{
    ObtainSemaphore(m->Lock);
}

/* Unlock IPC map */
void ipc_unlock(struct IPCIdKeyMap *m)
{
    ReleaseSemaphore(m->Lock);
}

/* Get IPC object by predicate */
void *get_ipc_by_p(struct IPCIdKeyMap *m, int (*eq)(void *, void *), 
                   void *v, int *id)
{
    int i;
    
    for (i = 0; i < m->nobj; i++) {
        if (m->objv[i] && eq(m->objv[i], v)) {
            if (id) *id = i;
            return m->objv[i];
        }
    }
    return NULL;
}

/* Remove IPC object by ID */
void ipc_rm_id(struct IPCIdKeyMap *m, int id, 
               void (*destroy)(struct IPCGeneric *))
{
    if (id >= 0 && id < m->nobj && m->objv[id]) {
        destroy(m->objv[id]);
        m->objv[id] = NULL;
        m->nused--;
    }
}

/* Get IPC IDs */
int ipc_ids(struct IPCIdKeyMap *m, int *idbuf, int idblen, int *idcnt)
{
    int i, cnt = 0;
    
    if (!idbuf || !idcnt) {
        return -1;
    }
    
    for (i = 0; i < m->nobj && cnt < idblen; i++) {
        if (m->objv[i]) {
            idbuf[cnt++] = i;
        }
    }
    
    *idcnt = cnt;
    return 0;
}

/* Wake up processes in a list */
void wake_list(struct MinList *ml)
{
    struct WProc *wp;
    struct Task *self;
    uint32_t pri;
    
    wp = (struct WProc *)RemHead((struct List *)ml);
    while (wp) {
        self = FindTask(0);
        pri = self->tc_Node.ln_Pri;
        
        SetTaskPri(wp->T, pri);
        Signal(wp->T, SIGF_SINGLE);
        
        wp = (struct WProc *)RemHead((struct List *)ml);
    }
}

/* Request wakeup for a process */
struct WProc *request_wakeup(struct MinList *ml)
{
    struct WProc *wp;
    
    wp = AllocMem(sizeof(struct WProc), MEMF_PUBLIC | MEMF_CLEAR);
    if (wp) {
        wp->T = FindTask(0);
        AddTail((struct List *)ml, (struct Node *)wp);
    }
    return wp;
}

/* Wait for wakeup */
void wait_list(struct WProc *wp)
{
    if (wp) {
        Wait(SIGF_SINGLE);
        FreeVec(wp);
    }
}

/* Cancel wakeup request */
void cancel_wakeup(struct MinList *ml)
{
    struct WProc *wp;
    
    wp = (struct WProc *)RemTail((struct List *)ml);
    if (wp) {
        FreeVec(wp);
    }
}

/* Set IPC error number */
void set_ipc_errno(int error)
{
    errno = error;
}

/* Initialize all IPC systems */
void ipc_init_all(void)
{
    shm_init();
    msg_init();
    sem_init();
}

/* Cleanup all IPC systems */
void ipc_cleanup_all(void)
{
    shm_cleanup();
    msg_cleanup();
    sem_cleanup();
}
