#ifndef _SYS_POLL_H
#define _SYS_POLL_H

#ifdef HAVE_PSOCKETS

/* POSIX poll() support */
#include <sys/types.h>

/* Poll events */
#define POLLIN     0x0001  /* There is data to read */
#define POLLPRI    0x0002  /* There is urgent data to read */
#define POLLOUT    0x0004  /* Writing now will not block */
#define POLLERR    0x0008  /* Error condition */
#define POLLHUP    0x0010  /* Hung up */
#define POLLNVAL   0x0020  /* Invalid request: fd not open */

/* Poll file descriptor structure */
struct pollfd {
    int fd;         /* File descriptor */
    short events;   /* Events to look for */
    short revents;  /* Events that occurred */
};

/* Type for number of file descriptors */
typedef unsigned long nfds_t;

/* Function prototype */
extern int poll(struct pollfd fds[], nfds_t nfds, int timeout);

#endif /* HAVE_PSOCKETS */

#endif /* _SYS_POLL_H */
