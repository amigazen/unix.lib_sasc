#ifndef DIR_DATA_H
#define DIR_DATA_H

/*
 * This header defines the internal data structures used to emulate
 * the POSIX directory stream functions (opendir, readdir, etc.)
 * on AmigaOS.
 * 
 * We override the local sys/dir.h definitions to provide POSIX compatibility.
 */

// 1. Include necessary system and AmigaOS headers first
#include <dos/dos.h>      // For BPTR, struct FileInfoBlock, struct DateStamp

// 2. Override local sys/dir.h definitions for POSIX compatibility
#undef DIR
#undef struct dirent
#undef opendir
#undef readdir
#undef closedir
#undef telldir
#undef seekdir
#undef rewinddir

// 3. Define POSIX-compatible DIR structure
typedef struct {
  long dd_fd;             // File descriptor for the directory
  long dd_loc;            // Current location in the directory
  char *dd_buf;           // Pointer to internal buffer
  long dd_size;           // Size of internal buffer
} DIR;

// 4. Define POSIX-compatible directory entry structure
struct direct {
  unsigned long d_ino;          // Inode number (or unique file ID)
  long d_off;                   // Offset in directory
  unsigned short d_reclen;      // Length of this record
  unsigned short d_namlen;      // Length of the filename
  char d_name[256];             // Filename (null-terminated) - size can be adjusted
};

// 5. Define the internal, extended directory stream structure
typedef struct
{
  char *dirname;              // The name of the directory that was opened
  BPTR cdir;                  // The current directory when opendir() was called
  struct FileInfoBlock fib;   // A FileInfoBlock for AmigaDOS calls
  struct idirect *files;      // Head of the linked list of directory entries
  struct idirect *pos;        // Current position in the linked list for readdir()
  int seeked;                 // Flag to indicate if seekdir() has been called
} iDIR;

// 6. Define the structure for each cached directory entry
struct idirect
{
  struct idirect *next;       // Pointer to the next entry in the linked list

  /* Amiga-specific info needed for our custom stat() implementation */
  long numblocks;
  long size;
  struct DateStamp date;
  long type;
  long protection;

  /* The standard POSIX directory entry structure */
  struct direct entry;
};

// 7. Extern declarations for global variables used by the stat() hack
extern iDIR *last_info;
extern struct idirect *last_entry;

// 8. Function prototype for an internal helper function
BPTR _get_cd(void);

#endif /* DIR_DATA_H */
