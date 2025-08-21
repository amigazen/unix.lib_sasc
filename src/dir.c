#include <stdlib.h>   // For malloc(), free()
#include <string.h>   // For strlen(), strcpy()
#include <errno.h>    // For standard error numbers like ENOMEM
#include <stddef.h>   // For offsetof()

#include "amiga.h"
#include "dir_data.h"

/*
  The dir_data.h header now handles undefining and redefining
  the directory functions to ensure POSIX compatibility.
*/

/*
  opendir/readdir/etc ... emulation for AmigaOS.
  This version reads the entire directory into a linked list in memory
  upon opendir(), and then traverses that list for subsequent calls.
*/

// These are global variables for stat() support
static iDIR *last_info = NULL;
static struct idirect *last_entry = NULL;

// Simple implementation of _get_cd since it's not used elsewhere
static long _get_cd(void)
{
  return 0; // Return 0 since cdir is not used
}

// Function prototypes to avoid warnings
static void free_entries(iDIR *info);
static int gobble_dir(DIR *dir);


/*
 * Frees the linked list of directory entries.
 */
static void free_entries(iDIR *info)
{
  struct idirect *scan = info->files;
  
  while (scan)
    {
      struct idirect *next = scan->next;
      free(scan);
      scan = next;
    }
  info->files = NULL; // Avoid dangling pointer
}

/*
 * Reads all entries from an AmigaDOS directory lock into a linked list.
 */
static int gobble_dir(DIR *dir)
{
  iDIR *info = (iDIR *)dir->dd_buf;
  int ioerr;
  struct idirect **last = &info->files;
  
  free_entries(info);
  last_info = 0;
  info->files = 0;
  dir->dd_loc = 0;

  while (ExNext(dir->dd_fd, &info->fib))
    {
      size_t namlen = strlen(info->fib.fib_FileName);
      // Calculate required size for the struct plus the flexible name array
      size_t reclen = offsetof(struct idirect, entry.d_name) + namlen + 1;
      struct idirect *newentry = (struct idirect *)malloc(reclen);
      struct direct *entry;
      
      if (!newentry)
      {
        errno = ENOMEM;
        return 0; // Failure
      }
      newentry->next = 0;
      *last = newentry;
      last = &newentry->next;
      
      // Cache Amiga-specific file info
      newentry->numblocks = info->fib.fib_NumBlocks;
      newentry->size = info->fib.fib_Size;
      newentry->date = info->fib.fib_Date;
      newentry->type = info->fib.fib_DirEntryType;
      newentry->protection = info->fib.fib_Protection;
      
      // Populate the POSIX-like direct entry
      entry = &newentry->entry;
      entry->d_ino = info->fib.fib_DiskKey;
      entry->d_reclen = reclen;
      entry->d_namlen = namlen;
      entry->d_off = dir->dd_loc++;
      strcpy(entry->d_name, info->fib.fib_FileName);
    }

  info->pos = info->files; // Reset position to the start of the list
  dir->dd_loc = 0;
  ioerr = IoErr();

  // In AmigaDOS, ExNext failing usually means no more entries (success)
  // Only set errno if there's a real error
  if (ioerr != 0) {
    errno = convert_oserr(ioerr);
    return 0; // Failure
  }
  
  return 1; // Success
}

/*
 * Opens a directory stream.
 */
DIR *opendir(const char *dirname)
{
  // In C, you don't need to cast the result of malloc
  DIR *new = (DIR *)malloc(sizeof *new);
  iDIR *info = (iDIR *)malloc(sizeof *info);
  char *dircopy = (char *)malloc(strlen(dirname) + 1);

  chkabort();
  if (new && dircopy && info)
    {
      new->dd_buf = (char *)info;
      new->dd_size = sizeof *info;
      
      info->files = info->pos = 0;
      info->seeked = 0;
      info->dirname = dircopy;
      strcpy(dircopy, dirname);
      info->cdir = _get_cd();
      
      if ((new->dd_fd = Lock(dirname, ACCESS_READ)) &&
          Examine(new->dd_fd, &info->fib))
      {
        if (gobble_dir(new)) {
          return new; // Success
        }
      }
      else {
        errno = convert_oserr((int)IoErr());
      }
      // Cleanup on failure
      closedir(new);
      return NULL;
    }
  
  errno = ENOMEM;
  if (new) free(new);
  if (dircopy) free(dircopy);
  if (info) free(info);
  
  return NULL;
}

/*
 * Closes a directory stream. Must return an int.
 */
int closedir(DIR *dir)
{
  iDIR *info = (iDIR *)dir->dd_buf;
  
  chkabort();
  last_info = 0;
  free_entries(info);
  free(info->dirname);
  if (dir->dd_fd) {
    UnLock(dir->dd_fd);
  }
  free(dir->dd_buf);
  free(dir);

  return 0; // Return 0 for success
}

/*
 * Reads the next entry from a directory stream.
 */
struct direct *readdir(DIR *dir)
{
  iDIR *info = (iDIR *)dir->dd_buf;
  struct direct *entry = NULL;
  
  chkabort();
  if (info->seeked)
    {
      long cloc = 0;
      struct idirect *pos = info->files;
      
      while (cloc < dir->dd_loc && pos)
      {
        cloc++; 
        pos = pos->next;
      }
      info->pos = pos;
      info->seeked = 0;
    }

  if (info->pos)
    {
      entry = &info->pos->entry;
      
      // Update globals for stat() hack
      last_info = info;
      last_entry = info->pos;
      
      info->pos = info->pos->next;
      dir->dd_loc++;
    }
  return entry;
}

/*
 * Returns the current location in the directory stream.
 */
long telldir(DIR *dir)
{
  chkabort();
  return dir->dd_loc;
}

/*
 * Sets the location in the directory stream for the next readdir().
 */
void seekdir(DIR *dir, long loc)
{
  iDIR *info = (iDIR *)dir->dd_buf;
  
  chkabort();
  info->seeked = 1;
  dir->dd_loc = loc;
}

/*
 * Resets a directory stream to the beginning.
 */
void rewinddir(DIR *dir)
{
  iDIR *info = (iDIR *)dir->dd_buf;

  chkabort();
  info->pos = info->files; // Reset position to the start of the list
  dir->dd_loc = 0;
  info->seeked = 0;
}
