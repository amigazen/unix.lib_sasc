
#ifndef	SYSVIPC_H__
#define	SYSVIPC_H__

#include <stdint.h>
#include <sys/types.h>

/* Error codes returned. 
 *
 * OBS: This implementation does not return -1 on error, but a specific error code
 * with a value < 0. For use in a C-library, a simple wrapper can be written.
 */

#define	SV_EPERM		(-1)
#define	SV_ENOENT	(-2)
#define	SV_EINTR		(-4)
#define	SV_E2BIG		(-7)
#define	SV_ENOMEM	(-12)
#define	SV_EACCESS	(-13)
#define	SV_EFAULT	(-14)
#define	SV_EEXIST	(-17)
#define	SV_EINVAL	(-22)
#define	SV_ENOSPC	(-28)
#define	SV_EAGAIN	(-35)
#define	SV_EMSGSIZE	(-40)
#define	SV_EIDRM		(-82)
#define	SV_ENOMSG	(-83)

/* Common IPC defined. */
typedef uint32_t key_t;

#define	IPC_CREAT	001000
#define	IPC_EXCL		002000
#define	IPC_NOWAIT	004000
#define	IPC_NOERROR	010000

#define	IPC_RMID		7
#define	IPC_SET		2
#define	IPC_STAT		3

#define	IPC_PRIVATE	((key_t)0x80000000L)

struct ipc_perm {
	uid_t		uid;	/* Owner's UID.  Matches UID/GID types in dos/dos.h */
	gid_t		gid;	/* Owner's GID */
	uid_t		cuid;	/* Creator's UID */
	gid_t		cgid;	/* Creator's GID */
	mode_t	mode;	/* Protection flags. */
	uint32_t	seq;	/* Sequence no. */
	key_t		key;	/* IPC key */
};

extern key_t ftok(const char *path,const int id);

/* Shared memory */

#define	SHM_R		0400
#define	SHM_W		0200

#define	SHMLBA		0x10000	/* Replace with a call to determine pagesize? */
#define	SHM_RDONLY	(1L<<0)
#define	SHM_RND		(1L<<0)

struct shmid_ds {
	struct ipc_perm	shm_perm;
	size_t				shm_segsz;
	void					*shm_amp;
	int					shm_nattach;
	uint32_t				flags;
	time_t				shm_atime;	/* attach time. */
	time_t				shm_dtime;	/* detach time. */
	time_t				shm_ctime;	/* control time. */
};

extern void *shmat(int shmid,const void *prefadds,int flags);
extern int shmdt(void *shmaddr);
extern int shmget(key_t key,size_t size,int flags);
extern int shmctl(int shmid,int cmd,struct shmid_ds *cbuf);

/* Message Queues */

#define	MSG_R		0400
#define	MSG_W		0200

typedef	uint32_t	msglen_t;
typedef	uint32_t	msgqnum_t;

struct msgsnap_head {
	size_t	msgsnap_size;
	size_t	msgsnap_nmsg;
};

struct msgsnap_mhead {
	size_t	msgsnap_mlen;
	long int	msgsnap_mtype;
};

struct msgqid_ds {
	struct ipc_perm	msg_perm;
	void			*msg_first;
	void			*msg_last;
	msglen_t		msg_cbytes;
	msgqnum_t	msg_qnum;
	msglen_t		msg_qbytes;
	pid_t			msg_lspid;
	pid_t			msg_lrpid;
	time_t		msg_stime;
	time_t		msg_rtime;
	time_t		msg_ctime;
	void			*Lock;
	void			*WList,*RList;
};

extern int msgget(key_t key,int flags);
extern int msgsnd(int qid,const void *msg,size_t mlen,int flags);
extern ssize_t msgrcv(int qid,void *msg,size_t mlen,long int mtype,int flags);
extern int msgctl(int qid,int cmd,struct msgqid_ds *buf);
extern int msgsnap(int qid,void *qbuf,size_t blen,long mtype);
extern int msgids(int *buf,unsigned int blen,unsigned int *qcnt);


/* Semaphores */

#define	SEM_R		0400
#define	SEM_W		0200

struct sem {
	int	semval;
	pid_t	sempid;
	uint16_t	semncnt;
	uint16_t	semzcnt;
};

struct semid_ds {
	struct ipc_perm sem_perm;
	struct sem *sem_base;
	int		sem_nsems;
	time_t	sem_otime;
	time_t	sem_ctime;
	int		sem_binary;
	void	*Lock;
};

/* Semaphore operations */
extern int semget(key_t key, int nsems, int flags);
extern int semop(int semid, const struct sembuf *ops, int nops);
extern int semtimedop(int semid, const struct sembuf *ops, int nops, const struct timespec *timeout);
extern int semctl(int semid, int semnum, int cmd, union semun arg);
extern int semids(int *buf, unsigned int nids, unsigned int *idcnt);

/* Union for semctl */
union semun {
	int val;
	struct semid_ds *buf;
	uint16_t *array;
};

/* Semaphore operation structure */
struct sembuf {
	uint16_t	sem_num;
	int16_t		sem_op;
	int16_t		sem_flg;
};

/* Semaphore control commands */
#define GETALL	0
#define SETALL	1
#define GETNCNT	2
#define GETPID	3
#define GETVAL	4
#define GETZCNT	5
#define SETVAL	6

/* Semaphore operation flags */
#define SEM_UNDO	0x1000

#endif

/* vi: set ts=3: */
