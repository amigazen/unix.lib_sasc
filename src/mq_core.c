/*
 * POSIX Message Queue Core Implementation for Amiga
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 */

#include "amiga.h"
#include "include/internal/mq_internal.h"

/* Global variables */
struct List g_MQQueues;
struct SignalSemaphore g_MQSemaphore;

/* Initialize message queue system */
void _mq_init(void) {
    NewList(&g_MQQueues);
    InitSemaphore(&g_MQSemaphore);
}

/* Cleanup message queue system */
void _mq_cleanup(void) {
    MQQueue *queue;
    
    ObtainSemaphore(&g_MQSemaphore);
    
    /* Close all remaining queues */
    while ((queue = (MQQueue *)RemHead(&g_MQQueues)) != NULL) {
        _mq_destroy_queue(queue);
    }
    
    ReleaseSemaphore(&g_MQSemaphore);
}

/* Create or open a message queue */
mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr) {
    MQQueue *queue;
    MQHandle *handle;
    char queue_name[MQ_NAME_MAX + 1];
    int create_new = 0;
    
    if (!name || strlen(name) >= MQ_NAME_MAX) {
        errno = EINVAL;
        return MQ_FAILED;
    }
    
    strcpy(queue_name, name);
    
    ObtainSemaphore(&g_MQSemaphore);
    
    /* Look for existing queue */
    queue = (MQQueue *)FindName(&g_MQQueues, queue_name);
    
    if (queue == NULL) {
        /* Queue doesn't exist */
        if (!(oflag & O_CREAT)) {
            ReleaseSemaphore(&g_MQSemaphore);
            errno = ENOENT;
            return MQ_FAILED;
        }
        
        /* Create new queue */
        queue = _mq_create_queue(queue_name, attr);
        if (!queue) {
            ReleaseSemaphore(&g_MQSemaphore);
            return MQ_FAILED;
        }
        
        create_new = 1;
    } else {
        /* Queue exists */
        if ((oflag & O_CREAT) && (oflag & O_EXCL)) {
            ReleaseSemaphore(&g_MQSemaphore);
            errno = EEXIST;
            return MQ_FAILED;
        }
    }
    
    /* Create handle for this open */
    handle = _mq_create_handle(queue, oflag);
    if (!handle) {
        if (create_new) {
            _mq_destroy_queue(queue);
        }
        ReleaseSemaphore(&g_MQSemaphore);
        return MQ_FAILED;
    }
    
    ReleaseSemaphore(&g_MQSemaphore);
    
    return (mqd_t)handle;
}

/* Close a message queue */
int mq_close(mqd_t mqdes) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    
    if (!handle) {
        errno = EBADF;
        return -1;
    }
    
    ObtainSemaphore(&g_MQSemaphore);
    
    queue = handle->mh_Queue;
    
    /* Remove handle from queue's handle list */
    Remove(&handle->mh_Node);
    
    /* Free handle */
    FreeMem(handle, sizeof(MQHandle));
    
    /* If no more handles, destroy queue */
    if (IsListEmpty(&queue->mq_OpenHandles)) {
        _mq_destroy_queue(queue);
    }
    
    ReleaseSemaphore(&g_MQSemaphore);
    
    return 0;
}

/* Send a message */
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    MQMessage *message;
    
    if (!handle || !msg_ptr) {
        errno = EINVAL;
        return -1;
    }
    
    if (msg_prio >= MQ_PRIO_MAX) {
        errno = EINVAL;
        return -1;
    }
    
    queue = handle->mh_Queue;
    
    ObtainSemaphore(&queue->mq_Semaphore);
    
    /* Check queue limits */
    if (queue->mq_QueueCounts[msg_prio] >= queue->mq_Attributes.mq_maxmsg) {
        if (handle->mh_Flags & O_NONBLOCK) {
            ReleaseSemaphore(&queue->mq_Semaphore);
            errno = EAGAIN;
            return -1;
        }
        
        /* Block until space available */
        while (queue->mq_QueueCounts[msg_prio] >= queue->mq_Attributes.mq_maxmsg) {
            ReleaseSemaphore(&queue->mq_Semaphore);
            Wait(1L << queue->mq_Port->mp_SigBit);
            ObtainSemaphore(&queue->mq_Semaphore);
        }
    }
    
    /* Check message size */
    if (msg_len > queue->mq_Attributes.mq_msgsize) {
        ReleaseSemaphore(&queue->mq_Semaphore);
        errno = EMSGSIZE;
        return -1;
    }
    
    /* Create message */
    message = _mq_create_message(msg_ptr, msg_len, msg_prio);
    if (!message) {
        ReleaseSemaphore(&queue->mq_Semaphore);
        errno = ENOMEM;
        return -1;
    }
    
    /* Add to priority queue */
    AddTail(&queue->mq_PriorityQueues[msg_prio], &message->mm_Msg.mn_Node);
    queue->mq_QueueCounts[msg_prio]++;
    queue->mq_Attributes.mq_curmsgs++;
    
    /* Notify waiting readers */
    _mq_notify_readers(queue);
    
    ReleaseSemaphore(&queue->mq_Semaphore);
    
    return 0;
}

/* Receive a message */
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    MQMessage *message;
    ssize_t result = 0;
    
    if (!handle || !msg_ptr) {
        errno = EINVAL;
        return -1;
    }
    
    queue = handle->mh_Queue;
    
    ObtainSemaphore(&queue->mq_Semaphore);
    
    /* Find highest priority message */
    message = _mq_find_highest_priority_message(queue);
    
    if (!message) {
        if (handle->mh_Flags & O_NONBLOCK) {
            ReleaseSemaphore(&queue->mq_Semaphore);
            errno = EAGAIN;
            return -1;
        }
        
        /* Block until message available */
        while (!(message = _mq_find_highest_priority_message(queue))) {
            ReleaseSemaphore(&queue->mq_Semaphore);
            Wait(1L << queue->mq_Port->mp_SigBit);
            ObtainSemaphore(&queue->mq_Semaphore);
        }
    }
    
    /* Process the message */
    result = _mq_process_received_message(queue, message, msg_ptr, msg_len, msg_prio);
    
    ReleaseSemaphore(&queue->mq_Semaphore);
    
    return result;
}

/* Get message queue attributes */
int mq_getattr(mqd_t mqdes, struct mq_attr *attr) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    
    if (!handle || !attr) {
        errno = EINVAL;
        return -1;
    }
    
    queue = handle->mh_Queue;
    
    ObtainSemaphore(&queue->mq_Semaphore);
    *attr = queue->mq_Attributes;
    ReleaseSemaphore(&queue->mq_Semaphore);
    
    return 0;
}

/* Set message queue attributes */
int mq_setattr(mqd_t mqdes, const struct mq_attr *newattr, struct mq_attr *oldattr) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    
    if (!handle || !newattr) {
        errno = EINVAL;
        return -1;
    }
    
    queue = handle->mh_Queue;
    
    ObtainSemaphore(&queue->mq_Semaphore);
    
    if (oldattr) {
        *oldattr = queue->mq_Attributes;
    }
    
    /* Only mq_flags can be changed */
    queue->mq_Attributes.mq_flags = newattr->mq_flags;
    
    ReleaseSemaphore(&queue->mq_Semaphore);
    
    return 0;
}

/* Unlink a message queue */
int mq_unlink(const char *name) {
    MQQueue *queue;
    char queue_name[MQ_NAME_MAX + 1];
    
    if (!name || strlen(name) >= MQ_NAME_MAX) {
        errno = EINVAL;
        return -1;
    }
    
    strcpy(queue_name, name);
    
    ObtainSemaphore(&g_MQSemaphore);
    
    queue = (MQQueue *)FindName(&g_MQQueues, queue_name);
    if (!queue) {
        ReleaseSemaphore(&g_MQSemaphore);
        errno = ENOENT;
        return -1;
    }
    
    /* Mark queue for destruction */
    queue->mq_Node.ln_Type = NT_UNKNOWN; /* Mark as unlinked */
    
    /* If no open handles, destroy immediately */
    if (IsListEmpty(&queue->mq_OpenHandles)) {
        _mq_destroy_queue(queue);
    }
    
    ReleaseSemaphore(&g_MQSemaphore);
    
    return 0;
}
