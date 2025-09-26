/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * sys/un.h - Unix domain sockets
 * 
 * This header provides Unix domain socket definitions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _SYS_UN_H
#define _SYS_UN_H 1

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Unix domain socket address structure */
struct sockaddr_un {
    sa_family_t sun_family;    /* Address family */
    char sun_path[108];        /* Socket pathname */
};

/* Function prototypes */
int socketpair(int domain, int type, int protocol, int socket_vector[2]);

/* Unix domain socket constants */
#define AF_UNIX     1   /* Unix domain sockets */
#define AF_LOCAL    1   /* Local domain sockets (alias for AF_UNIX) */
#define PF_UNIX     AF_UNIX
#define PF_LOCAL    AF_LOCAL

/* Socket pathname length */
#define SUN_LEN(ptr) ((size_t)(((struct sockaddr_un *)0)->sun_path) + strlen((ptr)->sun_path))

#ifdef __cplusplus
}
#endif

#endif /* _SYS_UN_H */
