#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <sys/types.h> /* For ino_t */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The DIR type represents a directory stream.
 * Internal implementation details are defined below.
 */
typedef struct DIR DIR;

/*
 * The dirent structure contains information about a single directory entry.
 * POSIX only requires d_ino and d_name, but we include BSD extensions.
 */
struct dirent {
    ino_t d_ino;            /* File serial number (inode number) */
    off_t d_off;            /* Offset to next record */
    unsigned short d_reclen; /* Length of this record */
    unsigned char d_namlen;  /* Length of string in d_name */
    char  d_name[256];      /* Null-terminated filename */
};

/*
 * The DIR structure for directory streams.
 * This is the actual implementation, not opaque.
 */
struct DIR {
    long dd_fd;             /* File descriptor */
    long dd_loc;            /* Current position in directory */
    long dd_size;           /* Size of directory buffer */
    char *dd_buf;           /* Directory buffer */
};


/* --- C89-compliant function prototypes --- */

DIR *opendir(const char *path);

struct dirent *readdir(DIR *dirp);

void rewinddir(DIR *dirp);

void closedir(DIR *dirp);


#ifdef __cplusplus
}
#endif

#endif /* !_DIRENT_H_ */
