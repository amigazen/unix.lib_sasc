/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * mqueue.h - Message queue
 * 
 * This header provides POSIX message queue functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _MQUEUE_H
#define _MQUEUE_H 1

#include <sys/types.h>
#include <signal.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Message queue limits */
#define MQ_PRIO_MAX 32
#define MQ_NAME_MAX 255

/* Message queue descriptor */
typedef struct mq_descriptor *mqd_t;

/* Message queue attributes */
struct mq_attr {
    long mq_flags;       /* Message queue flags */
    long mq_maxmsg;      /* Maximum number of messages */
    long mq_msgsize;     /* Maximum message size */
    long mq_curmsgs;     /* Number of messages currently queued */
};

/* Signal value union */
union sigval {
    int sival_int;
    void *sival_ptr;
};

/* Signal event structure */
struct sigevent {
    int sigev_notify;              /* Notification method */
    int sigev_signo;               /* Signal number */
    union sigval sigev_value;      /* Signal value */
    void (*sigev_notify_function)(union sigval); /* Notification function */
    struct pthread_attr_t *sigev_notify_attributes;     /* Notification attributes */
};

/* Function prototypes */
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

#ifdef __cplusplus
}
#endif

#endif /* _MQUEUE_H */
