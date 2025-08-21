#include <stdarg.h>
#include "amiga.h"
#include "files.h"

/*
 * C89-compliant POSIX ioctl function.
 * The third argument is now variadic (...) to match the standard.
 */
int ioctl(int fd, unsigned long request, ...)
{
  struct fileinfo *fi;
  void *arg;
  va_list args;

  chkabort();

  /* Initialize variadic argument list */
  va_start(args, request);
  /*
   * We assume the optional third argument is a pointer, which covers
   * most use cases. We extract it from the argument list here.
   */
  arg = va_arg(args, void *);
  /* Clean up the argument list */
  va_end(args);

  fi = _find_fd(fd);
  if (fi)
    {
      /* Pass the extracted argument to the underlying file-specific handler */
      return fi->ioctl(fi->userinfo, request, arg);
    }
    
  return -1;
}