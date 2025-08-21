#include "amiga.h"
#include <utility/tagitem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h> 
#include <unistd.h> 

int access(const char *name, int mode)
{
  struct FileInfoBlock *fib;
  BPTR lock = 0;
  int ret = -1;

  chkabort(); 

  if ((fib = AllocDosObjectTags(DOS_FIB, TAG_END)) &&
      (lock = Lock(name, ACCESS_READ)) &&
      Examine(lock, fib))
    {
      // If we are just checking for existence (F_OK), we're done.
      // Lock() and Examine() succeeding is proof of existence.
      if (mode == F_OK) {
        ret = 0;
      } else {
        struct stat sbuf;
        int fmode = 0;
        BPTR parent = ParentDir(lock);
        int isroot = !parent;

        if (parent) UnLock(parent);
        
        // Assuming _fibstat correctly converts Amiga protection bits
        // to a POSIX-like st_mode field.
        _fibstat(fib, isroot, &sbuf);

        if (sbuf.st_mode & S_IREAD)  fmode |= R_OK;
        if (sbuf.st_mode & S_IWRITE) fmode |= W_OK;
        if (sbuf.st_mode & S_IEXEC)  fmode |= X_OK;
        
        // Check if all requested permission bits are set in the file's mode
        if ((fmode & mode) == mode) {
          ret = 0;
        } else {
          errno = EACCES; // Access denied
        }
      }
    }
  else {
    // If Lock() or Examine() fails, set errno based on the AmigaDOS error
    errno = convert_oserr(IoErr());
  }

  if (lock) UnLock(lock);
  if (fib) FreeDosObject(DOS_FIB, fib);
  
  return ret;
}