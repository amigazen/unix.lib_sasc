/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * 
 * Memory Mapping Implementation for Amiga
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

/* Memory mapping structure */
struct mmap_region {
    void *addr;              /* Mapped address */
    size_t len;              /* Length of mapping */
    int prot;                /* Protection flags */
    int flags;               /* Mapping flags */
    int fd;                  /* File descriptor (or System V shmid) */
    off_t offset;            /* File offset */
    struct mmap_region *next;
};

/* Global list of memory mappings */
static struct mmap_region *mmap_list = NULL;
static struct SignalSemaphore *mmap_lock = NULL;
static void *mmap_base = NULL;
static size_t mmap_size = 0;

/* Initialize memory mapping system */
void mmap_init(void)
{
    if (!mmap_lock) {
        mmap_lock = AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC | MEMF_CLEAR);
        if (mmap_lock) {
            InitSemaphore(mmap_lock);
        }
        
        /* Allocate a large chunk of memory for mappings */
        mmap_size = 16 * 1024 * 1024;  /* 16MB */
        mmap_base = AllocVec(mmap_size, MEMF_ANY | MEMF_CLEAR);
    }
}

/* Cleanup memory mapping system */
void mmap_cleanup(void)
{
    struct mmap_region *region, *next;
    
    if (mmap_lock) {
        ObtainSemaphore(mmap_lock);
        
        region = mmap_list;
        while (region) {
            next = region->next;
            FreeVec(region);
            region = next;
        }
        mmap_list = NULL;
        
        ReleaseSemaphore(mmap_lock);
        FreeMem(mmap_lock, sizeof(struct SignalSemaphore));
        mmap_lock = NULL;
    }
    
    if (mmap_base) {
        FreeVec(mmap_base);
        mmap_base = NULL;
        mmap_size = 0;
    }
}

/* Find a free region in our mapping space */
static void *find_free_region(size_t len)
{
    struct mmap_region *region;
    void *current_addr = mmap_base;
    void *end_addr = (char *)mmap_base + mmap_size;
    
    if (!mmap_lock) return NULL;
    
    ObtainSemaphore(mmap_lock);
    
    region = mmap_list;
    while (region) {
        /* Check if there's space before this region */
        if ((char *)current_addr + len <= (char *)region->addr) {
            ReleaseSemaphore(mmap_lock);
            return current_addr;
        }
        current_addr = (char *)region->addr + region->len;
        region = region->next;
    }
    
    /* Check if there's space at the end */
    if ((char *)current_addr + len <= (char *)end_addr) {
        ReleaseSemaphore(mmap_lock);
        return current_addr;
    }
    
    ReleaseSemaphore(mmap_lock);
    return NULL;
}

/* Add memory mapping to list */
static int add_mmap_region(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    struct mmap_region *region;
    
    if (!mmap_lock) return -1;
    
    region = AllocVec(sizeof(struct mmap_region), MEMF_ANY | MEMF_CLEAR);
    if (!region) return -1;
    
    region->addr = addr;
    region->len = len;
    region->prot = prot;
    region->flags = flags;
    region->fd = fd;
    region->offset = offset;
    
    ObtainSemaphore(mmap_lock);
    region->next = mmap_list;
    mmap_list = region;
    ReleaseSemaphore(mmap_lock);
    
    return 0;
}

/* Remove memory mapping from list */
static void remove_mmap_region(void *addr)
{
    struct mmap_region *region, *prev = NULL;
    
    if (!mmap_lock) return;
    
    ObtainSemaphore(mmap_lock);
    
    region = mmap_list;
    while (region) {
        if (region->addr == addr) {
            if (prev) {
                prev->next = region->next;
            } else {
                mmap_list = region->next;
            }
            FreeVec(region);
            break;
        }
        prev = region;
        region = region->next;
    }
    
    ReleaseSemaphore(mmap_lock);
}

/* Map memory or device into process address space */
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t offset)
{
    void *map_addr;
    struct shmid_ds shm_info;
    int shmid = fildes;  /* Treat file descriptor as System V shmid for now */
    
    ENTER();
    SHOWPOINTER(addr);
    SHOWVALUE(len);
    SHOWVALUE(prot);
    SHOWVALUE(flags);
    SHOWVALUE(fildes);
    SHOWVALUE(offset);
    
    /* Initialize if needed */
    mmap_init();
    
    /* Validate parameters */
    if (len == 0) {
        errno = EINVAL;
        return MAP_FAILED;
    }
    
    /* For now, only support anonymous mappings and System V shared memory */
    if (fildes == -1) {
        /* Anonymous mapping - allocate memory */
        map_addr = AllocVec(len, MEMF_ANY | MEMF_CLEAR);
        if (!map_addr) {
            errno = ENOMEM;
            return MAP_FAILED;
        }
    } else {
        /* Map from System V shared memory */
        if (shmctl(shmid, IPC_STAT, &shm_info) == -1) {
            errno = EINVAL;
            return MAP_FAILED;
        }
        
        if (len > shm_info.shm_segsz) {
            errno = EINVAL;
            return MAP_FAILED;
        }
        
        /* Attach to shared memory */
        map_addr = shmat(shmid, NULL, 0);
        if (map_addr == (void *)-1) {
            return MAP_FAILED;
        }
    }
    
    /* If MAP_FIXED is specified, use the exact address */
    if (flags & MAP_FIXED) {
        if (addr && addr != map_addr) {
            if (fildes == -1) {
                FreeVec(map_addr);
            } else {
                shmdt(map_addr);
            }
            errno = EINVAL;
            return MAP_FAILED;
        }
        map_addr = addr;
    } else if (addr) {
        /* Preferred address - try to use it if possible */
        if (fildes == -1) {
            FreeVec(map_addr);
            map_addr = AllocVec(len, MEMF_ANY | MEMF_CLEAR);
        }
    }
    
    /* Add to our mapping list */
    if (add_mmap_region(map_addr, len, prot, flags, fildes, offset) != 0) {
        if (fildes == -1) {
            FreeVec(map_addr);
        } else {
            shmdt(map_addr);
        }
        errno = ENOMEM;
        return MAP_FAILED;
    }
    
    SHOWPOINTER(map_addr);
    return map_addr;
}

/* Unmap memory */
int munmap(void *addr, size_t len)
{
    struct mmap_region *region;
    int found = 0;
    
    ENTER();
    SHOWPOINTER(addr);
    SHOWVALUE(len);
    
    if (!mmap_lock) {
        errno = EINVAL;
        return -1;
    }
    
    ObtainSemaphore(mmap_lock);
    
    region = mmap_list;
    while (region) {
        if (region->addr == addr) {
            found = 1;
            break;
        }
        region = region->next;
    }
    
    ReleaseSemaphore(mmap_lock);
    
    if (!found) {
        errno = EINVAL;
        return -1;
    }
    
    /* Free the memory */
    if (region->fd == -1) {
        /* Anonymous mapping */
        FreeVec(addr);
    } else {
        /* System V shared memory */
        shmdt(addr);
    }
    
    /* Remove from list */
    remove_mmap_region(addr);
    
    return 0;
}

/* Change memory protection */
int mprotect(void *addr, size_t len, int prot)
{
    struct mmap_region *region;
    int found = 0;
    
    ENTER();
    SHOWPOINTER(addr);
    SHOWVALUE(len);
    SHOWVALUE(prot);
    
    if (!mmap_lock) {
        errno = EINVAL;
        return -1;
    }
    
    ObtainSemaphore(mmap_lock);
    
    region = mmap_list;
    while (region) {
        if (region->addr == addr) {
            found = 1;
            region->prot = prot;
            break;
        }
        region = region->next;
    }
    
    ReleaseSemaphore(mmap_lock);
    
    if (!found) {
        errno = EINVAL;
        return -1;
    }
    
    /* On Amiga, we can't actually change memory protection */
    /* Just update our record */
    
    return 0;
}

/* Synchronize memory with storage */
int msync(void *addr, size_t len, int flags)
{
    struct mmap_region *region;
    int found = 0;
    
    ENTER();
    SHOWPOINTER(addr);
    SHOWVALUE(len);
    SHOWVALUE(flags);
    
    if (!mmap_lock) {
        errno = EINVAL;
        return -1;
    }
    
    ObtainSemaphore(mmap_lock);
    
    region = mmap_list;
    while (region) {
        if (region->addr == addr) {
            found = 1;
            break;
        }
        region = region->next;
    }
    
    ReleaseSemaphore(mmap_lock);
    
    if (!found) {
        errno = EINVAL;
        return -1;
    }
    
    /* On Amiga, shared memory is already synchronized */
    /* For file mappings, we would sync to disk here */
    
    return 0;
}
