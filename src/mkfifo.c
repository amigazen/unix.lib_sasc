/*
 * mkfifo.c - create named pipe (POSIX compliant)
 * mknod.c - create special files (POSIX compliant)
 *
 * These functions create special files including named pipes (FIFOs)
 * and device files on AmigaOS.
 *
 * POSIX.1-2001, POSIX.1-2008
 */

#include "amiga.h"
#include <sys/stat.h>
#include <errno.h>
#include <proto/dos.h>
#include <dos/dostags.h>

/* AmigaOS file type constants */
#define ST_FILE     0       /* Regular file */
#define ST_CDEVICE  1       /* Character device */
#define ST_BDEVICE  2       /* Block device */
#define ST_PIPEFILE 3       /* Named pipe (FIFO) */
#define ST_WHITEOUT 4       /* Whiteout file */

/* AmigaOS action constants */
#define ACTION_CREATE_OBJECT 2002

/* Internal function to create special files */
static int create_special_file(const char *path, int type, int device, int mode)
{
    struct MsgPort *msgport;
    struct MsgPort *replyport;
    struct StandardPacket *packet;
    BPTR dlock;
    char buf[512];
    int result = -1;

    /* Get the device port for the path */
    msgport = (struct MsgPort *)DeviceProc((char *)path);
    if (msgport == NULL) {
        errno = EOSERR;
        return -1;
    }

    /* Save current directory */
    dlock = CurrentDir(0);
    CurrentDir(dlock);

    /* Create reply port */
    replyport = (struct MsgPort *)CreatePort(NULL, 0);
    if (!replyport) {
        errno = ENOMEM;
        return -1;
    }

    /* Allocate packet */
    packet = (struct StandardPacket *)
             AllocMem(sizeof(struct StandardPacket), MEMF_CLEAR | MEMF_PUBLIC);
    if (packet == NULL) {
        DeletePort(replyport);
        errno = ENOMEM;
        return -1;
    }

    /* Prepare filename buffer */
    strcpy(buf + 1, path);
    buf[0] = strlen(buf + 1);

    /* Set up packet for CREATE_OBJECT action */
    packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link = &(packet->sp_Msg);
    packet->sp_Pkt.dp_Port = replyport;
    packet->sp_Pkt.dp_Type = ACTION_CREATE_OBJECT;
    packet->sp_Pkt.dp_Arg1 = dlock;
    packet->sp_Pkt.dp_Arg2 = CTOB(buf);
    packet->sp_Pkt.dp_Arg3 = type;
    packet->sp_Pkt.dp_Arg4 = device;

    /* Send message */
    PutMsg(msgport, (struct Message *)packet);

    /* Wait for reply */
    WaitPort(replyport);
    GetMsg(replyport);

    /* Check result */
    if (packet->sp_Pkt.dp_Res1 == DOSFALSE) {
        SetIoErr(packet->sp_Pkt.dp_Res2);
        errno = EOSERR;
        result = -1;
    } else {
        /* Set permissions if successful */
        packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
        packet->sp_Pkt.dp_Link = &(packet->sp_Msg);
        packet->sp_Pkt.dp_Port = replyport;
        packet->sp_Pkt.dp_Type = ACTION_SET_PERMS;
        packet->sp_Pkt.dp_Arg1 = dlock;
        packet->sp_Pkt.dp_Arg2 = CTOB(buf);
        packet->sp_Pkt.dp_Arg3 = mode;
        packet->sp_Pkt.dp_Arg4 = device;

        PutMsg(msgport, (struct Message *)packet);
        WaitPort(replyport);
        GetMsg(replyport);

        if (packet->sp_Pkt.dp_Res1 == DOSFALSE) {
            SetIoErr(packet->sp_Pkt.dp_Res2);
            errno = EOSERR;
            result = -1;
        } else {
            result = 0;
        }
    }

    /* Clean up */
    FreeMem(packet, sizeof(struct StandardPacket));
    DeletePort(replyport);

    return result;
}

/*
 * mkfifo() - create named pipe (FIFO)
 *
 * The mkfifo() function creates a new FIFO special file named path.
 * The file permission bits of the new FIFO are initialized from mode.
 *
 * Returns: 0 on success, -1 on error
 */
int mkfifo(const char *path, mode_t mode)
{
    if (path == NULL) {
        errno = EINVAL;
        return -1;
    }

    return create_special_file(path, ST_PIPEFILE, 0, mode & ~S_IFMT);
}

/*
 * mknod() - create special files
 *
 * The mknod() function creates a new file named path, with the file type
 * and permissions specified by mode and dev.
 *
 * Returns: 0 on success, -1 on error
 */
int mknod(const char *path, mode_t mode, dev_t dev)
{
    int type;

    if (path == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* Determine file type from mode */
    switch (mode & S_IFMT) {
        case S_IFCHR:
            type = ST_CDEVICE;
            break;
        case S_IFBLK:
            type = ST_BDEVICE;
            break;
        case S_IFREG:
            type = ST_FILE;
            break;
        case S_IFIFO:
            type = ST_PIPEFILE;
            break;
        default:
            errno = EINVAL;
            return -1;
    }

    return create_special_file(path, type, (int)dev, mode & ~S_IFMT);
}
