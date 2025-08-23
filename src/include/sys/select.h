#ifndef _SYS_SELECT_H_
#define _SYS_SELECT_H_

#include <sys/types.h>
#include <time.h> /* For struct timeval */

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of file descriptors in an fd_set. */
#define FD_SETSIZE      256

/*
 * fd_set is a bit array. We use an array of longs for portability.
 * The size is calculated to hold at least FD_SETSIZE bits.
 */
typedef struct {
    long fds_bits[(FD_SETSIZE + 31) / 32];
} fd_set;

/* --- Macros for manipulating fd_set --- */
#define FD_ZERO(p)      (memset((void *)(p), 0, sizeof(*(p))))
#define FD_SET(n, p)    ((p)->fds_bits[(n)/32] |= (1L << ((n) % 32)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/32] &= ~(1L << ((n) % 32)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/32] & (1L << ((n) % 32)))

/* --- C89-compliant function prototype --- */
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);

#ifdef __cplusplus
}
#endif

#endif /* !_SYS_SELECT_H_ */
