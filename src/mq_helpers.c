/*
 * POSIX Message Queue Helper Functions for Amiga
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 */

#include "amiga.h"
#include "include/internal/mq_internal.h"

/* Create a new message queue */
MQQueue *_mq_create_queue(const char *name, struct mq_attr *attr) {
    MQQueue *queue;
    int i;
    
    queue = AllocMem(sizeof(MQQueue), MEMF_CLEAR | MEMF_PUBLIC);
    if (!queue) {
        return NULL;
    }
    
    /* Initialize queue structure */
    strcpy(queue->mq_Name, name);
    queue->mq_Node.ln_Name = queue->mq_Name;
    queue->mq_Node.ln_Type = NT_UNKNOWN;
    
    /* Create message port */
    queue->mq_Port = CreateMsgPort();
    if (!queue->mq_Port) {
        FreeMem(queue, sizeof(MQQueue));
        return NULL;
    }
    
    /* Initialize priority queues */
    for (i = 0; i < MQ_PRIO_MAX; i++) {
        NewList(&queue->mq_PriorityQueues[i]);
        queue->mq_QueueCounts[i] = 0;
    }
    
    /* Set default attributes */
    if (attr) {
        queue->mq_Attributes = *attr;
    } else {
        queue->mq_Attributes.mq_flags = 0;
        queue->mq_Attributes.mq_maxmsg = 10;
        queue->mq_Attributes.mq_msgsize = 8192;
        queue->mq_Attributes.mq_curmsgs = 0;
    }
    
    /* Initialize semaphore */
    InitSemaphore(&queue->mq_Semaphore);
    
    /* Initialize handle list */
    NewList(&queue->mq_OpenHandles);
    
    /* Add to global queue list */
    AddTail(&g_MQQueues, &queue->mq_Node);
    
    return queue;
}

/* Destroy a message queue */
void _mq_destroy_queue(MQQueue *queue) {
    MQMessage *message;
    int i;
    
    if (!queue) return;
    
    /* Remove from global list */
    Remove(&queue->mq_Node);
    
    /* Free all messages */
    for (i = 0; i < MQ_PRIO_MAX; i++) {
        while ((message = (MQMessage *)RemHead(&queue->mq_PriorityQueues[i])) != NULL) {
            FreeMem(message, sizeof(MQMessage) + message->mm_Length);
        }
    }
    
    /* Delete message port */
    if (queue->mq_Port) {
        DeleteMsgPort(queue->mq_Port);
    }
    
    /* Free queue structure */
    FreeMem(queue, sizeof(MQQueue));
}

/* Create a message */
MQMessage *_mq_create_message(const char *data, size_t length, unsigned int priority) {
    MQMessage *message;
    
    message = AllocMem(sizeof(MQMessage) + length, MEMF_CLEAR | MEMF_PUBLIC);
    if (!message) {
        return NULL;
    }
    
    message->mm_Msg.mn_Node.ln_Type = NT_UNKNOWN;
    message->mm_Priority = priority;
    message->mm_Length = length;
    
    if (data && length > 0) {
        memcpy((void *)message->mm_Data, (const void *)data, length);
    }
    
    return message;
}

/* Find highest priority message */
MQMessage *_mq_find_highest_priority_message(MQQueue *queue) {
    MQMessage *message;
    int i;
    
    /* Search from highest priority down */
    for (i = MQ_PRIO_MAX - 1; i >= 0; i--) {
        if (!IsListEmpty(&queue->mq_PriorityQueues[i])) {
            message = (MQMessage *)queue->mq_PriorityQueues[i].lh_Head;
            return message;
        }
    }
    
    return NULL;
}

/* Notify waiting readers */
void _mq_notify_readers(MQQueue *queue) {
    /* Signal the message port to wake up waiting readers */
    Signal((struct Task *)queue->mq_Port, 1L << queue->mq_Port->mp_SigBit);
    
    /* Handle async notification */
    if (queue->mq_NotifyEnabled && queue->mq_NotifyTask) {
        Signal(queue->mq_NotifyTask, queue->mq_NotifySignal);
    }
}

/* Create a handle */
MQHandle *_mq_create_handle(MQQueue *queue, int flags) {
    MQHandle *handle;
    
    handle = AllocMem(sizeof(MQHandle), MEMF_CLEAR | MEMF_PUBLIC);
    if (!handle) {
        return NULL;
    }
    
    handle->mh_Queue = queue;
    handle->mh_Flags = flags;
    handle->mh_Task = FindTask(NULL);
    
    /* Add to queue's handle list */
    AddTail(&queue->mq_OpenHandles, &handle->mh_Node);
    queue->mq_OpenCount++;
    
    return handle;
}

/* Process a received message */
ssize_t _mq_process_received_message(MQQueue *queue, MQMessage *message, 
                                   char *msg_ptr, size_t msg_len, unsigned int *msg_prio) {
    int priority;
    ssize_t result = 0;
    
    if (!message) {
        return -1;
    }
    
    /* Get message priority */
    priority = message->mm_Priority;
    if (msg_prio) {
        *msg_prio = priority;
    }
    
    /* Check buffer size */
    if (msg_len < message->mm_Length) {
        errno = EMSGSIZE;
        return -1;
    }
    
    /* Copy message data */
    memcpy((void *)msg_ptr, (const void *)message->mm_Data, message->mm_Length);
    result = (ssize_t)message->mm_Length;
    
    /* Remove from queue */
    Remove(&message->mm_Msg.mn_Node);
    queue->mq_QueueCounts[priority]--;
    queue->mq_Attributes.mq_curmsgs--;
    
    /* Free message */
    FreeMem(message, sizeof(MQMessage) + message->mm_Length);
    
    return result;
}
