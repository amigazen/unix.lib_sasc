#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * POSIX-mandated types for a 32-bit system.
 */

/* Used for block counts in file systems */
typedef long            blkcnt_t;

/* Used for block sizes */
typedef long            blksize_t;

/* Used for clock ticks */
typedef long            clock_t;

/* Used for device IDs */
typedef long            dev_t;

/* Used for group IDs */
typedef unsigned int    gid_t;

/* Used for file serial numbers (inodes) */
typedef unsigned long   ino_t;

/* Used for file modes (permissions) */
typedef unsigned int    mode_t;

/* Used for link counts */
typedef unsigned short  nlink_t;

/* Used for file sizes and offsets */
typedef long            off_t;

/* Used for process IDs */
typedef int             pid_t;

/* Used for sizes of memory objects */
typedef unsigned int    size_t;

/* Used for byte counts or error indication (-1) */
typedef int             ssize_t;

/* Used for time in seconds */
typedef long            time_t;

/* Used for user IDs */
typedef unsigned int    uid_t;

/* Add this line for legacy BSD compatibility */
typedef char * caddr_t;   /* core address */


#ifdef __cplusplus
}
#endif

#endif /* !_SYS_TYPES_H_ */