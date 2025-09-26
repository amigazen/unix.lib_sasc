/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * stropts.h - Stream options
 * 
 * This header provides stream options for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _STROPTS_H
#define _STROPTS_H 1

#include <sys/types.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Stream options */
#define I_PUSH      1   /* Push module onto stream */
#define I_POP       2   /* Pop module from stream */
#define I_LOOK      3   /* Look at next message */
#define I_FLUSH     4   /* Flush all messages */
#define I_FLUSHBAND 5   /* Flush messages on priority band */
#define I_SETSIG    6   /* Set signal for SIGPOLL */
#define I_GETSIG    7   /* Get signal for SIGPOLL */
#define I_FIND      8   /* Look for module */
#define I_PEEK      9   /* Peek at next message */
#define I_SRDOPT    10  /* Set read options */
#define I_GRDOPT    11  /* Get read options */
#define I_NREAD     12  /* Get number of bytes to read */
#define I_FDINSERT  13  /* Insert file descriptor */
#define I_STR       14  /* Send/receive string */
#define I_SWROPT    15  /* Set write options */
#define I_GWROPT    16  /* Get write options */
#define I_SENDFD    17  /* Send file descriptor */
#define I_RECVFD    18  /* Receive file descriptor */
#define I_LIST      19  /* List all modules */
#define I_ATMARK    20  /* Check for out-of-band data */
#define I_CKBAND    21  /* Check for data on priority band */
#define I_GETBAND   22  /* Get priority band of next message */
#define I_CANPUT    23  /* Check if write is possible */
#define I_SETCLTIME 24  /* Set close time */
#define I_GETCLTIME 25  /* Get close time */
#define I_CANPUT    26  /* Check if write is possible */
#define I_SERROPT   27  /* Set error options */
#define I_GERROPT   28  /* Get error options */
#define I_LINK      29  /* Link streams */
#define I_UNLINK    30  /* Unlink streams */
#define I_PLINK     31  /* Link streams persistently */
#define I_PUNLINK   32  /* Unlink streams persistently */

/* Stream read options */
#define RNORM       0x0000  /* Normal read */
#define RMSGD       0x0001  /* Message discard */
#define RMSGN       0x0002  /* Message non-discard */
#define RPROTNORM   0x0004  /* Normal protocol */
#define RPROTDAT    0x0008  /* Data protocol */
#define RPROTDIS    0x0010  /* Disconnect protocol */
#define RPROT       0x001c  /* Protocol mask */
#define RS_HIPRI    0x0020  /* High priority read */
#define RS_ERR      0x0040  /* Error read */
#define RS_ERRMASK  0x007f  /* Error mask */

/* Stream write options */
#define WNORM       0x0000  /* Normal write */
#define WMSGD       0x0001  /* Message discard */
#define WMSGN       0x0002  /* Message non-discard */
#define WPROTNORM   0x0004  /* Normal protocol */
#define WPROTDAT    0x0008  /* Data protocol */
#define WPROTDIS    0x0010  /* Disconnect protocol */
#define WPROT       0x001c  /* Protocol mask */
#define WS_HIPRI    0x0020  /* High priority write */
#define WS_ERR      0x0040  /* Error write */
#define WS_ERRMASK  0x007f  /* Error mask */

/* Stream error options */
#define E_NORM      0x0000  /* Normal error handling */
#define E_AGAIN     0x0001  /* Try again on error */
#define E_IOCTL     0x0002  /* Error on ioctl */
#define E_SWAP      0x0004  /* Swap error codes */
#define E_CARRIER   0x0008  /* Error on carrier loss */

/* Stream band priorities */
#define BAND0       0       /* Normal priority band */
#define BAND1       1       /* High priority band */
#define BAND2       2       /* Higher priority band */
#define BAND3       3       /* Highest priority band */

/* Stream message types */
#define M_DATA      0x0001  /* Data message */
#define M_PROTO     0x0002  /* Protocol message */
#define M_EXPRESS   0x0004  /* Express message */
#define M_DELAY     0x0008  /* Delay message */
#define M_IOCTL     0x0010  /* I/O control message */
#define M_PASSFP    0x0020  /* Pass file pointer */
#define M_SIG       0x0040  /* Signal message */
#define M_ERROR     0x0080  /* Error message */
#define M_HANGUP    0x0100  /* Hangup message */
#define M_RSE       0x0200  /* Read side error */
#define M_WSE       0x0400  /* Write side error */
#define M_PCPROTO   0x0800  /* Protocol control message */
#define M_CTL       0x1000  /* Control message */
#define M_READ      0x2000  /* Read message */
#define M_WRITE     0x4000  /* Write message */
#define M_FLUSH     0x8000  /* Flush message */

/* Stream control structure */
struct strbuf {
    int maxlen;     /* Maximum length of buffer */
    int len;        /* Current length of data */
    char *buf;      /* Pointer to buffer */
};

/* Stream list structure */
struct str_list {
    int sl_nmods;   /* Number of modules */
    struct str_mlist *sl_modlist;  /* Module list */
};

/* Stream module list structure */
struct str_mlist {
    char l_name[FMNAMESZ+1];  /* Module name */
};

/* Function prototypes */
int getmsg(int fildes, struct strbuf *ctlptr, struct strbuf *dataptr, int *flagsp);
int putmsg(int fildes, const struct strbuf *ctlptr, const struct strbuf *dataptr, int flags);
int getpmsg(int fildes, struct strbuf *ctlptr, struct strbuf *dataptr, int *bandp, int *flagsp);
int putpmsg(int fildes, const struct strbuf *ctlptr, const struct strbuf *dataptr, int band, int flags);
int fattach(int fildes, const char *path);
int fdetach(const char *path);
int isastream(int fildes);
int ioctl(int fildes, int request, ...);

#ifdef __cplusplus
}
#endif

#endif /* _STROPTS_H */
