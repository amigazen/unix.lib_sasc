#include "amiga.h"
#include "fibex.h"
#include <dos/dosextens.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

typedef struct DevProc		DevProc;

int
readlink(const char *path, char *name, int max)
{
    int r = -1;
    DevProc *dp = NULL;
    DevProc *dp2;
    short failsafe = 20;

    __chkabort();
    while (dp2 = GetDeviceProc(path, dp)) {
	dp = dp2;
	if (ReadLink(dp->dvp_Port, dp->dvp_Lock, path, name, max)) {
	    r = strlen(name);
	    break;
	}
	if (--failsafe == 0)
	    break;
	if ((dp->dvp_Flags & DVPF_ASSIGN) == 0)
	    break;
    }
    FreeDeviceProc(dp);
    return(r);
}

int
lstat(const char *name, struct stat *sbuf)
{
    char buf[256];
    struct FileInfoBlock *fib = 0;
    BPTR lock = 0;
    int ret;

    __chkabort();
    
    /* Check if this is a symbolic link first */
    if (readlink(name, buf, sizeof(buf)) >= 0) {
	/* It's a symbolic link - return link info, not target info */
	memset(sbuf, 0, sizeof(*sbuf));
	sbuf->st_dev = (long) DeviceProc((UBYTE *)name);
	sbuf->st_nlink = 1;
	/* Use default ownership for symbolic links */
	sbuf->st_uid = AMIGA_UID;
	sbuf->st_gid = AMIGA_GID;
	sbuf->st_blksize = 512;
	sbuf->st_mode = S_IFLNK | S_IREAD | S_IWRITE;
	/* 1-Jan-1978 */
	sbuf->st_ctime = sbuf->st_atime = sbuf->st_mtime = 252460800;
	return(0);
    }
    
    /* Not a symbolic link - use stat() logic but don't follow links */
    /* This is the same as stat() but we're being explicit about not following links */
    if ((fib = AllocDosObjectTags(DOS_FIB, TAG_END)) &&
	(lock = Lock(name, SHARED_LOCK)) &&
	Examine(lock, fib)) {
	BPTR parent = ParentDir(lock);
	int isroot = !parent;
	long handler = (long)((struct FileLock *)((long)lock << 2))->fl_Task;

	if (parent)
	    UnLock(parent);
	_fibstat(fib, isroot, sbuf, handler);
	ret = 0;
    } else {
	ret = -1;
	errno = convert_oserr(IoErr());
    }
    
    if (lock)
	UnLock(lock);
    if (fib)
	FreeDosObject(DOS_FIB, fib);
    return ret;
}
