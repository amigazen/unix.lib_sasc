/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on sysvipc implementation by Peter Bengtsson
 *
 * System V Semaphore implementation
 */

#include "include/internal/ipc_internal.h"
#include "include/sys/ipc.h"
#include "debug.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <time.h>

/* Global semaphore context */
struct IPCIdKeyMap sem_keymap;
struct UndoInfo *undo_list = NULL;

/* Get undo information for semaphore operations */
struct UndoInfo *get_undo(int id, int create)
{
    struct UndoInfo *ui = undo_list;
    
    while (ui) {
        if (ui->Id == id) {
            break;
        }
        ui = ui->Next;
    }
    
    if (create && !ui) {
        struct semid_ds *si;
        si = (struct semid_ds *)get_ipc_by_id(&sem_keymap, id);
        if (si) {
            ui = AllocVec(sizeof(struct UndoInfo) + sizeof(int32_t) * si->sem_nsems, 
                                MEMF_CLEAR | MEMF_ANY);
            if (ui) {
                ui->Id = id;
                ui->Next = undo_list;
                undo_list = ui;
            }
        }
    }
    return ui;
}

/* Initialize semaphore context */
void sem_init(void)
{
    ipc_map_init(&sem_keymap);
}

/* Cleanup semaphore context */
void sem_cleanup(void)
{
    struct UndoInfo *ui, *next;
    
    ui = undo_list;
    while (ui) {
        next = ui->Next;
        FreeVec(ui);
        ui = next;
    }
    undo_list = NULL;
    
    ipc_map_uninit(&sem_keymap);
}

/* Construct a semaphore set */
struct semid_ds *sem_construct(int key, int flags)
{
    /* Removed unused TagItem array */
    struct semid_ds *si;
    
    si = AllocVec(sizeof(struct semid_ds), MEMF_ANY | MEMF_CLEAR);
    if (si) {
        si->sem_base = NULL;
        si->sem_perm.mode = flags & 0777;
        si->sem_perm.key = key;
        si->sem_nsems = 0;
        si->sem_binary = 0;
        si->sem_otime = 0;
        si->sem_ctime = time(NULL);
        si->Lock = AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC | MEMF_CLEAR);
        if (si->Lock) {
            InitSemaphore(si->Lock);
        }
    }
    return si;
}

/* Destroy a semaphore set */
void sem_destroy(struct semid_ds *si)
{
    struct sem_internal *sa;
    int i;
    
    if (si) {
        /* Wake everyone who is waiting on this semaphore array */
        sa = (struct sem_internal *)si->sem_base;
        for (i = 0; i < si->sem_nsems; i++) {
            wake_list(sa[i].zList);
            wake_list(sa[i].nList);
            FreeMem(sa[i].zList, sizeof(struct MinList));
            FreeMem(sa[i].nList, sizeof(struct MinList));
        }
        FreeMem(si->Lock, sizeof(struct SignalSemaphore));
        FreeVec(si->sem_base);
        FreeVec(si);
    }
}

/* Setup a semaphore */
void sem_setup(struct sem_internal *s)
{
    /* Removed unused TagItem array */
    
    s->semval = 1;
    s->sempid = 0;
    s->semncnt = 0;
    s->semzcnt = 0;
    s->zList = AllocMem(sizeof(struct MinList), MEMF_PUBLIC | MEMF_CLEAR);
    s->nList = AllocMem(sizeof(struct MinList), MEMF_PUBLIC | MEMF_CLEAR);
}

/* Get a semaphore set identifier */
int semget(key_t key, int nsems, int flags)
{
    int id = -1;
    int i;
    struct sem_internal *sa;
    struct semid_ds *si;
    
    ENTER();
    SHOWVALUE(key);
    SHOWVALUE(nsems);
    SHOWVALUE(flags);
    
    if (nsems <= 0 || nsems > 32) {  /* Arbitrary limit */
        set_ipc_errno(EINVAL);
        return -1;
    }
    
    ipc_lock(&sem_keymap);
    
    id = get_ipc_key_id(&sem_keymap, key, flags, 
                       (void *(*)(int, int))sem_construct);
    if (id >= 0) {
        si = (struct semid_ds *)get_ipc_by_id(&sem_keymap, id);
        if (si && !si->sem_base) {
            si->sem_base = AllocVec(sizeof(struct sem_internal) * nsems, 
                                          MEMF_ANY);
            if (si->sem_base) {
                sa = (struct sem_internal *)si->sem_base;
                for (i = 0; i < nsems; i++) {
                    sem_setup(&sa[i]);
                }
                si->sem_nsems = nsems;
            } else {
                ipc_rm_id(&sem_keymap, id, 
                         (void (*)(struct IPCGeneric *))sem_destroy);
                set_ipc_errno(ENOMEM);
                id = -1;
            }
        }
    } else {
        set_ipc_errno(-id);
        id = -1;
    }
    
    ipc_unlock(&sem_keymap);
    
    SHOWVALUE(id);
    return id;
}

/* Semaphore operations */
int semop(int semid, const struct sembuf *ops, int nops)
{
    struct semid_ds *si;
    int ret = -1;
    struct WProc *wp;
    int i, t;
    int zW = 0, nW = 0;
    struct sem_internal *sa;
    struct UndoInfo *ui = NULL;
    
    ENTER();
    SHOWVALUE(semid);
    SHOWPOINTER(ops);
    SHOWVALUE(nops);
    
    if (!ops || nops <= 0) {
        set_ipc_errno(EINVAL);
        return -1;
    }
    
    ipc_lock(&sem_keymap);
    
    si = (struct semid_ds *)get_ipc_by_id(&sem_keymap, semid);
    if (si) {
        ObtainSemaphore(si->Lock);
        
        sa = (struct sem_internal *)si->sem_base;
        
        /* Check if all operations can be performed */
        for (i = 0; i < nops; i++) {
            if (ops[i].sem_num >= si->sem_nsems) {
                set_ipc_errno(EFBIG);
                ReleaseSemaphore(si->Lock);
                ipc_unlock(&sem_keymap);
                return -1;
            }
            
            t = sa[ops[i].sem_num].semval + ops[i].sem_op;
            if (t < 0) {
                if (ops[i].sem_flg & IPC_NOWAIT) {
                    set_ipc_errno(EAGAIN);
                    ReleaseSemaphore(si->Lock);
                    ipc_unlock(&sem_keymap);
                    return -1;
                }
                nW++;
            }
            if (t == 0 && ops[i].sem_op < 0) {
                zW++;
            }
        }
        
        if (nW == 0 && zW == 0) {
            /* All operations can be performed immediately */
            for (i = 0; i < nops; i++) {
                sa[ops[i].sem_num].semval += ops[i].sem_op;
                sa[ops[i].sem_num].sempid = (int)((ULONG)FindTask(0));  /* Use Task pointer as PID */
                
                if (ops[i].sem_flg & SEM_UNDO) {
                    ui = get_undo(semid, 1);
                    if (ui) {
                        ui->Adj[ops[i].sem_num] -= ops[i].sem_op;
                    }
                }
            }
            si->sem_otime = time(NULL);
            ret = 0;
        } else {
            /* Need to wait */
            if (ops[0].sem_flg & IPC_NOWAIT) {
                set_ipc_errno(EAGAIN);
            } else {
                /* Wait for conditions to be met */
                wp = request_wakeup((struct MinList *)sa[ops[0].sem_num].nList);
                ReleaseSemaphore(si->Lock);
                ipc_unlock(&sem_keymap);
                wait_list(wp);
                return semop(semid, ops, nops);  /* Retry */
            }
        }
        
        ReleaseSemaphore(si->Lock);
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&sem_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Timed semaphore operations */
int semtimedop(int semid, const struct sembuf *ops, int nops, const struct timespec *timeout)
{
    /* For now, just call semop - timeout handling would require more complex implementation */
    return semop(semid, ops, nops);
}

/* Control semaphore set */
int semctl(int semid, int semnum, int cmd, union semun arg)
{
    int ret = -1;
    struct semid_ds *si;
    struct sem_internal *sb;
    int i;
    
    ENTER();
    SHOWVALUE(semid);
    SHOWVALUE(semnum);
    SHOWVALUE(cmd);
    
    ipc_lock(&sem_keymap);
    
    si = (struct semid_ds *)get_ipc_by_id(&sem_keymap, semid);
    if (si) {
        ObtainSemaphore(si->Lock);
        
        sb = (struct sem_internal *)si->sem_base;
        
        switch (cmd) {
        case IPC_STAT:
            if (arg.buf) {
                memcpy(arg.buf, si, sizeof(struct semid_ds));
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        case IPC_SET:
            if (arg.buf) {
                si->sem_perm.uid = arg.buf->sem_perm.uid;
                si->sem_perm.gid = arg.buf->sem_perm.gid;
                si->sem_perm.mode = arg.buf->sem_perm.mode;
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        case IPC_RMID:
            ipc_rm_id(&sem_keymap, semid, 
                     (void (*)(struct IPCGeneric *))sem_destroy);
            ret = 0;
            break;
            
        case GETVAL:
            if (semnum >= 0 && semnum < si->sem_nsems) {
                ret = sb[semnum].semval;
            } else {
                set_ipc_errno(EINVAL);
            }
            break;
            
        case SETVAL:
            if (semnum >= 0 && semnum < si->sem_nsems) {
                sb[semnum].semval = arg.val;
                sb[semnum].sempid = (int)((ULONG)FindTask(0));  /* Use Task pointer as PID */
                ret = 0;
            } else {
                set_ipc_errno(EINVAL);
            }
            break;
            
        case GETALL:
            if (arg.array) {
                for (i = 0; i < si->sem_nsems; i++) {
                    arg.array[i] = sb[i].semval;
                }
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        case SETALL:
            if (arg.array) {
                for (i = 0; i < si->sem_nsems; i++) {
                    sb[i].semval = arg.array[i];
                    sb[i].sempid = (int)((ULONG)FindTask(0));  /* Use Task pointer as PID */
                }
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        default:
            set_ipc_errno(EINVAL);
            break;
        }
        
        ReleaseSemaphore(si->Lock);
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&sem_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Get semaphore IDs */
int semids(int *buf, unsigned int nids, unsigned int *idcnt)
{
    int ret;
    
    ENTER();
    SHOWPOINTER(buf);
    SHOWVALUE(nids);
    SHOWPOINTER(idcnt);
    
    ret = ipc_ids(&sem_keymap, buf, nids, (int *)idcnt);
    
    SHOWVALUE(ret);
    return ret;
}
