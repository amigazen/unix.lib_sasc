/*
 * POSIX Message Queue Implementation for Amiga
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 */

#ifndef _SYS_MQUEUE_H_
#define _SYS_MQUEUE_H_

#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

/* timespec structure for C89 compatibility */
#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
#endif

/* Message queue descriptor */
typedef struct mq_descriptor *mqd_t;

/* Message queue attributes */
struct mq_attr {
    long mq_flags;       /* Message queue flags */
    long mq_maxmsg;      /* Maximum number of messages */
    long mq_msgsize;     /* Maximum message size */
    long mq_curmsgs;     /* Number of messages currently queued */
};

/* POSIX constants */
#define MQ_PRIO_MAX      32
#define MQ_NAME_MAX      255
#define MQ_FAILED        ((mqd_t)-1)

/* Signal value union */
union sigval {
    int sival_int;
    void *sival_ptr;
};

/* Signal event constants */
#define SIGEV_NONE       1
#define SIGEV_SIGNAL     2
#define SIGEV_THREAD     3

/* Signal event structure */
struct sigevent {
    int sigev_notify;
    int sigev_signo;
    union sigval sigev_value;
};

/* POSIX function prototypes */
mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
int mq_close(mqd_t mqdes);
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
int mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, 
                   unsigned int *msg_prio, const struct timespec *abs_timeout);
int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, 
                unsigned int msg_prio, const struct timespec *abs_timeout);
int mq_notify(mqd_t mqdes, const struct sigevent *notification);
int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
int mq_setattr(mqd_t mqdes, const struct mq_attr *newattr, struct mq_attr *oldattr);
int mq_unlink(const char *name);

#endif /* _SYS_MQUEUE_H_ */
