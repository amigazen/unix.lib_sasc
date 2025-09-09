/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 * Based on sysvipc implementation by Peter Bengtsson
 *
 * System V Message Queue implementation
 */

#include "include/internal/ipc_internal.h"
#include "include/sys/ipc.h"
#include "debug.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <time.h>

/* Global message queue context */
struct IPCIdKeyMap msg_keymap;

#define DEFAULT_QSIZE	4096

/* Message structure */
struct Msg {
    struct Msg *Next;
    uint32_t Size;
    int32_t Type;
    uint8_t Data[1];  /* Flexible array for C89 compatibility */
};

/* Initialize message queue context */
void msg_init(void)
{
    ipc_map_init(&msg_keymap);
}

/* Cleanup message queue context */
void msg_cleanup(void)
{
    ipc_map_uninit(&msg_keymap);
}

/* Construct a message queue */
struct msgqid_ds *msg_construct(int key, int flags)
{
    /* Removed unused TagItem arrays */
    struct msgqid_ds *qi;
    
    qi = AllocVec(sizeof(struct msgqid_ds), MEMF_ANY | MEMF_CLEAR);
    if (qi) {
        qi->msg_perm.mode = flags & 0777;
        qi->msg_perm.key = key;
        qi->msg_first = NULL;
        qi->msg_last = NULL;
        qi->msg_cbytes = 0;
        qi->msg_qnum = 0;
        qi->msg_qbytes = DEFAULT_QSIZE;
        qi->msg_lspid = 0;
        qi->msg_lrpid = 0;
        qi->msg_stime = 0;
        qi->msg_rtime = 0;
        qi->msg_ctime = time(NULL);
        qi->Lock = AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC | MEMF_CLEAR);
        if (qi->Lock) {
            InitSemaphore(qi->Lock);
        }
        qi->WList = AllocMem(sizeof(struct MinList), MEMF_PUBLIC | MEMF_CLEAR);
        qi->RList = AllocMem(sizeof(struct MinList), MEMF_PUBLIC | MEMF_CLEAR);
    }
    return qi;
}

/* Destroy a message queue */
void msg_destroy(struct msgqid_ds *qi)
{
    struct Msg *m, *rm;
    
    if (qi) {
        m = (struct Msg *)qi->msg_first;
        while (m) {
            rm = m;
            m = m->Next;
            FreeVec(rm);
        }
        ReleaseSemaphore(qi->Lock);
        FreeMem(qi->Lock, sizeof(struct SignalSemaphore));
        FreeMem(qi->WList, sizeof(struct MinList));
        FreeMem(qi->RList, sizeof(struct MinList));
        FreeVec(qi);
    }
}

/* Get a message queue identifier */
int msgget(key_t key, int flags)
{
    int id = -1;
    
    ENTER();
    SHOWVALUE(key);
    SHOWVALUE(flags);
    
    ipc_lock(&msg_keymap);
    
    id = get_ipc_key_id(&msg_keymap, key, flags, 
                       (void *(*)(int, int))msg_construct);
    if (id < 0) {
        set_ipc_errno(-id);
        id = -1;
    }
    
    ipc_unlock(&msg_keymap);
    
    SHOWVALUE(id);
    return id;
}

/* Send a message */
int msgsnd(int qid, const void *msg, size_t mlen, int flags)
{
    struct msgqid_ds *qi;
    int ret = -1;
    int bleft;
    struct Msg *m;
    struct WProc *wp;
    
    ENTER();
    SHOWVALUE(qid);
    SHOWPOINTER(msg);
    SHOWVALUE(mlen);
    SHOWVALUE(flags);
    
    if (!msg) {
        set_ipc_errno(EFAULT);
        return -1;
    }
    
    ipc_lock(&msg_keymap);
    
    qi = (struct msgqid_ds *)get_ipc_by_id(&msg_keymap, qid);
    if (qi) {
        ObtainSemaphore(qi->Lock);
        
        bleft = qi->msg_qbytes - qi->msg_cbytes;
        if (mlen <= bleft) {
            m = AllocVec(mlen + sizeof(struct Msg), MEMF_ANY);
            if (m) {
                m->Size = mlen;
                m->Type = ((long *)msg)[0];  /* First word is message type */
                memcpy(m->Data, (char *)msg + sizeof(long), mlen - sizeof(long));
                m->Next = NULL;
                
                if (qi->msg_last) {
                    ((struct Msg *)qi->msg_last)->Next = m;
                } else {
                    qi->msg_first = m;
                }
                qi->msg_last = m;
                qi->msg_cbytes += mlen;
                qi->msg_qnum++;
                qi->msg_stime = time(NULL);
                qi->msg_lspid = (int)((ULONG)FindTask(0));  /* Use Task pointer as PID */
                
                /* Wake up waiting readers */
                wake_list((struct MinList *)qi->RList);
                
                ret = 0;
            } else {
                set_ipc_errno(ENOMEM);
            }
        } else {
            if (flags & IPC_NOWAIT) {
                set_ipc_errno(EAGAIN);
            } else {
                /* Wait for space to become available */
                wp = request_wakeup((struct MinList *)qi->WList);
                ReleaseSemaphore(qi->Lock);
                ipc_unlock(&msg_keymap);
                wait_list(wp);
                return msgsnd(qid, msg, mlen, flags);  /* Retry */
            }
        }
        
        ReleaseSemaphore(qi->Lock);
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&msg_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Receive a message */
ssize_t msgrcv(int qid, void *msg, size_t mlen, long int mtype, int flags)
{
    int ret = -1;
    struct Msg *m, *prev;
    int nbytes;
    struct msgqid_ds *qi;
    struct WProc *wp;
    
    ENTER();
    SHOWVALUE(qid);
    SHOWPOINTER(msg);
    SHOWVALUE(mlen);
    SHOWVALUE(mtype);
    SHOWVALUE(flags);
    
    if (!msg) {
        set_ipc_errno(EFAULT);
        return -1;
    }
    
    ipc_lock(&msg_keymap);
    
    qi = (struct msgqid_ds *)get_ipc_by_id(&msg_keymap, qid);
    if (qi) {
        ObtainSemaphore(qi->Lock);
        
        /* Find a matching message */
        m = (struct Msg *)qi->msg_first;
        prev = NULL;
        while (m) {
            if (mtype == 0 || mtype == m->Type || (mtype < 0 && m->Type <= -mtype)) {
                if (m->Size <= mlen || (flags & IPC_NOERROR)) {
                    nbytes = (m->Size <= mlen) ? m->Size : mlen;
                    ((long *)msg)[0] = m->Type;
                    memcpy((char *)msg + sizeof(long), m->Data, nbytes - sizeof(long));
                    
                    /* Remove message from queue */
                    if (prev) {
                        prev->Next = m->Next;
                    } else {
                        qi->msg_first = m->Next;
                    }
                    if (m == qi->msg_last) {
                        qi->msg_last = prev;
                    }
                    
                    qi->msg_cbytes -= m->Size;
                    qi->msg_qnum--;
                    qi->msg_rtime = time(NULL);
                    qi->msg_lrpid = (int)((ULONG)FindTask(0));  /* Use Task pointer as PID */
                    
                    FreeVec(m);
                    
                    /* Wake up waiting writers */
                    wake_list((struct MinList *)qi->WList);
                    
                    ret = nbytes;
                    break;
                } else {
                    set_ipc_errno(E2BIG);
                    break;
                }
            }
            prev = m;
            m = m->Next;
        }
        
        if (ret < 0 && !(flags & IPC_NOWAIT)) {
            /* Wait for a message to become available */
            wp = request_wakeup((struct MinList *)qi->RList);
            ReleaseSemaphore(qi->Lock);
            ipc_unlock(&msg_keymap);
            wait_list(wp);
            return msgrcv(qid, msg, mlen, mtype, flags);  /* Retry */
        }
        
        ReleaseSemaphore(qi->Lock);
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&msg_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Control message queue */
int msgctl(int qid, int cmd, struct msgqid_ds *buf)
{
    int ret = -1;
    struct msgqid_ds *qi;
    
    ENTER();
    SHOWVALUE(qid);
    SHOWVALUE(cmd);
    SHOWPOINTER(buf);
    
    ipc_lock(&msg_keymap);
    
    qi = (struct msgqid_ds *)get_ipc_by_id(&msg_keymap, qid);
    if (qi) {
        switch (cmd) {
        case IPC_STAT:
            if (buf) {
                memcpy(buf, qi, sizeof(struct msgqid_ds));
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        case IPC_SET:
            if (buf) {
                qi->msg_perm.uid = buf->msg_perm.uid;
                qi->msg_perm.gid = buf->msg_perm.gid;
                qi->msg_perm.mode = buf->msg_perm.mode;
                qi->msg_qbytes = buf->msg_qbytes;
                ret = 0;
            } else {
                set_ipc_errno(EFAULT);
            }
            break;
            
        case IPC_RMID:
            ipc_rm_id(&msg_keymap, qid, 
                     (void (*)(struct IPCGeneric *))msg_destroy);
            ret = 0;
            break;
            
        default:
            set_ipc_errno(EINVAL);
            break;
        }
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&msg_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Snapshot message queue */
int msgsnap(int qid, void *qbuf, size_t blen, long mtype)
{
    struct Msg *m;
    int ret = -1;
    int tlen, act_size = 0, act_cnt = 0;
    int bpos = 0;
    int i;
    struct msgqid_ds *qi;
    const int sizemask = sizeof(uint32_t) - 1;
    
    ENTER();
    SHOWVALUE(qid);
    SHOWPOINTER(qbuf);
    SHOWVALUE(blen);
    SHOWVALUE(mtype);
    
    ipc_lock(&msg_keymap);
    
    qi = (struct msgqid_ds *)get_ipc_by_id(&msg_keymap, qid);
    if (qi) {
        ObtainSemaphore(qi->Lock);
        
        tlen = qi->msg_cbytes + sizeof(struct msgsnap_head) + 
               qi->msg_qnum * (sizeof(struct msgsnap_mhead) + sizemask);
        
        if (blen < sizeof(struct msgsnap_head)) {
            set_ipc_errno(EINVAL);
        } else if (blen < tlen) {
            if (qbuf) {
                {
                    struct msgsnap_head head;
                    head.msgsnap_size = tlen;
                    head.msgsnap_nmsg = 0;
                    memcpy(qbuf, &head, sizeof(struct msgsnap_head));
                }
            }
            ret = tlen;
        } else {
            /* Copy messages */
            bpos += sizeof(struct msgsnap_head);
            m = (struct Msg *)qi->msg_first;
            while (m) {
                if (mtype == 0 || (mtype == m->Type) || (mtype < 0 && m->Type <= -mtype)) {
                    i = m->Size + sizeof(struct msgsnap_mhead);
                    if (bpos + i <= blen) {
                        {
                            struct msgsnap_mhead mhead;
                            mhead.msgsnap_mlen = m->Size;
                            mhead.msgsnap_mtype = m->Type;
                            memcpy((char *)qbuf + bpos, &mhead, sizeof(struct msgsnap_mhead));
                        }
                        memcpy((char *)qbuf + bpos + sizeof(struct msgsnap_mhead), m->Data, m->Size - sizeof(long));
                        bpos += i;
                        act_size += i;
                        act_cnt++;
                    }
                }
                m = m->Next;
            }
            
            {
                struct msgsnap_head head;
                head.msgsnap_size = act_size;
                head.msgsnap_nmsg = act_cnt;
                memcpy(qbuf, &head, sizeof(struct msgsnap_head));
            }
            ret = act_size;
        }
        
        ReleaseSemaphore(qi->Lock);
    } else {
        set_ipc_errno(EINVAL);
    }
    
    ipc_unlock(&msg_keymap);
    
    SHOWVALUE(ret);
    return ret;
}

/* Get message queue IDs */
int msgids(int *buf, unsigned int nids, unsigned int *qcnt)
{
    int ret;
    
    ENTER();
    SHOWPOINTER(buf);
    SHOWVALUE(nids);
    SHOWPOINTER(qcnt);
    
    ret = ipc_ids(&msg_keymap, buf, nids, (int *)qcnt);
    
    SHOWVALUE(ret);
    return ret;
}
