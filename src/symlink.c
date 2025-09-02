/*
 * symlink.c - create symbolic link (POSIX compliant)
 *
 * This function creates a symbolic link named to which contains the string from.
 * This implementation provides enhanced AmigaOS integration using message passing
 * for better filesystem support.
 *
 * POSIX.1-2001, POSIX.1-2008
 */

#include "amiga.h"
#include <errno.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dostags.h>

/* Define ACTION constants locally since they're not available in unix.lib2 */
#define ACTION_MAKE_LINK 2004

/*
 * PMakeLink() - Enhanced link creation using AmigaOS message passing
 *
 * This function provides better filesystem integration than the basic
 * MakeLink() call by using AmigaOS message passing to communicate
 * directly with the filesystem.
 *
 * Parameters:
 *   dname: destination pathname for the new link
 *   sname: source pathname to link to
 *   type: 0 for hard link, 1 for symbolic link
 *
 * Returns: 0 on success, -1 on error
 */
static int PMakeLink(const char *dname, const char *sname, unsigned long type)
{
    int err = 0;
    struct MsgPort *msgport;
    struct MsgPort *replyport;
    struct StandardPacket *packet;
    BPTR dlock;
    char buf[1024];
    char buf2[1024];

    /* Get the device port for the destination */
    msgport = (struct MsgPort *)DeviceProc(dname);
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

    /* Prepare BCPL string for source */
    strcpy(buf + 1, sname);
    buf[0] = strlen(sname);

    /* Set up packet */
    packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link = &(packet->sp_Msg);
    packet->sp_Pkt.dp_Port = replyport;
    packet->sp_Pkt.dp_Type = ACTION_MAKE_LINK;
    packet->sp_Pkt.dp_Arg1 = dlock;
    packet->sp_Pkt.dp_Arg2 = CTOB(buf);

    /* For symbolic links, use string pointer instead of file lock */
    strcpy(buf2 + 1, dname);
    buf2[0] = strlen(dname);
    packet->sp_Pkt.dp_Arg3 = CTOB(buf2);
    packet->sp_Pkt.dp_Arg4 = type;  /* 1 = symbolic link */

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
 * symlink() - create symbolic link (POSIX compliant)
 *
 * The symlink() function creates a symbolic link named to which contains
 * the string from.
 *
 * Parameters:
 *   from: contents of the symbolic link (target path)
 *   to: pathname of the symbolic link to create
 *
 * Returns: 0 on success, -1 on error with errno set
 */
int symlink(const char *from, const char *to)
{
    int result;

    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (from == NULL || to == NULL) {
        errno = EFAULT;
        return -1;
    }

    /* Try enhanced PMakeLink() first */
    result = PMakeLink(to, from, 1);  /* 1 = symbolic link */
    if (result == 0) {
        return 0;  /* Success */
    }

    /* Fallback to basic MakeLink() for compatibility */
    if (MakeLink((char *)to, (long)from, LINK_SOFT)) {
        return 0;  /* Success */
    }
    
    /* Handle error */
    _seterr();
    return -1;
}
