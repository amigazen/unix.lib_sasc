/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on sysvipc implementation by Peter Bengtsson
 *
 * ftok - generate an IPC key from a pathname and project identifier
 */

#include "include/sys/ipc.h"
#include "debug.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <errno.h>

key_t ftok(const char *path, const int id)
{
    key_t key = (key_t)-1;
    BPTR lock = 0;
    uint32_t blockno = 0;
    struct FileLock *flock;
    struct Library *DOSBase;
    
    ENTER();
    SHOWSTRING(path);
    SHOWVALUE(id);
    
    if (path) {
        DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0L);
        if (DOSBase) {
            lock = Lock(path, SHARED_LOCK);
            
            if (lock) {
                flock = BADDR(lock);
                blockno = flock->fl_Key;
                UnLock(lock);
                key = (blockno << 8) | (id & 0xFF);
            } else {
                errno = ENOENT;
            }
            
            CloseLibrary((struct Library *)DOSBase);
        } else {
            errno = ENOENT;
        }
    } else {
        errno = EFAULT;
    }
    
    SHOWVALUE(key);
    return key;
}
