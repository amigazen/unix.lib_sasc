/*
 * ustat - get file system statistics for Amiga
 *
 * Copyright (C) 1995 by Ingo Wilken
 * Copyright (C) 2025 by amigazen project
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <errno.h>
#include <string.h>

/* Include the ustat header for our own function declarations */
#include "include/ustat.h"

/* External function for sending packets to AmigaOS */
extern LONG SendPacket(struct MsgPort *handler, LONG action, LONG *arglist, LONG nargs);

/*
 * ustat() - Get file system statistics
 * 
 * This function provides information about a file system device,
 * including free space and volume name.
 *
 * Parameters:
 *   dev - Device identifier (from stat.st_dev)
 *   ub  - Pointer to ustat structure to fill
 *
 * Returns:
 *   0 on success, -1 on failure with errno set
 */
int ustat(long dev, struct ustat *ub)
{
    if (dev) {
        __aligned struct InfoData info;
        BPTR ifp = MKBADDR(&info);

        if (SendPacket((struct MsgPort *)dev, ACTION_DISK_INFO, (LONG *)&ifp, 1) != DOSFALSE) {
            struct DeviceList *dlist;
            LONG nblocks;

            /* Calculate free blocks and free space in KB */
            nblocks = info.id_NumBlocks - info.id_NumBlocksUsed;
            ub->f_tinode = nblocks;
            ub->f_tfree = nblocks * info.id_BytesPerBlock / 1024;
            
            /* Initialize volume name and pack fields */
            ub->f_fname[0] = '\0';
            ub->f_fpack[0] = '\0';

            /* Get volume name from device list */
            dlist = BADDR(info.id_VolumeNode);
            if (dlist) {
                UBYTE *name;
                int len;

                /* Convert BCPL string to C string */
                name = BADDR(dlist->dl_Name);
                if (name) {
                    len = *name++;
                    if (len > 255) len = 255; /* Ensure we don't overflow */
                    strncpy(ub->f_fname, (char *)name, len);
                    ub->f_fname[len] = '\0';
                }
            }
            return 0;
        }
        errno = ENXIO;
    } else {
        errno = EINVAL;
    }
    return -1;
}
