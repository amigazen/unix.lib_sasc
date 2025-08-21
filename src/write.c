#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>  // For size_t

#include "amiga.h"
#include "files.h"

/*
 * Modern POSIX-compatible write() function implementation for AmigaOS.
 * This function provides POSIX write() semantics while maintaining
 * AmigaOS-specific functionality for file handling.
 */

ssize_t write(int fd, const void *buffer, size_t length)
{
  struct fileinfo *fi;

  chkabort();
  
  fi = _find_fd(fd);
  if (fi == NULL) {
    errno = EBADF;  // Invalid file descriptor
    return -1;
  }
  
  if (fi->flags & FI_WRITE) {
    if (fi->flags & O_APPEND) {
      fi->lseek(fi->userinfo, 0, SEEK_END);
    }
    return fi->write(fi->userinfo, buffer, length);
  }
  
  errno = EACCES;  // Permission denied - file not open for writing
  return -1;
}
