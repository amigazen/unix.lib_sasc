/*
 * Internal Message Queue Structures and Functions for Amiga
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 */

#ifndef _MQ_INTERNAL_H_
#define _MQ_INTERNAL_H_

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/ports.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <dos/dos.h>
#include <devices/timer.h>
#include <sys/mqueue.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* Internal message structure */
typedef struct mq_message {
    struct Message      mm_Msg;        /* Amiga message header */
    unsigned int        mm_Priority;   /* Message priority */
    size_t              mm_Length;     /* Message length */
    char                mm_Data[1];    /* Variable-length data (C89 compatible) */
} MQMessage;

/* Message queue control structure */
typedef struct mq_queue {
    struct Node         mq_Node;       /* For global queue list */
    char                mq_Name[MQ_NAME_MAX + 1];  /* Queue name */
    struct MsgPort     *mq_Port;       /* Amiga message port */
    struct List         mq_PriorityQueues[MQ_PRIO_MAX];  /* Priority queues */
    int                 mq_QueueCounts[MQ_PRIO_MAX];     /* Count per priority */
    struct mq_attr      mq_Attributes; /* Queue attributes */
    int                 mq_OpenCount;  /* Number of open handles */
    struct List         mq_OpenHandles; /* List of open handles */
    struct SignalSemaphore mq_Semaphore; /* Protection semaphore */
    struct sigevent     mq_Notification; /* Async notification */
    int                 mq_NotifyEnabled; /* Notification enabled flag */
    struct Task        *mq_NotifyTask; /* Task to notify */
    ULONG               mq_NotifySignal; /* Signal to send */
} MQQueue;

/* Open handle structure */
typedef struct mq_handle {
    struct Node         mh_Node;       /* For queue's handle list */
    MQQueue            *mh_Queue;      /* Associated queue */
    int                 mh_Flags;      /* Open flags */
    struct Task        *mh_Task;       /* Owning task */
} MQHandle;

/* Global queue management */
extern struct List      g_MQQueues;    /* Global queue list */
extern struct SignalSemaphore g_MQSemaphore; /* Global protection */

/* Internal function prototypes */
void _mq_init(void);
void _mq_cleanup(void);
MQQueue *_mq_create_queue(const char *name, struct mq_attr *attr);
void _mq_destroy_queue(MQQueue *queue);
MQMessage *_mq_create_message(const char *data, size_t length, unsigned int priority);
MQMessage *_mq_find_highest_priority_message(MQQueue *queue);
void _mq_notify_readers(MQQueue *queue);
MQHandle *_mq_create_handle(MQQueue *queue, int flags);
ssize_t _mq_process_received_message(MQQueue *queue, MQMessage *message, 
                                    char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
int _mq_set_timer(const struct timespec *abs_timeout, ULONG signal);

/* Amiga system function prototypes */
extern void GetSysTime(struct timeval *);
extern int SetTimer(ULONG, struct timeval *, ULONG);

#endif /* _MQ_INTERNAL_H_ */