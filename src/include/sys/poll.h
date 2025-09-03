/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * sys/poll.h - POSIX poll implementation using select
 * 
 * This is a fallback poll implementation that uses select() internally.
 * Warning: a call to this poll() takes about 4K of stack space.
 * 
 * Based on work by Greg Parker (gparker-web@sealiesoftware.com)
 * This code is in the public domain and may be copied or modified without 
 * permission.
 */

#ifndef _FAKE_POLL_H
#define _FAKE_POLL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pollfd {
    int fd;                         /* file desc to poll */
    short events;                   /* events of interest on fd */
    short revents;                  /* events that occurred on fd */
} pollfd_t;

/* poll flags */
#define POLLIN  0x0001
#define POLLOUT 0x0004
#define POLLERR 0x0008

/* synonyms */
#define POLLNORM POLLIN
#define POLLPRI POLLIN
#define POLLRDNORM POLLIN
#define POLLRDBAND POLLIN
#define POLLWRNORM POLLOUT
#define POLLWRBAND POLLOUT

/* ignored */
#define POLLHUP 0x0010
#define POLLNVAL 0x0020

extern int poll(struct pollfd *pollSet, int pollCount, int pollTimeout);

#ifdef __cplusplus
}
#endif

#endif /* _FAKE_POLL_H */
