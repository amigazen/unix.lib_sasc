#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

/* POSIX requires these types to be defined in <sys/types.h> */
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * POSIX-compliant structure for file status.
 * Contains the fields required by the POSIX.1-2017 standard.
 */
 struct stat {
    dev_t     st_dev;      /* ID of device containing file */
    ino_t     st_ino;      /* File serial number (inode number) */
    mode_t    st_mode;     /* Mode of file (permissions, type) */
    nlink_t   st_nlink;    /* Number of hard links */
    uid_t     st_uid;      /* User ID of owner */
    gid_t     st_gid;      /* Group ID of owner */
    dev_t     st_rdev;     /* Device ID (if special file) */
    off_t     st_size;     /* File size in bytes */
    time_t    st_atime;    /* Time of last access */
    time_t    st_mtime;    /* Time of last data modification */
    time_t    st_ctime;    /* Time of last status change */
    blksize_t st_blksize;  /* A file system-specific preferred I/O block size */
    blkcnt_t  st_blocks;   /* Number of blocks allocated for this object */
};

/* File type masks for st_mode */
#define S_IFMT   0170000 /* Type of file mask */
#define S_IFIFO  0010000 /* Named pipe (fifo) */
#define S_IFCHR  0020000 /* Character special file */
#define S_IFDIR  0040000 /* Directory */
#define S_IFBLK  0060000 /* Block special file */
#define S_IFREG  0100000 /* Regular file */
#define S_IFLNK  0120000 /* Symbolic link */

/* File type testing macros */
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)

/* File mode bits (permissions) */
#define S_ISUID 0004000 /* Set-user-ID on execution */
#define S_ISGID 0002000 /* Set-group-ID on execution */
#define S_ISVTX 0001000 /* Sticky bit */

/* Read, write, and execute permissions for owner */
#define S_IRWXU 0000700 /* RWE mask for owner */
#define S_IRUSR 0000400 /* R for owner */
#define S_IWUSR 0000200 /* W for owner */
#define S_IXUSR 0000100 /* X for owner */

/* Read, write, and execute permissions for group */
#define S_IRWXG 0000070 /* RWE mask for group */
#define S_IRGRP 0000040 /* R for group */
#define S_IWGRP 0000020 /* W for group */
#define S_IXGRP 0000010 /* X for group */

/* Read, write, and execute permissions for other */
#define S_IRWXO 0000007 /* RWE mask for other */
#define S_IROTH 0000004 /* R for other */
#define S_IWOTH 0000002 /* W for other */
#define S_IXOTH 0000001 /* X for other */

/* Standard POSIX function prototypes */
int    chmod(const char *path, mode_t mode);
int    fstat(int fildes, struct stat *buf);
int    mkdir(const char *path, mode_t mode);
int    mkfifo(const char *path, mode_t mode);
int    stat(const char *path, struct stat *buf);
mode_t umask(mode_t cmask);

/* XSI (X/Open System Interfaces) extensions are common and expected */
int    fchmod(int fildes, mode_t mode);
int    lstat(const char *path, struct stat *buf);
#ifdef __cplusplus
}
#endif

#endif /* !_SYS_STAT_H_ */