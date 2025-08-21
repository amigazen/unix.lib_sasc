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
      if (mode == F_OK) {
        ret = 0;
      } else {
        struct stat sbuf;
        BPTR parent = ParentDir(lock);
        int isroot = !parent;

        if (parent) UnLock(parent);
        
        // This helper converts Amiga FIB protection bits to a POSIX st_mode.
        // It should set S_IXUSR if either the 'e' (execute) or 's' (script)
        // bit is set on the Amiga file for full compatibility.
        _fibstat(fib, isroot, &sbuf);

        ret = 0; // Assume success unless a check fails

        // Check each requested permission. If any check fails, deny access.
        if (((mode & R_OK) && !(sbuf.st_mode & S_IRUSR)) ||
            ((mode & W_OK) && !(sbuf.st_mode & S_IWUSR)) ||
            ((mode & X_OK) && !(sbuf.st_mode & S_IXUSR))) {
          errno = EACCES; // Access denied
          ret = -1;
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