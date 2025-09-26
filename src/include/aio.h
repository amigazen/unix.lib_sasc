/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * aio.h - Asynchronous I/O operations
 * 
 * This header provides POSIX asynchronous I/O operations for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _AIO_H
#define _AIO_H 1

#include <sys/types.h>
#include <signal.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AIO control block structure */
struct aiocb {
    int aio_fildes;           /* File descriptor */
    off_t aio_offset;         /* File offset */
    volatile void *aio_buf;   /* Buffer location */
    size_t aio_nbytes;        /* Length of transfer */
    int aio_reqprio;          /* Request priority offset */
    struct sigevent aio_sigevent; /* Signal number and value */
    int aio_lio_opcode;       /* Operation to be performed */
};

/* AIO operation codes */
#define LIO_NOP    0   /* No operation */
#define LIO_READ   1   /* Read operation */
#define LIO_WRITE  2   /* Write operation */

/* AIO list operations */
#define LIO_NOWAIT 0   /* Don't wait for completion */
#define LIO_WAIT   1   /* Wait for completion */

/* AIO error codes */
#define AIO_ALLDONE    1   /* All operations completed */
#define AIO_CANCELED   2   /* Operation was canceled */
#define AIO_NOTCANCELED 3  /* Operation could not be canceled */

/* Function prototypes */
int aio_read(struct aiocb *aiocbp);
int aio_write(struct aiocb *aiocbp);
int aio_fsync(int op, struct aiocb *aiocbp);
int aio_error(const struct aiocb *aiocbp);
ssize_t aio_return(struct aiocb *aiocbp);
int aio_suspend(const struct aiocb *const list[], int nent,
                const struct timespec *timeout);
int aio_cancel(int fildes, struct aiocb *aiocbp);
int lio_listio(int mode, struct aiocb *const list[], int nent,
               struct sigevent *sig);

#ifdef __cplusplus
}
#endif

#endif /* _AIO_H */
