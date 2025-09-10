/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * 
 * POSIX Shared Memory Implementation for Amiga
 * Implemented on top of System V IPC shared memory
 */

#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

/* POSIX shared memory object structure */
struct posix_shm_obj {
    char name[256];           /* POSIX name */
    int sysv_shmid;          /* System V shared memory ID */
    int ref_count;           /* Reference count */
    struct posix_shm_obj *next;
};

/* Global list of POSIX shared memory objects */
static struct posix_shm_obj *shm_list = NULL;
static struct SignalSemaphore *shm_list_lock = NULL;

/* Initialize POSIX shared memory system */
void posix_shm_init(void)
{
    if (!shm_list_lock) {
        shm_list_lock = AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC | MEMF_CLEAR);
        if (shm_list_lock) {
            InitSemaphore(shm_list_lock);
        }
    }
}

/* Cleanup POSIX shared memory system */
void posix_shm_cleanup(void)
{
    struct posix_shm_obj *obj, *next;
    
    if (shm_list_lock) {
        ObtainSemaphore(shm_list_lock);
        
        obj = shm_list;
        while (obj) {
            next = obj->next;
            /* Remove System V shared memory */
            shmctl(obj->sysv_shmid, IPC_RMID, NULL);
            FreeVec(obj);
            obj = next;
        }
        shm_list = NULL;
        
        ReleaseSemaphore(shm_list_lock);
        FreeMem(shm_list_lock, sizeof(struct SignalSemaphore));
        shm_list_lock = NULL;
    }
}

/* Generate a System V key from POSIX name */
static key_t posix_name_to_key(const char *name)
{
    char full_path[512];
    int id = 1;  /* Use 1 as default ID for POSIX shm */
    
    /* Create a pseudo-path for ftok */
    snprintf(full_path, sizeof(full_path), "/tmp/posix_shm_%s", name);
    
    return ftok(full_path, id);
}

/* Find POSIX shared memory object by name */
static struct posix_shm_obj *find_shm_obj(const char *name)
{
    struct posix_shm_obj *obj;
    
    if (!shm_list_lock) return NULL;
    
    ObtainSemaphore(shm_list_lock);
    
    obj = shm_list;
    while (obj) {
        if (strcmp(obj->name, name) == 0) {
            ReleaseSemaphore(shm_list_lock);
            return obj;
        }
        obj = obj->next;
    }
    
    ReleaseSemaphore(shm_list_lock);
    return NULL;
}

/* Add POSIX shared memory object to list */
static int add_shm_obj(const char *name, int sysv_shmid)
{
    struct posix_shm_obj *obj;
    
    if (!shm_list_lock) return -1;
    
    obj = AllocVec(sizeof(struct posix_shm_obj), MEMF_ANY | MEMF_CLEAR);
    if (!obj) return -1;
    
    strncpy(obj->name, name, sizeof(obj->name) - 1);
    obj->name[sizeof(obj->name) - 1] = '\0';
    obj->sysv_shmid = sysv_shmid;
    obj->ref_count = 1;
    
    ObtainSemaphore(shm_list_lock);
    obj->next = shm_list;
    shm_list = obj;
    ReleaseSemaphore(shm_list_lock);
    
    return 0;
}

/* Remove POSIX shared memory object from list */
static void remove_shm_obj(const char *name)
{
    struct posix_shm_obj *obj, *prev = NULL;
    
    if (!shm_list_lock) return;
    
    ObtainSemaphore(shm_list_lock);
    
    obj = shm_list;
    while (obj) {
        if (strcmp(obj->name, name) == 0) {
            if (prev) {
                prev->next = obj->next;
            } else {
                shm_list = obj->next;
            }
            FreeVec(obj);
            break;
        }
        prev = obj;
        obj = obj->next;
    }
    
    ReleaseSemaphore(shm_list_lock);
}

/* POSIX shared memory open/create */
int shm_open(const char *name, int oflag, mode_t mode)
{
    struct posix_shm_obj *obj;
    key_t key;
    int sysv_shmid;
    int flags = 0;
    
    ENTER();
    SHOWSTRING(name);
    SHOWVALUE(oflag);
    SHOWVALUE(mode);
    
    if (!name || name[0] != '/') {
        errno = EINVAL;
        return -1;
    }
    
    /* Skip leading slash for internal name */
    if (strlen(name) <= 1) {
        errno = EINVAL;
        return -1;
    }
    
    /* Initialize if needed */
    posix_shm_init();
    
    /* Check if object already exists */
    obj = find_shm_obj(name);
    if (obj) {
        if (oflag & O_CREAT && oflag & O_EXCL) {
            errno = EEXIST;
            return -1;
        }
        obj->ref_count++;
        return obj->sysv_shmid;  /* Return as file descriptor */
    }
    
    /* Object doesn't exist */
    if (!(oflag & O_CREAT)) {
        errno = ENOENT;
        return -1;
    }
    
    /* Generate System V key */
    key = posix_name_to_key(name);
    if (key == (key_t)-1) {
        errno = EINVAL;
        return -1;
    }
    
    /* Convert POSIX flags to System V flags */
    if (oflag & O_CREAT) flags |= IPC_CREAT;
    if (oflag & O_EXCL) flags |= IPC_EXCL;
    
    /* Create System V shared memory segment */
    sysv_shmid = shmget(key, 0, flags | (mode & 0777));
    if (sysv_shmid == -1) {
        return -1;
    }
    
    /* Add to our list */
    if (add_shm_obj(name, sysv_shmid) != 0) {
        shmctl(sysv_shmid, IPC_RMID, NULL);
        errno = ENOMEM;
        return -1;
    }
    
    SHOWVALUE(sysv_shmid);
    return sysv_shmid;  /* Return as file descriptor */
}

/* POSIX shared memory unlink */
int shm_unlink(const char *name)
{
    struct posix_shm_obj *obj;
    
    ENTER();
    SHOWSTRING(name);
    
    if (!name || name[0] != '/') {
        errno = EINVAL;
        return -1;
    }
    
    obj = find_shm_obj(name);
    if (!obj) {
        errno = ENOENT;
        return -1;
    }
    
    /* Decrement reference count */
    obj->ref_count--;
    
    /* If no more references, remove it */
    if (obj->ref_count <= 0) {
        /* Remove System V shared memory */
        shmctl(obj->sysv_shmid, IPC_RMID, NULL);
        remove_shm_obj(name);
    }
    
    return 0;
}
