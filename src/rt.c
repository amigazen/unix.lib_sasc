/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * rt.c - POSIX.1b realtime library implementation for Amiga
 *
 * This file implements POSIX realtime timer functions using Amiga's
 * timer.device API.
 *
 * * Functions implemented:
 * - timer_create, timer_delete, timer_gettime, timer_settime, timer_getoverrun
 * - clock_gettime, clock_settime, clock_getres, clock_nanosleep
 * - nanosleep
 */

 #include <sys/timer.h>
 #include <sys/time.h>
 #include <errno.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <signal.h>
 
 #include <proto/exec.h>
 #include <devices/timer.h>
 #include <utility/tagitem.h>
 #include <exec/signals.h>
 #include <exec/interrupts.h>
 
 /* Use lower-level chkabort consistently */
 extern void __chkabort(void);
 
 /* Define interrupt handler calling convention */
 #if defined(__GNUC__)
 #define INTERRUPT_HANDLER __saveds __asm
 #else
 #define INTERRUPT_HANDLER __interrupt
 #endif
 
 /* Helper structure for timer.device requests (for nanosleep) */
 struct TimerIFace {
     struct MsgPort *port;
     struct TimeRequest *req;
     int is_open;
 };
 
 /* Forward declaration for the interrupt handler */
 static void INTERRUPT_HANDLER _timer_handler_func(register __a0 struct amiga_timer *timer);
 
 /* Internal timer structure - now based on timer.device */
 struct amiga_timer {
     timer_t timer_id;
     struct sigevent notification;
     struct itimerspec current_value;
     int active;
     volatile int overrun_count;
     struct MsgPort *port;
     struct TimeRequest *req;
     struct Interrupt handler;
     struct Task *owner_task;
     ULONG signal_bit;
 };
 
 /* Global timer management */
 static struct amiga_timer *timers = NULL;
 static int timer_count = 0;
 static int timer_capacity = 0;
 static timer_t next_timer_id = 1;
 
 /* Amiga Library/Device bases */
 static struct TimerIFace TimerInterface = {NULL, NULL, 0};
 
 /* Forward declarations */
 static void _cleanup_resources(void);
 static int _init_timer_device(void);
 static struct amiga_timer *_find_timer(timer_t timerid);
 
 /*
  * ============================================================================
  * Resource Management
  * ============================================================================
  */
 
 /* Cleanup libraries and devices on exit */
 static void _cleanup_resources(void) {
     if (TimerInterface.is_open) {
         if (!CheckIO((struct IORequest *)TimerInterface.req)) {
             AbortIO((struct IORequest *)TimerInterface.req);
         }
         WaitIO((struct IORequest *)TimerInterface.req);
         
         CloseDevice((struct IORequest *)TimerInterface.req);
         
         if (TimerInterface.req) DeleteExtIO((struct IORequest *)TimerInterface.req);
         if (TimerInterface.port) DeleteMsgPort(TimerInterface.port);
         
         TimerInterface.is_open = 0;
     }
 }
 
 /* Initialize timer.device interface for nanosleep */
 static int _init_timer_device(void) {
     if (TimerInterface.is_open) return 0;
     TimerInterface.port = CreateMsgPort();
     if (!TimerInterface.port) return -1;
     TimerInterface.req = (struct TimeRequest *)CreateExtIO(TimerInterface.port, sizeof(struct TimeRequest));
     if (!TimerInterface.req) {
         DeleteMsgPort(TimerInterface.port);
         TimerInterface.port = NULL;
         return -1;
     }
     if (OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)TimerInterface.req, 0) != 0) {
         DeleteExtIO((struct IORequest *)TimerInterface.req);
         TimerInterface.req = NULL;
         DeleteMsgPort(TimerInterface.port);
         TimerInterface.port = NULL;
         return -1;
     }
     TimerInterface.is_open = 1;
     atexit(_cleanup_resources);
     return 0;
 }
 
 /*
  * ============================================================================
  * Timer Management
  * ============================================================================
  */
 
 /* Find timer by ID */
 static struct amiga_timer *_find_timer(timer_t timerid) {
     for (int i = 0; i < timer_count; i++) {
         if (timers[i].timer_id == timerid) {
             return &timers[i];
         }
     }
     return NULL;
 }
 
 /*
  * ============================================================================
  * POSIX API Implementation
  * ============================================================================
  */
 
 /* The software interrupt handler for timer expirations */
 static void INTERRUPT_HANDLER _timer_handler_func(register __a0 struct amiga_timer *timer) {
     if (CheckIO((struct IORequest *)timer->req)) {
         WaitIO((struct IORequest *)timer->req);
     }
     timer->overrun_count++;
 
     if (timer->current_value.it_interval.tv_sec > 0 || timer->current_value.it_interval.tv_nsec > 0) {
         timer->req->tr_node.io_Command = TR_ADDREQUEST;
         timer->req->tr_time.tv_secs = timer->current_value.it_interval.tv_sec;
         timer->req->tr_time.tv_micro = timer->current_value.it_interval.tv_nsec / 1000;
         SendIO((struct IORequest *)timer->req);
     } else {
         timer->active = 0;
     }
 
     if (timer->notification.sigev_notify == SIGEV_SIGNAL) {
         Signal(timer->owner_task, SIGF_SINGLE);
     }
 }
 
 int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid) {
     __chkabort();
     if (timerid == NULL || (clockid != CLOCK_REALTIME && clockid != CLOCK_MONOTONIC)) {
         errno = EINVAL;
         return -1;
     }
     if (sevp && sevp->sigev_notify == SIGEV_THREAD) {
         errno = EINVAL; return -1; /* Unsupported */
     }
 
     if (timer_count >= timer_capacity) {
         int new_capacity = timer_capacity ? timer_capacity * 2 : 8;
         struct amiga_timer *new_timers = realloc(timers, new_capacity * sizeof(struct amiga_timer));
         if (!new_timers) { errno = ENOMEM; return -1; }
         timers = new_timers;
         timer_capacity = new_capacity;
     }
 
     struct amiga_timer *timer = &timers[timer_count];
     memset(timer, 0, sizeof(struct amiga_timer));
 
     timer->timer_id = next_timer_id++;
     timer->owner_task = FindTask(NULL);
     timer->signal_bit = AllocSignal(-1);
     if (timer->signal_bit == -1) { errno = ENOMEM; return -1; }
 
     timer->port = CreateMsgPort();
     if (!timer->port) { FreeSignal(timer->signal_bit); errno = ENOMEM; return -1; }
     
     timer->port->mp_SigBit = timer->signal_bit;
     timer->port->mp_SigTask = &timer->handler;
     timer->port->mp_Flags = PA_SOFTINT;
 
     timer->handler.is_Node.ln_Type = NT_INTERRUPT;
     timer->handler.is_Data = (APTR)timer;
     timer->handler.is_Code = (void (*)())_timer_handler_func;
 
     timer->req = (struct TimeRequest *)CreateIORequest(timer->port, sizeof(struct TimeRequest));
     if (!timer->req) { DeleteMsgPort(timer->port); FreeSignal(timer->signal_bit); errno = ENOMEM; return -1; }
 
     if (OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)timer->req, 0) != 0) {
         DeleteIORequest((struct IORequest *)timer->req);
         DeleteMsgPort(timer->port);
         FreeSignal(timer->signal_bit);
         errno = ENOSYS; return -1;
     }
 
     if (sevp) timer->notification = *sevp;
     else {
         timer->notification.sigev_notify = SIGEV_SIGNAL;
         timer->notification.sigev_signo = SIGALRM;
         timer->notification.sigev_value.sival_int = timer->timer_id;
     }
     
     *timerid = timer->timer_id;
     timer_count++;
     return 0;
 }
 
 int timer_delete(timer_t timerid) {
     __chkabort();
     struct amiga_timer *timer = NULL;
     int i;
     for (i = 0; i < timer_count; i++) {
         if (timers[i].timer_id == timerid) { timer = &timers[i]; break; }
     }
     if (!timer) { errno = EINVAL; return -1; }
 
     if (timer->active) { AbortIO((struct IORequest *)timer->req); WaitIO((struct IORequest *)timer->req); }
     CloseDevice((struct IORequest *)timer->req);
     DeleteIORequest((struct IORequest *)timer->req);
     DeleteMsgPort(timer->port);
     FreeSignal(timer->signal_bit);
 
     timer_count--;
     if (i < timer_count) memmove(&timers[i], &timers[i+1], (timer_count - i) * sizeof(struct amiga_timer));
     return 0;
 }
 
 int timer_gettime(timer_t timerid, struct itimerspec *curr_value) {
     __chkabort();
     if (!curr_value) { errno = EINVAL; return -1; }
     struct amiga_timer *timer = _find_timer(timerid);
     if (!timer) { errno = EINVAL; return -1; }
 
     *curr_value = timer->current_value;
     if (!timer->active) {
         curr_value->it_value.tv_sec = 0;
         curr_value->it_value.tv_nsec = 0;
     }
     /* Note: a precise implementation would require reading the timer device
        to see how much time is left. This is a reasonable approximation. */
     return 0;
 }
 
 int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value) {
     __chkabort();
     if (!new_value) { errno = EINVAL; return -1; }
     struct amiga_timer *timer = _find_timer(timerid);
     if (!timer) { errno = EINVAL; return -1; }
     if (old_value) *old_value = timer->current_value;
 
     if (timer->active) {
         AbortIO((struct IORequest *)timer->req);
         WaitIO((struct IORequest *)timer->req);
         timer->active = 0;
     }
 
     timer->current_value = *new_value;
     timer->overrun_count = 0;
 
     if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 0) return 0;
 
     struct timespec request_time = new_value->it_value;
     if (flags & TIMER_ABSTIME) {
         struct timespec now;
         clock_gettime(CLOCK_REALTIME, &now);
         if (now.tv_sec > request_time.tv_sec || (now.tv_sec == request_time.tv_sec && now.tv_nsec >= request_time.tv_nsec)) return 0;
         request_time.tv_sec -= now.tv_sec;
         request_time.tv_nsec -= now.tv_nsec;
         if (request_time.tv_nsec < 0) { request_time.tv_sec--; request_time.tv_nsec += 1000000000; }
     }
     
     timer->req->tr_node.io_Command = TR_ADDREQUEST;
     timer->req->tr_time.tv_secs = request_time.tv_sec;
     timer->req->tr_time.tv_micro = request_time.tv_nsec / 1000;
     
     SendIO((struct IORequest *)timer->req);
     timer->active = 1;
     return 0;
 }
 
 int timer_getoverrun(timer_t timerid) {
     __chkabort();
     struct amiga_timer *timer = _find_timer(timerid);
     if (!timer) { errno = EINVAL; return -1; }
     int overruns = timer->overrun_count;
     timer->overrun_count = 0;
     return overruns;
 }
 
 int clock_gettime(clockid_t clockid, struct timespec *tp) {
     __chkabort();
     if (!tp) { errno = EINVAL; return -1; }
     struct timeval tv;
     switch (clockid) {
         case CLOCK_REALTIME: case CLOCK_MONOTONIC:
             gettimeofday(&tv, NULL);
             tp->tv_sec = tv.tv_sec; tp->tv_nsec = tv.tv_usec * 1000;
             return 0;
         case CLOCK_PROCESS_CPUTIME_ID: case CLOCK_THREAD_CPUTIME_ID:
             errno = EINVAL; return -1;
         default: errno = EINVAL; return -1;
     }
 }
 
 int clock_settime(clockid_t clockid, const struct timespec *tp) {
     __chkabort();
     if (!tp) { errno = EINVAL; return -1; }
     if (clockid == CLOCK_REALTIME) {
         struct timeval tv = {tp->tv_sec, tp->tv_nsec / 1000};
         if (settimeofday(&tv, NULL) != 0) return -1;
         return 0;
     }
     errno = EPERM; return -1;
 }
 
 int clock_getres(clockid_t clockid, struct timespec *res) {
     __chkabort();
     if (!res) { errno = EINVAL; return -1; }
     switch (clockid) {
         case CLOCK_REALTIME: case CLOCK_MONOTONIC:
             res->tv_sec = 0; res->tv_nsec = 1000;
             return 0;
         case CLOCK_PROCESS_CPUTIME_ID: case CLOCK_THREAD_CPUTIME_ID:
             errno = EINVAL; return -1;
         default: errno = EINVAL; return -1;
     }
 }
 
 int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *remain) {
     __chkabort();
     if (!request || request->tv_nsec < 0 || request->tv_nsec >= 1000000000 || request->tv_sec < 0) { errno = EINVAL; return -1; }
     if (clockid != CLOCK_REALTIME && clockid != CLOCK_MONOTONIC) { errno = EINVAL; return -1; }
 
     struct timespec relative_request;
     if (flags & TIMER_ABSTIME) {
         struct timespec now;
         clock_gettime(clockid, &now);
         if (now.tv_sec > request->tv_sec || (now.tv_sec == request->tv_sec && now.tv_nsec >= request->tv_nsec)) return 0;
         relative_request.tv_sec = request->tv_sec - now.tv_sec;
         relative_request.tv_nsec = request->tv_nsec - now.tv_nsec;
         if (relative_request.tv_nsec < 0) { relative_request.tv_sec--; relative_request.tv_nsec += 1000000000; }
     } else {
         relative_request = *request;
     }
     
     if (_init_timer_device() != 0) { errno = ENOSYS; return -1; }
 
     struct timeval start_tv, end_tv;
     gettimeofday(&start_tv, NULL);
     TimerInterface.req->tr_node.io_Command = TR_ADDREQUEST;
     TimerInterface.req->tr_time.tv_secs = relative_request.tv_sec;
     TimerInterface.req->tr_time.tv_micro = relative_request.tv_nsec / 1000;
 
     ULONG timer_sig = 1L << TimerInterface.port->mp_SigBit;
     ULONG break_sigs = SIGBREAKF_CTRLC | SIGBREAKF_CTRLD;
 
     SendIO((struct IORequest *)TimerInterface.req);
     ULONG signals = Wait(timer_sig | break_sigs);
 
     if (signals & timer_sig) {
         WaitIO((struct IORequest *)TimerInterface.req);
         if (remain) { remain->tv_sec = 0; remain->tv_nsec = 0; }
         return 0;
     } else {
         gettimeofday(&end_tv, NULL);
         AbortIO((struct IORequest *)TimerInterface.req);
         WaitIO((struct IORequest *)TimerInterface.req);
         if (remain) {
             long es = end_tv.tv_sec - start_tv.tv_sec, eus = end_tv.tv_usec - start_tv.tv_usec;
             if (eus < 0) { es--; eus += 1000000; }
             long rs = relative_request.tv_sec - es, rns = relative_request.tv_nsec - (eus * 1000);
             if (rns < 0) { rs--; rns += 1000000000; }
             if (rs < 0) { remain->tv_sec = 0; remain->tv_nsec = 0; }
             else { remain->tv_sec = rs; remain->tv_nsec = rns; }
         }
         errno = EINTR; return -1;
     }
 }
 
 int nanosleep(const struct timespec *request, struct timespec *remain) {
     __chkabort();
     return clock_nanosleep(CLOCK_MONOTONIC, 0, request, remain);
 }
 
 