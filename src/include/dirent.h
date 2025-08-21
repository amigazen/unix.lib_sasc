#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <sys/types.h> /* For ino_t */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The DIR type is an opaque structure that represents a directory stream.
 * You should never need to access its members directly.
 */
typedef struct DIR DIR;

/*
 * The dirent structure contains information about a single directory entry.
 * POSIX only requires d_ino and d_name.
 */
struct dirent {
    ino_t d_ino;            /* File serial number (inode number) */
    char  d_name[256];      /* Null-terminated filename */
};


/* --- C89-compliant function prototypes --- */

DIR *opendir(const char *path);

struct dirent *readdir(DIR *dirp);

void rewinddir(DIR *dirp);

int closedir(DIR *dirp);


#ifdef __cplusplus
}
#endif

#endif /* !_DIRENT_H_ */