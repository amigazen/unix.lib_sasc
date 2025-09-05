/*
 * POSIX Message Queue Advanced Features for Amiga
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 amigazen project
 */

#include "amiga.h"
#include "include/internal/mq_internal.h"

/* Timed receive */
int mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, 
                   unsigned int *msg_prio, const struct timespec *abs_timeout) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    MQMessage *message;
    ULONG timeout_signal;
    ULONG signals;
    int result = 0;
    
    if (!handle || !msg_ptr || !abs_timeout) {
        errno = EINVAL;
        return -1;
    }
    
    queue = handle->mh_Queue;
    
    /* Create timeout signal */
    timeout_signal = AllocSignal(-1);
    if (timeout_signal == -1) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Set up timer */
    if (_mq_set_timer(abs_timeout, timeout_signal) != 0) {
        FreeSignal(timeout_signal);
        return -1;
    }
    
    ObtainSemaphore(&queue->mq_Semaphore);
    
    /* Check for immediate message */
    message = _mq_find_highest_priority_message(queue);
    
    if (!message) {
        ReleaseSemaphore(&queue->mq_Semaphore);
        
        /* Wait for message or timeout */
        signals = Wait((1L << queue->mq_Port->mp_SigBit) | (1L << timeout_signal));
        
        if (signals & (1L << timeout_signal)) {
            FreeSignal(timeout_signal);
            errno = ETIMEDOUT;
            return -1;
        }
        
        ObtainSemaphore(&queue->mq_Semaphore);
        message = _mq_find_highest_priority_message(queue);
    }
    
    /* Process message */
    result = _mq_process_received_message(queue, message, msg_ptr, msg_len, msg_prio);
    
    ReleaseSemaphore(&queue->mq_Semaphore);
    FreeSignal(timeout_signal);
    
    return result;
}

/* Timed send */
int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, 
                unsigned int msg_prio, const struct timespec *abs_timeout) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    MQMessage *message;
    ULONG timeout_signal;
    ULONG signals;
    
    if (!handle || !msg_ptr) {
        errno = EINVAL;
        return -1;
    }
    
    if (msg_prio >= MQ_PRIO_MAX) {
        errno = EINVAL;
        return -1;
    }
    
    queue = handle->mh_Queue;
    
    /* Create timeout signal */
    timeout_signal = AllocSignal(-1);
    if (timeout_signal == -1) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Set up timer */
    if (_mq_set_timer(abs_timeout, timeout_signal) != 0) {
        FreeSignal(timeout_signal);
        return -1;
    }
    
    ObtainSemaphore(&queue->mq_Semaphore);
    
    /* Check queue limits */
    if (queue->mq_QueueCounts[msg_prio] >= queue->mq_Attributes.mq_maxmsg) {
        if (handle->mh_Flags & O_NONBLOCK) {
            ReleaseSemaphore(&queue->mq_Semaphore);
            FreeSignal(timeout_signal);
            errno = EAGAIN;
            return -1;
        }
        
        /* Block until space available or timeout */
        while (queue->mq_QueueCounts[msg_prio] >= queue->mq_Attributes.mq_maxmsg) {
            ReleaseSemaphore(&queue->mq_Semaphore);
            
            signals = Wait((1L << queue->mq_Port->mp_SigBit) | (1L << timeout_signal));
            
            if (signals & (1L << timeout_signal)) {
                FreeSignal(timeout_signal);
                errno = ETIMEDOUT;
                return -1;
            }
            
            ObtainSemaphore(&queue->mq_Semaphore);
        }
    }
    
    /* Check message size */
    if (msg_len > queue->mq_Attributes.mq_msgsize) {
        ReleaseSemaphore(&queue->mq_Semaphore);
        FreeSignal(timeout_signal);
        errno = EMSGSIZE;
        return -1;
    }
    
    /* Create message */
    message = _mq_create_message(msg_ptr, msg_len, msg_prio);
    if (!message) {
        ReleaseSemaphore(&queue->mq_Semaphore);
        FreeSignal(timeout_signal);
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
    FreeSignal(timeout_signal);
    
    return 0;
}

/* Async notification */
int mq_notify(mqd_t mqdes, const struct sigevent *notification) {
    MQHandle *handle = (MQHandle *)mqdes;
    MQQueue *queue;
    
    if (!handle) {
        errno = EBADF;
        return -1;
    }
    
    queue = handle->mh_Queue;
    
    ObtainSemaphore(&queue->mq_Semaphore);
    
    if (notification) {
        /* Enable notification */
        queue->mq_Notification = *notification;
        queue->mq_NotifyEnabled = 1;
        queue->mq_NotifyTask = FindTask(NULL);
        
        /* Set up signal */
        if (notification->sigev_notify == SIGEV_SIGNAL) {
            queue->mq_NotifySignal = 1L << notification->sigev_signo;
        }
    } else {
        /* Disable notification */
        queue->mq_NotifyEnabled = 0;
        queue->mq_NotifyTask = NULL;
    }
    
    ReleaseSemaphore(&queue->mq_Semaphore);
    
    return 0;
}

/* Set timer for timeout */
int _mq_set_timer(const struct timespec *abs_timeout, ULONG signal) {
    struct timeval current_time;
    struct timeval timeout_time;
    struct timeval delay;
    
    /* Get current time */
    GetSysTime(&current_time);
    
    /* Convert timespec to timeval */
    timeout_time.tv_sec = abs_timeout->tv_sec;
    timeout_time.tv_usec = abs_timeout->tv_nsec / 1000;
    
    /* Calculate delay */
    if (timeout_time.tv_sec < current_time.tv_sec ||
        (timeout_time.tv_sec == current_time.tv_sec && 
         timeout_time.tv_usec <= current_time.tv_usec)) {
        /* Already expired */
        Signal(FindTask(NULL), 1L << signal);
        return 0;
    }
    
    delay.tv_sec = timeout_time.tv_sec - current_time.tv_sec;
    delay.tv_usec = timeout_time.tv_usec - current_time.tv_usec;
    
    if (delay.tv_usec < 0) {
        delay.tv_sec--;
        delay.tv_usec += 1000000;
    }
    
    /* Set timer */
    return SetTimer(TR_ADDREQUEST, &delay, signal);
}
