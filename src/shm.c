/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on sysvipc implementation by Peter Bengtsson
 *
 * System V Shared Memory implementation
 */

#include "include/internal/ipc_internal.h"
#include "include/sys/ipc.h"
#include "debug.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <time.h>

/* Global shared memory context */
struct IPCIdKeyMap shm_keymap;
static uint32_t total_shm = 0;
static uint32_t shm_max = DEF_SHMMAX;

#define SHMFLG_DeleteMe	(1L<<0)

/* Initialize shared memory context */
void shm_init(void)
{
    ipc_map_init(&shm_keymap);
}

/* Cleanup shared memory context */
void shm_cleanup(void)
{
    ipc_map_uninit(&shm_keymap);
}

/* Construct a shared memory segment */
struct shmid_ds *shm_construct(int key, int flags)
{
    struct shmid_ds *si;
    
    si = AllocVec(sizeof(struct shmid_ds), MEMF_ANY | MEMF_CLEAR);
    if (si) {
        si->shm_amp = NULL;
        si->shm_perm.mode = (flags & 0777) | 0x1000;  /* SHM_CLEAR equivalent */
        si->shm_perm.key = key;
        si->shm_segsz = 0;
        si->shm_nattach = 0;
        si->shm_atime = 0;
        si->shm_dtime = 0;
        si->shm_ctime = time(NULL);
    }
    return si;
}

/* Destroy a shared memory segment */
void shm_destroy(struct shmid_ds *si)
{
    if (si) {
        if (si->shm_amp) {
            FreeVec(si->shm_amp);
        }
        FreeVec(si);
    }
}

/* Get a shared memory identifier */
int shmget(key_t key, size_t size, int flags)
{
    int id = -1;
    struct shmid_ds *si;
    
    ENTER();
    SHOWVALUE(key);
    SHOWVALUE(size);
    SHOWVALUE(flags);
    
    ipc_lock(&shm_keymap);
    
    if ((size + total_shm) < shm_max) {
        id = get_ipc_key_id(&shm_keymap, key, flags, 
                           (void *(*)(int, int))shm_construct);
        if (id >= 0) {
            si = (struct shmid_ds *)get_ipc_by_id(&shm_keymap, id);
            if (si && !si->shm_amp) {
                si->shm_amp = AllocVec(size, MEMF_ANY);
                if (si->shm_amp) {
                    total_shm += size;
                    si->shm_segsz = size;
                } else {
                    ipc_rm_id(&shm_keymap, id, 
                             (void (*)(struct IPCGeneric *))shm_destroy);
                    set_ipc_errno(ENOMEM);
                    id = -1;
                }
            }
        } else {
            set_ipc_errno(-id);
            id = -1;
        }
    } else {
        set_ipc_errno(ENOMEM);
    }
    
    ipc_unlock(&shm_keymap);
    
    SHOWVALUE(id);
    return id;
}

/* Attach to a shared memory segment */
void *shmat(int shmid, const void *prefaddr, int flags)
{
    int i;
    void *addr = (void *)-1L;
    struct shmid_ds *si;
    
    ENTER();
    SHOWVALUE(shmid);
    SHOWPOINTER(prefaddr);
    SHOWVALUE(flags);
    
    ipc_lock(&shm_keymap);
    
    si = (struct shmid_ds *)get_ipc_by_id(&shm_keymap, shmid);
    if (si) {
        if (prefaddr == NULL || prefaddr == si->shm_amp) {
            si->shm_nattach++;
            addr = si->shm_amp;
            if (si->shm_perm.mode & 0x1000) {  /* SHM_CLEAR equivalent */
                /* Clear memory first time it is attached */
                for (i = 0; i < (si->shm_segsz >> 2); i++) {
                    ((uint32_t *)addr)[i] = 0;
                }
                for (i <<= 2; i < si->shm_segsz; i++) {
                    ((uint8_t *)addr)[i] = 0;
                }
                si->shm_perm.mode &= ~0x1000;  /* Clear SHM_CLEAR flag */
            }
        } else {
            set_ipc_errno(EINVAL);
        }
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&shm_keymap);
    
    SHOWPOINTER(addr);
    return addr;
}

/* Detach from a shared memory segment */
int shmdt(void *shmaddr)
{
    int id;
    int ret = -1;
    struct shmid_ds *si;
    
    ENTER();
    SHOWPOINTER(shmaddr);
    
    if (shmaddr == (void *)-1L) {
        set_ipc_errno(EINVAL);
        return -1;
    }
    
    ipc_lock(&shm_keymap);
    
    /* Find the segment by address */
    for (id = 0; id < shm_keymap.nobj; id++) {
        si = (struct shmid_ds *)get_ipc_by_id(&shm_keymap, id);
        if (si && si->shm_amp == shmaddr) {
            si->shm_nattach--;
            if (si->shm_nattach <= 0) {
                ipc_rm_id(&shm_keymap, id, 
                         (void (*)(struct IPCGeneric *))shm_destroy);
            }
            ret = 0;
            break;
        }
    }
    
    if (ret < 0) {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&shm_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Control shared memory segment */
int shmctl(int shmid, int cmd, struct shmid_ds *cbuf)
{
    int ret = -1;
    struct shmid_ds *si;
    
    ENTER();
    SHOWVALUE(shmid);
    SHOWVALUE(cmd);
    SHOWPOINTER(cbuf);
    
    ipc_lock(&shm_keymap);
    
    si = (struct shmid_ds *)get_ipc_by_id(&shm_keymap, shmid);
    if (si) {
        switch (cmd) {
        case IPC_STAT:
            if (cbuf) {
                memcpy(cbuf, si, sizeof(struct shmid_ds));
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        case IPC_SET:
            if (cbuf) {
                si->shm_perm.uid = cbuf->shm_perm.uid;
                si->shm_perm.gid = cbuf->shm_perm.gid;
                si->shm_perm.mode = cbuf->shm_perm.mode;
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        case IPC_RMID:
            ipc_rm_id(&shm_keymap, shmid, 
                     (void (*)(struct IPCGeneric *))shm_destroy);
            ret = 0;
            break;
            
        default:
            set_ipc_errno(EINVAL);
            break;
        }
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&shm_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Get shared memory IDs */
int shmids(int *buf, unsigned int nids, unsigned int *idcnt)
{
    int ret;
    
    ENTER();
    SHOWPOINTER(buf);
    SHOWVALUE(nids);
    SHOWPOINTER(idcnt);
    
    ret = ipc_ids(&shm_keymap, buf, nids, (int *)idcnt);
    
    SHOWVALUE(ret);
    return ret;
}
