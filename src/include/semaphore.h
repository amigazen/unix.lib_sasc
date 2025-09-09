/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on libsem implementation by Diego Casorran (Public Domain)
 *
 * POSIX Semaphores for Amiga
 */

#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H 1

#include "amigapthread.h"
#include <exec/types.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <dos/dos.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

/* POSIX semaphore constants */
#define SEM_FAILED      ((sem_t *) 0L)
#define SEM_VALUE_MAX   (~0U)

/* Internal semaphore structure size - must match psem_t */
#define __SIZEOF_SEM_T  130

/* Padding for alignment */
#if defined(__SIZEOF_SEM_T_PAD)
# if __SIZEOF_SEM_T_PAD == 0
#  undef __SIZEOF_SEM_T_PAD
#  define __SIZEOF_SEM_T_PAD __SIZEOF_SEM_T
# elif __SIZEOF_SEM_T_PAD < __SIZEOF_SEM_T
#  undef __SIZEOF_SEM_T_PAD
# endif
#endif
#ifndef __SIZEOF_SEM_T_PAD
# define __SIZEOF_SEM_T_PAD 256
#endif

/* POSIX semaphore type - opaque to users */
typedef union {
    char __size[__SIZEOF_SEM_T];
    char __pad[__SIZEOF_SEM_T_PAD];
} sem_t;

/* Internal psem structure - matches libsem implementation */
#define _PSEM_NAME              "POSIX Semaphore [%s]"
#define _PSEM_NAME_MAXLENGTH    63
#define _PSEM_ERASEDBIT         0x7f
#define _SSEM(PSEM)             ((struct SignalSemaphore *)(PSEM))

#define _PSEM_INVALID(PSEM) \
    (((PSEM) == NULL) ? TRUE : ((((psem_t *)PSEM)->magic == _PSEM_MAGIC) ? FALSE \
        : ((((psem_t *)PSEM)->name[0] == _PSEM_ERASEDBIT) ? abort(),TRUE:TRUE)))

/* Internal psem structure */
typedef struct {
    struct SignalSemaphore ssem;
    
    unsigned long magic;
    #define __PSEM_MAGIC    0x53454D00
    
    unsigned char name[_PSEM_NAME_MAXLENGTH+1];
    
    unsigned int flags;
    unsigned int value;
    unsigned int semid;
    unsigned int owner;
    
} psem_t;

/* Semaphore node for tracking */
typedef struct {
    struct MinNode node;
    psem_t *sem;
} psem_node_t;

/* Semaphore flags */
enum {
    SEMF_SHARED  = (1L << 15), /* semaphore is shared type */
    SEMF_UNNAMED = (1L << 16), /* sem_init()'s semaphore */
    SEMF_EXPUNGE = (1L << 23), /* sem_unlink()'ed semaphore */
    SEMF_OPENDUP = (1L << 24), /* sem_open() not owned */
};

/* Global variables */
extern const unsigned long _PSEM_MAGIC;
extern struct MinList *__psem_list;

/* Internal function declarations */
void psem_list_add(psem_t *sem);
psem_t *psem_init(const char *name, int shared, unsigned int value);
void psem_destroy(psem_t *sem);
char *psem_name(const char *name, char *out, unsigned len);
psem_t *sem2psem(sem_t *sem);

/* Internal helper functions */
int psem_trywait(psem_t *sem);
int psem_wait(psem_t *sem);

/* POSIX semaphore API */
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_timedwait(sem_t *sem, const struct timespec *abstime);

/* Named semaphore functions */
sem_t *sem_open(const char *name, int oflag, ...);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);

/* Process cleanup */
void __psem_list_destruct(void);

#endif /* _SEMAPHORE_H */