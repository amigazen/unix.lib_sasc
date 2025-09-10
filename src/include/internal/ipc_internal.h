/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on sysvipc implementation by Peter Bengtsson
 *
 * Internal header for System V IPC functions
 */

#ifndef IPC_INTERNAL_H__
#define IPC_INTERNAL_H__

#include <exec/types.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
#include <dos/dosextens.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <errno.h>

/* Internal constants */
#define DEF_SHMMAX	8388608	/* 8 MB */
#define DEF_QSIZEMAX	4194304	/* 4 MB */
#define VEXTEND		32

/* Internal semaphore structure */
struct sem_internal {
	int32_t		semval;
	uint32_t	sempid;
	uint16_t	semncnt;
	uint16_t	semzcnt;
	struct MinList	*zList;	/* List of processes waiting for zero */
	struct MinList	*nList;	/* List of processes waiting for non-zero */
};

/* Undo information for semaphore operations */
struct UndoInfo {
	struct UndoInfo *Next;
	int Id;	/* Semaphore id */
	int Adj[1];	/* Array of adjustment values - flexible array */
};

/* Generic IPC object header */
struct IPCGeneric {
	struct ipc_perm	perm;
};

/* IPC ID/Key mapping structure */
struct IPCIdKeyMap {
	int nobj;		/* Highest valid index + 1 */
	int vlen;		/* Vector length */
	int idx;		/* Total created objects */
	int nused;		/* Total used slots */
	struct IPCGeneric **objv;	/* Object vector */
	struct SignalSemaphore *Lock;	/* Protection semaphore */
};

/* Wait process structure */
struct WProc {
	struct MinNode	head;
	struct Task *T;
};

/* Timeout context for semaphore operations */
struct TimeoutCx {
	int semid;
	struct IPCIdKeyMap *km;
	struct Task *T;
	struct timespec to;
	volatile int Status;
};

/* Global IPC contexts */
extern struct IPCIdKeyMap shm_keymap;
extern struct IPCIdKeyMap msg_keymap;
extern struct IPCIdKeyMap sem_keymap;
extern struct UndoInfo *undo_list;

/* Internal function prototypes */
void ipc_map_init(struct IPCIdKeyMap *m);
void ipc_map_uninit(struct IPCIdKeyMap *m);
int get_ipc_key_id(struct IPCIdKeyMap *m, key_t key, int flags, 
                   void *(*construct)(int, int));
void *get_ipc_by_id(struct IPCIdKeyMap *m, int id);
void ipc_lock(struct IPCIdKeyMap *m);
void ipc_unlock(struct IPCIdKeyMap *m);
void *get_ipc_by_p(struct IPCIdKeyMap *m, int (*eq)(void *, void *), 
                   void *v, int *id);
void ipc_rm_id(struct IPCIdKeyMap *m, int id, 
               void (*destroy)(struct IPCGeneric *));
int ipc_ids(struct IPCIdKeyMap *m, int *idbuf, int idblen, int *idcnt);

/* Wait queue support */
void wake_list(struct MinList *ml);
struct WProc *request_wakeup(struct MinList *ml);
void wait_list(struct WProc *wp);
void cancel_wakeup(struct MinList *ml);

/* Shared memory internal functions */
struct shmid_ds *shm_construct(int key, int flags);
void shm_destroy(struct shmid_ds *si);

/* Message queue internal functions */
struct msgqid_ds *msg_construct(int key, int flags);
void msg_destroy(struct msgqid_ds *qi);

/* Semaphore internal functions */
struct semid_ds *sem_construct(int key, int flags);
void sem_destroy(struct semid_ds *si);
void sem_setup(struct sem_internal *s);
struct UndoInfo *get_undo(int id, int create);

/* Error handling */
void set_ipc_errno(int error);

/* System initialization */
void ipc_init_all(void);
void ipc_cleanup_all(void);
void shm_init(void);
void shm_cleanup(void);
void msg_init(void);
void msg_cleanup(void);
void sem_init(void);
void sem_cleanup(void);
void posix_shm_init(void);
void posix_shm_cleanup(void);
void mmap_init(void);
void mmap_cleanup(void);

#endif /* IPC_INTERNAL_H__ */
