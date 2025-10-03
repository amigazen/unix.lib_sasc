/* sys/socket.h - Socket interface definitions */

#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <sys/types.h>

/* Socket address families */
#define AF_INET 2

/* Socket types */
#define SOCK_STREAM 1

/* Missing socket function prototypes */
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int close(int fd);

/* Missing network function prototypes */
struct hostent *gethostbyname(const char *name);

#endif /* _SYS_SOCKET_H */