/*
 * statvfs.c - POSIX filesystem statistics implementation
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

/* Include our ustat header for the structure */
#include "include/ustat.h"

/* External function for sending packets to AmigaOS */
extern LONG SendPacket(struct MsgPort *handler, LONG action, LONG *arglist, LONG nargs);

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
    if (!dev || !buf) {
        errno = EINVAL;
        return -1;
    }

    __aligned struct InfoData info;
    BPTR ifp = MKBADDR(&info);

    if (SendPacket((struct MsgPort *)dev, ACTION_DISK_INFO, (LONG *)&ifp, 1) != DOSFALSE) {
        struct DeviceList *dlist;
        LONG nblocks, nfree;
        LONG bytes_per_block;

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
        buf->f_namemax = 107;   /* AmigaDOS filename length limit */

        /* Initialize Amiga-specific fields */
        buf->f_fstypename[0] = '\0';
        buf->f_mntonname[0] = '\0';
        buf->f_mntfromname[0] = '\0';

        /* Get volume name and device information */
        dlist = BADDR(info.id_VolumeNode);
        if (dlist) {
            UBYTE *name;
            int len;

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
            } else {
                /* Use the disk type string if available */
                if (info.id_DiskType < 0) {
                    strcpy(buf->f_fstypename, "UNKNOWN");
                } else {
                    sprintf(buf->f_fstypename, "TYPE_%ld", info.id_DiskType);
                }
            }

            /* Set mount flags based on device properties */
            if (info.id_DiskState == ID_WRITE_PROTECTED) {
                buf->f_flag |= ST_RDONLY;
            } else {
                buf->f_flag |= ST_WRITE;
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
        char device_path[256];
        char *colon_pos = strchr(path, ':');
        int device_len = colon_pos - path + 1;
        
        if (device_len < sizeof(device_path)) {
            strncpy(device_path, path, device_len);
            device_path[device_len] = '\0';
            
            lock = Lock(device_path, ACCESS_READ);
            if (lock) {
                long handler = (long)((struct FileLock *)((long)lock << 2))->fl_Task;
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
