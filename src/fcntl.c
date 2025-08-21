/*
 * fcntl.c - file descriptor control (POSIX compliant)
 *
 * This function performs one of the operations described below on
 * the open file descriptor fd. The operation is determined by cmd.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <fcntl.h>
#include <errno.h>
#include "files.h"
#include <sys/filio.h>
#include <stdarg.h>

/* Flags that can be changed with fcntl */
/* #define FCNTL_FLAGS (O_NDELAY | O_APPEND) */

int fcntl(int fd, int cmd, ...)
{
/* Original unix.lib code
  struct fileinfo *fi;
  va_list args;
  int arg;

  chkabort();
  va_start(args, cmd);
  arg = va_arg(args, int);
  va_end(args);

  if (fi = _find_fd(fd))
    {
      switch (cmd)
    {
    default: errno = EINVAL; break;
    case F_GETFL: return fi->flags & FCNTL_FLAGS;
    case F_SETFL:
      {
        int oldfl = fi->flags;

        fi->flags = (fi->flags & ~FCNTL_FLAGS) | (arg & FCNTL_FLAGS);
        if ((oldfl & O_NDELAY) != (fi->flags & O_NDELAY))
          {
        int ndelay = fi->flags & O_NDELAY;

        return fi->ioctl(fi->userinfo, FIONBIO, &ndelay);
          }
        return 0;
      }
    }
    }
  return -1;
  */

    struct fileinfo *fi;
    va_list args;
    int arg;
    
    chkabort();
    
    /* Check if file descriptor is valid */
    if ((fi = _find_fd(fd)) == NULL) {
        errno = EBADF;
        return -1;
    }
    
    va_start(args, cmd);
    
    switch (cmd) {
        case F_DUPFD:
            /* Duplicate file descriptor */
            arg = va_arg(args, int);
            va_end(args);
            return dup2(fd, arg);
            
        case F_GETFD:
            /* Get file descriptor flags */
            va_end(args);
            return 0;  /* No close-on-exec flag on AmigaOS */
            
        case F_SETFD:
            /* Set file descriptor flags */
            arg = va_arg(args, int);
            va_end(args);
            /* Ignore close-on-exec flag on AmigaOS */
            return 0;
            
        case F_GETFL:
            /* Get file status flags */
            va_end(args);
            return fi->flags;
            
        case F_SETFL:
            /* Set file status flags */
            arg = va_arg(args, int);
            va_end(args);
            /* Only allow setting O_APPEND and O_NONBLOCK */
            fi->flags = (fi->flags & ~(O_APPEND | O_NONBLOCK)) | 
                       (arg & (O_APPEND | O_NONBLOCK));
            return 0;
            
        case F_GETLK:
        case F_SETLK:
        case F_SETLKW:
            /* File locking not supported on AmigaOS */
            va_end(args);
            errno = ENOSYS;
            return -1;
            
        default:
            va_end(args);
            errno = EINVAL;
            return -1;
    }
}
