/*
 * statvfs.c - POSIX filesystem statistics implementation
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include "include/sys/statvfs.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

/* Include our ustat header for the structure */
#include "include/ustat.h"

/* External function for sending packets to AmigaOS */
extern LONG SendPacket(struct MsgPort *handler, LONG action, LONG *arglist, LONG nargs);

/* Function prototype for filesystem limits */
static int get_filesystem_limits(long dev, long *name_max, long *path_max);

/* AmigaOS disk type constants */
#ifndef ID_DOS_DISK
#define ID_DOS_DISK 0x444F5300
#endif
#ifndef ID_FAST_DOS_DISK
#define ID_FAST_DOS_DISK 0x444F5301
#endif
#ifndef ID_INTER_DOS_DISK
#define ID_INTER_DOS_DISK 0x444F5302
#endif
#ifndef ID_INTER_FFS_DISK
#define ID_INTER_FFS_DISK 0x444F5303
#endif
#ifndef ID_DOS_LNFS_DISK
#define ID_DOS_LNFS_DISK 0x444F5304
#endif
#ifndef ID_FAST_DOS_LNFS_DISK
#define ID_FAST_DOS_LNFS_DISK 0x444F5305
#endif
#ifndef ID_DOS_DCFS_DISK
#define ID_DOS_DCFS_DISK 0x444F5306
#endif
#ifndef ID_FAST_DOS_DCFS_DISK
#define ID_FAST_DOS_DCFS_DISK 0x444F5307
#endif
#ifndef ID_DOS_DCFS_LNFS_DISK
#define ID_DOS_DCFS_LNFS_DISK 0x444F5308
#endif
#ifndef ID_FAST_DOS_DCFS_LNFS_DISK
#define ID_FAST_DOS_DCFS_LNFS_DISK 0x444F5309
#endif

/* AmigaOS disk state constants */
#ifndef ID_WRITE_PROTECTED
#define ID_WRITE_PROTECTED 0x00000001
#endif

/*
 * _get_filesystem_info() - Internal function to get filesystem information
 * 
 * This function retrieves filesystem statistics using AmigaOS InfoData
 * and populates a statvfs structure.
 *
 * Parameters:
 *   dev - Device identifier (from stat.st_dev)
 *   buf - Pointer to statvfs structure to fill
 *
 * Returns:
 *   0 on success, -1 on failure with errno set
 */
static int _get_filesystem_info(long dev, struct statvfs *buf)
{
    struct InfoData info;
    BPTR ifp;
    struct DeviceList *dlist;
    LONG nblocks, nfree;
    LONG bytes_per_block;
    long name_max, path_max;
    UBYTE *name;
    int len;
    
    if (!dev || !buf) {
        errno = EINVAL;
        return -1;
    }

    ifp = MKBADDR(&info);

    if (SendPacket((struct MsgPort *)dev, ACTION_DISK_INFO, (LONG *)&ifp, 1) != DOSFALSE) {

        /* Get basic filesystem information */
        nblocks = info.id_NumBlocks;
        nfree = info.id_NumBlocks - info.id_NumBlocksUsed;
        bytes_per_block = info.id_BytesPerBlock;

        /* Fill in the POSIX statvfs structure */
        buf->f_bsize = bytes_per_block;
        buf->f_frsize = bytes_per_block;  /* AmigaOS doesn't distinguish fragment size */
        buf->f_blocks = nblocks;
        buf->f_bfree = nfree;
        buf->f_bavail = nfree;  /* On AmigaOS, all free blocks are available */
        
        /* File node information (AmigaOS doesn't have inode limits) */
        buf->f_files = 0;       /* Not applicable on AmigaOS */
        buf->f_ffree = 0;       /* Not applicable on AmigaOS */
        buf->f_favail = 0;      /* Not applicable on AmigaOS */
        
        /* Filesystem ID and flags */
        buf->f_fsid = (unsigned long)dev;
        buf->f_flag = 0;        /* Will be set based on device properties */
        
        /* Get dynamic filesystem limits */
        if (get_filesystem_limits(dev, &name_max, &path_max) == 0) {
            buf->f_namemax = name_max;
        } else {
            buf->f_namemax = 30;  /* Fallback to traditional limit */
        }

        /* Initialize Amiga-specific fields */
        buf->f_fstypename[0] = '\0';
        buf->f_mntonname[0] = '\0';
        buf->f_mntfromname[0] = '\0';

        /* Get volume name and device information */
        dlist = BADDR(info.id_VolumeNode);
        if (dlist) {

            /* Convert BCPL string to C string for volume name */
            name = BADDR(dlist->dl_Name);
            if (name) {
                len = *name++;
                if (len > 255) len = 255; /* Ensure we don't overflow */
                strncpy(buf->f_mntfromname, (char *)name, len);
                buf->f_mntfromname[len] = '\0';
            }

            /* Determine filesystem type based on disk type */
            if (info.id_DiskType == ID_DOS_DISK) {
                strcpy(buf->f_fstypename, "OFS");
            } else if (info.id_DiskType == ID_FAST_DOS_DISK) {
                strcpy(buf->f_fstypename, "FFS");
            } else if (info.id_DiskType == ID_INTER_DOS_DISK) {
                strcpy(buf->f_fstypename, "IOFS");
            } else if (info.id_DiskType == ID_INTER_FFS_DISK) {
                strcpy(buf->f_fstypename, "IFFS");
            } else if (info.id_DiskType == ID_DOS_LNFS_DISK) {
                strcpy(buf->f_fstypename, "LNFS");
            } else if (info.id_DiskType == ID_FAST_DOS_LNFS_DISK) {
                strcpy(buf->f_fstypename, "FFS+LNFS");
            } else if (info.id_DiskType == ID_DOS_DCFS_DISK) {
                strcpy(buf->f_fstypename, "DCFS");
            } else if (info.id_DiskType == ID_FAST_DOS_DCFS_DISK) {
                strcpy(buf->f_fstypename, "FFS+DCFS");
            } else if (info.id_DiskType == ID_DOS_DCFS_LNFS_DISK) {
                strcpy(buf->f_fstypename, "DCFS+LNFS");
            } else if (info.id_DiskType == ID_FAST_DOS_DCFS_LNFS_DISK) {
                strcpy(buf->f_fstypename, "FFS+DCFS+LNFS");
            } else {
                /* Unknown filesystem type */
                sprintf(buf->f_fstypename, "UNKNOWN_%08lX", info.id_DiskType);
            }

            /* Set mount flags based on device properties and filesystem features */
            if (info.id_DiskState == ID_WRITE_PROTECTED) {
                buf->f_flag |= ST_RDONLY;
            } else {
                buf->f_flag |= ST_WRITE;
            }
            
            /* Add enhanced filesystem feature flags */
            if (info.id_DiskType == ID_DOS_DCFS_DISK ||
                info.id_DiskType == ID_FAST_DOS_DCFS_DISK ||
                info.id_DiskType == ID_DOS_DCFS_LNFS_DISK ||
                info.id_DiskType == ID_FAST_DOS_DCFS_LNFS_DISK) {
                buf->f_flag |= 0x2000;  /* DCFS flag */
            }
            
            if (info.id_DiskType == ID_DOS_LNFS_DISK ||
                info.id_DiskType == ID_FAST_DOS_LNFS_DISK ||
                info.id_DiskType == ID_DOS_DCFS_LNFS_DISK ||
                info.id_DiskType == ID_FAST_DOS_DCFS_LNFS_DISK) {
                buf->f_flag |= 0x4000;  /* LNFS flag */
            }
        }

        return 0;
    }

    errno = ENXIO;
    return -1;
}

/*
 * statvfs() - Get filesystem statistics by path
 *
 * This function provides POSIX-compliant filesystem statistics
 * for the filesystem containing the specified path.
 *
 * Parameters:
 *   path - Path to file or directory
 *   buf  - Pointer to statvfs structure to fill
 *
 * Returns:
 *   0 on success, -1 on failure with errno set
 */
int statvfs(const char *path, struct statvfs *buf)
{
    struct stat stat_buf;
    BPTR lock;
    char device_path[256];
    char *colon_pos;
    int device_len;
    long handler;
    
    __chkabort();
    
    if (!path || !buf) {
        errno = EINVAL;
        return -1;
    }

    /* Get a lock on the path to determine the device */
    lock = Lock((char *)path, ACCESS_READ);
    if (!lock) {
        errno = ENOENT;
        return -1;
    }

    /* Get file status to obtain device ID */
    if (stat(path, &stat_buf) == 0) {
        int result = _get_filesystem_info(stat_buf.st_dev, buf);
        UnLock(lock);
        return result;
    }

    /* If stat failed, try to get device info directly from the lock */
    UnLock(lock);
    
    /* Try to get device info using the path directly */
    if (strchr(path, ':')) {
        /* Path contains device specification, try to get device info */
        colon_pos = strchr(path, ':');
        device_len = colon_pos - path + 1;
        
        if (device_len < sizeof(device_path)) {
            strncpy(device_path, path, device_len);
            device_path[device_len] = '\0';
            
            lock = Lock(device_path, ACCESS_READ);
            if (lock) {
                handler = (long)((struct FileLock *)((long)lock << 2))->fl_Task;
                UnLock(lock);
                return _get_filesystem_info(handler, buf);
            }
        }
    }

    errno = ENOENT;
    return -1;
}

/*
 * fstatvfs() - Get filesystem statistics by file descriptor
 *
 * This function provides POSIX-compliant filesystem statistics
 * for the filesystem containing the specified file descriptor.
 *
 * Parameters:
 *   fd  - File descriptor
 *   buf - Pointer to statvfs structure to fill
 *
 * Returns:
 *   0 on success, -1 on failure with errno set
 */
int fstatvfs(int fd, struct statvfs *buf)
{
    struct stat stat_buf;
    
    __chkabort();
    
    if (fd < 0 || !buf) {
        errno = EINVAL;
        return -1;
    }

    /* Get file status to obtain device ID */
    if (fstat(fd, &stat_buf) == 0) {
        return _get_filesystem_info(stat_buf.st_dev, buf);
    }

    /* If fstat failed, we can't determine the filesystem */
    return -1;
}

/*
 * get_filesystem_limits() - Get filesystem name and path limits
 *
 * This function retrieves the maximum filename and path lengths
 * for the specified filesystem device.
 *
 * Parameters:
 *   dev - Device identifier
 *   name_max - Pointer to store maximum filename length
 *   path_max - Pointer to store maximum path length
 *
 * Returns:
 *   0 on success, -1 on failure
 */
static int get_filesystem_limits(long dev, long *name_max, long *path_max)
{
    /* For now, return standard AmigaOS limits */
    if (name_max) {
        *name_max = 30;  /* Traditional AmigaOS filename limit */
    }
    if (path_max) {
        *path_max = 256; /* Standard path limit */
    }
    return 0;
}
