#ifndef _POLL_H_
#define _POLL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int nfds_t;

struct pollfd {
    int   fd;         /* File descriptor to check. */
    short events;     /* Events of interest. */
    short revents;    /* Events that occurred. */
};

/* --- Event flags for `events` and `revents` --- */
#define POLLIN      0x0001  /* There is data to read. */
#define POLLPRI     0x0002  /* There is urgent data to read. */
#define POLLOUT     0x0004  /* Writing now will not block. */

/* --- Error flags for `revents` only --- */
#define POLLERR     0x0008  /* An error has occurred. */
#define POLLHUP     0x0010  /* The device has been disconnected. */
#define POLLNVAL    0x0020  /* Invalid file descriptor. */

/* --- C89-compliant function prototype --- */
int poll(struct pollfd fds[], nfds_t nfds, int timeout);

#ifdef __cplusplus
}
#endif

#endif /* !_POLL_H_ */
