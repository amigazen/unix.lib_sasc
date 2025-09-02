
/*
 * chmod.c - change file permissions (POSIX compliant)
 *
 * This function changes the permissions of the file named by path to the
 * mode given by mode. This implementation provides enhanced AmigaOS
 * integration using message passing for better filesystem support.
 *
 * POSIX.1-2001, POSIX.1-2008
 */

#include "amiga.h"
#include <sys/stat.h>
#include <errno.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dostags.h>

/* Define ACTION constants locally since they're not available in unix.lib yet */
#define ACTION_SET_PERMS 2003

/*
 * SetPerms() - Enhanced permission setting using AmigaOS message passing
 *
 * This function provides better filesystem integration than the basic
 * SetProtection() call by using AmigaOS message passing to communicate
 * directly with the filesystem.
 */
static int SetPerms(const char *name, unsigned long mode)
{
    int err = 0;
    struct MsgPort *msgport;
    struct MsgPort *replyport;
    struct StandardPacket *packet;
    BPTR dlock;
    char buf[512];

    /* Get the device port for the file */
    msgport = (struct MsgPort *)DeviceProc(name);
    if (msgport == NULL) {
        errno = ENOENT;
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

    /* Prepare BCPL string */
    strcpy(buf + 1, name);
    buf[0] = strlen(buf + 1);

    /* Set up packet */
    packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link = &(packet->sp_Msg);
    packet->sp_Pkt.dp_Port = replyport;
    packet->sp_Pkt.dp_Type = ACTION_SET_PERMS;
    packet->sp_Pkt.dp_Arg1 = dlock;
    packet->sp_Pkt.dp_Arg2 = CTOB(buf);
    packet->sp_Pkt.dp_Arg3 = mode;

    /* Send message and wait for reply */
    PutMsg(msgport, (struct Message *)packet);
    WaitPort(replyport);
    GetMsg(replyport);

    /* Check result */
    if (packet->sp_Pkt.dp_Res1 == DOSFALSE) {
        SetIoErr(packet->sp_Pkt.dp_Res2);
        err = -1;
    }

    /* Cleanup */
    FreeMem(packet, sizeof(struct StandardPacket));
    DeletePort(replyport);

    return err;
}

/*
 * chmod() - change file permissions (POSIX compliant)
 *
 * The chmod() function changes the permissions of the file named by path
 * to the mode given by mode.
 *
 * Parameters:
 *   path: pathname of the file whose permissions to change
 *   mode: new permission mode (combination of S_IRWXU, S_IRWXG, S_IRWXO, etc.)
 *
 * Returns: 0 on success, -1 on error with errno set
 */
int chmod(const char *path, mode_t mode)
{
    int result;

    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (path == NULL) {
        errno = EFAULT;
        return -1;
    }

    /* Try enhanced SetPerms() first */
    result = SetPerms(path, (unsigned long)mode);
    if (result == 0) {
        return 0;  /* Success */
    }

    /* Fallback to basic SetProtection() for compatibility */
    if (SetProtection((char *)path, _make_protection(mode))) {
        return 0;  /* Success */
    }
    
    /* Handle error */
    _seterr();
    return -1;
}
