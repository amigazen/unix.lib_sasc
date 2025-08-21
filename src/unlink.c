#include "amiga.h"
#include "files.h"
#include <amiga/ioctl.h> /* For _AMIGA_DELETE_IF_ME */
#include <errno.h>

/* Helper function now correctly takes a const char * */
static int delete_open_file(const char *name)
{
  /* C89 variables */
  BPTR nlock;
  int err;
  int i;
  
  /* Cast away const for AmigaDOS function that expects (char *) */
  nlock = Lock((char *)name, SHARED_LOCK);
  if (nlock)
    {
      err = errno;
      i = _last_fd();
      while (--i >= 0)
      {
        if (ioctl(i, _AMIGA_DELETE_IF_ME, &nlock) == 0)
        {
          errno = err;
          return 0;
        } 
      }
      UnLock(nlock);
   }
  return -1;
}


/*
 * POSIX-compliant unlink function.
 * The 'file' argument is now correctly declared as 'const char *'.
 */
int unlink(const char *file)
{
  long err;

  chkabort();
  
  /* Cast away const for AmigaDOS functions */
  if (DeleteFile((char *)file))
  {
    return 0;
  }
  
  err = IoErr();
  if (err == ERROR_DELETE_PROTECTED)
    {
      /* Try to strip protection and delete again */
      if (SetProtection((char *)file, 0) && DeleteFile((char *)file))
      {
        return 0;
      }
      err = IoErr();
    }
    
  /* Your clever handler for files that are currently in use */
  if (err == ERROR_OBJECT_IN_USE && delete_open_file(file) == 0)
  {
    return 0;
  }
  
  errno = convert_oserr(err);
  return -1;
}