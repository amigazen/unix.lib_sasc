#include "amiga.h"
#include "files.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Creates and opens a file for writing. This is a legacy function.
 * It is equivalent to open(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode).
 */
int creat(const char *pathname, mode_t mode)
{
  chkabort(); // Your custom function for Amiga
  return open(pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
}