/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * * rt.c - POSIX.1b realtime library implementation for Amiga
 *
 * * This file implements POSIX realtime timer functions using Amiga's
 * realtime.library and timer.device APIs. 
 *
 * * Functions implemented:
 * - timer_create, timer_delete, timer_gettime, timer_settime
 * - clock_gettime, clock_settime, clock_getres, clock_nanosleep
 * - nanosleep
 */

 #include <sys/timer.h>
 #include <sys/time.h>
 #include <errno.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>
 
 #include <proto/exec.h>
 #include <proto/realtime.h>
 #include <devices/timer.h>
 #include <utility/tagitem.h>
 #include <exec/signals.h>
 
 /* Use lower-level chkabort consistently */
 extern void __chkabort(void);
 
 /* RealTime library constants from <libraries/realtime.h> */
 #ifndef CLOCKSTATE_STOPPED
 #define CLOCKSTATE_STOPPED 0
 #endif
 #ifndef CLOCKSTATE_RUNNING
 #define CLOCKSTATE_RUNNING 1
 #endif
 
 /* Player tag constants from <libraries/realtime.h> */
 #ifndef PLAYER_AlarmTime
 #define PLAYER_AlarmTime (TAG_USER + 1)
 #endif
 #ifndef PLAYER_Ready
 #define PLAYER_Ready (TAG_USER + 2)
 #endif
 #ifndef PLAYER_Name
 #define PLAYER_Name (TAG_USER + 3)
 #endif
 #ifndef PLAYER_Conductor
 #define PLAYER_Conductor (TAG_USER + 4)
 #endif
 
 /*
  * Helper structure for timer.device requests.
  * Standard Amiga practice.
  */
 struct TimerIFace {
     struct MsgPort *port;
     struct TimeRequest *req;
     int is_open;
 };
 
 /* Internal timer structure */
 struct amiga_timer {
     timer_t timer_id;                  /* POSIX timer ID */
     struct sigevent notification;      /* Notification method */
     struct Conductor *conductor;       /* RealTime library conductor */
     struct Player *player;             /* RealTime library player */
     int active;                        /* Timer active flag */
     struct itimerspec current_value;   /* Current timer value */
     struct timespec start_time;        /* Time when the timer was set */
     char conductor_name[32];           /* Name of the conductor */
 };
 
 /* Global timer management */
 static struct amiga_timer *timers = NULL;
 static int timer_count = 0;
 static int timer_capacity = 0;
 static timer_t next_timer_id = 1;
 
 /* Amiga Library/Device bases */
 static struct Library *RealTimeBase = NULL;
 static struct TimerIFace TimerInterface = {NULL, NULL, 0};
 
 /* Forward declarations */
 static void _cleanup_resources(void);
 static int _init_realtime_library(void);
 static int _init_timer_device(void);
 static struct amiga_timer *_find_timer(timer_t timerid);
 
 /*
  * ============================================================================
  * Resource Management
  * ============================================================================
  */
 
 /* Cleanup libraries and devices on exit */
 static void _cleanup_resources(void) {
     if (RealTimeBase != NULL) {
         CloseLibrary(RealTimeBase);
         RealTimeBase = NULL;
     }
     if (TimerInterface.is_open) {
         /* Ensure any pending request is aborted */
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
 
 /* Initialize RealTime library interface */
 static int _init_realtime_library(void) {
     if (RealTimeBase == NULL) {
         RealTimeBase = OpenLibrary("realtime.library", 37);
         if (RealTimeBase == NULL) {
             return -1;
         }
         /* Register cleanup function on first successful open */
         atexit(_cleanup_resources);
     }
     return 0;
 }
 
 /* Initialize timer.device interface */
 static int _init_timer_device(void) {
     if (TimerInterface.is_open) {
         return 0;
     }
 
     TimerInterface.port = CreateMsgPort();
     if (!TimerInterface.port) {
         return -1;
     }
 
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
     /* Also register cleanup if it hasn't been already */
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
 
 int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid) {
     __chkabort();
     
     if (timerid == NULL || (clockid != CLOCK_REALTIME && clockid != CLOCK_MONOTONIC)) {
         errno = EINVAL;
         return -1;
     }
 
     if (_init_realtime_library() != 0) {
         errno = ENOSYS;
         return -1;
     }
 
     if (timer_count >= timer_capacity) {
         int new_capacity = timer_capacity ? timer_capacity * 2 : 8;
         struct amiga_timer *new_timers = realloc(timers, new_capacity * sizeof(struct amiga_timer));
         if (new_timers == NULL) {
             errno = ENOMEM;
             return -1;
         }
         timers = new_timers;
         timer_capacity = new_capacity;
     }
 
     struct amiga_timer *timer = &timers[timer_count];
     memset(timer, 0, sizeof(struct amiga_timer));
 
     timer->timer_id = next_timer_id++;
     
     sprintf(timer->conductor_name, "posix_timer_%ld", timer->timer_id);
 
     timer->conductor = CreateConductor(
         PLAYER_Name, timer->conductor_name, 
         TAG_END
     );
 
     if (timer->conductor == NULL) {
         errno = ENOMEM;
         return -1;
     }
 
     if (sevp != NULL) {
         timer->notification = *sevp;
     } else {
         timer->notification.sigev_notify = SIGEV_SIGNAL;
         timer->notification.sigev_signo = SIGALRM;
         timer->notification.sigev_value.sival_int = timer->timer_id;
     }
 
     /* NOTE: Signal-based timers are complex with realtime.library.
        A robust implementation would require a separate handler process.
        This implementation is simplified and may not be fully conformant
        for SIGEV_SIGNAL without more infrastructure. */
 
     timer->active = 0;
     *timerid = timer->timer_id;
     timer_count++;
 
     return 0;
 }
 
 int timer_delete(timer_t timerid) {
     __chkabort();
 
     int i;
     struct amiga_timer *timer = NULL;
     for (i = 0; i < timer_count; i++) {
         if (timers[i].timer_id == timerid) {
             timer = &timers[i];
             break;
         }
     }
 
     if (timer == NULL) {
         errno = EINVAL;
         return -1;
     }
     
     if (timer->active) {
         SetConductorState(timer->conductor, CLOCKSTATE_STOPPED, 0);
     }
     if (timer->player) DeletePlayer(timer->player);
     if (timer->conductor) DeleteConductor(timer->conductor);
 
     /* Shift remaining timers down */
     timer_count--;
     if (i < timer_count) {
         memmove(&timers[i], &timers[i+1], (timer_count - i) * sizeof(struct amiga_timer));
     }
     
     return 0;
 }
 
 int timer_gettime(timer_t timerid, struct itimerspec *curr_value) {
     __chkabort();
     
     if (curr_value == NULL) {
         errno = EINVAL;
         return -1;
     }
 
     struct amiga_timer *timer = _find_timer(timerid);
     if (timer == NULL) {
         errno = EINVAL;
         return -1;
     }
 
     /* Copy the configured interval */
     curr_value->it_interval = timer->current_value.it_interval;
 
     if (!timer->active) {
         /* Timer is disarmed */
         curr_value->it_value.tv_sec = 0;
         curr_value->it_value.tv_nsec = 0;
     } else {
         /* Timer is armed, calculate remaining time */
         struct timespec now, elapsed;
         clock_gettime(CLOCK_MONOTONIC, &now);
 
         elapsed.tv_sec = now.tv_sec - timer->start_time.tv_sec;
         elapsed.tv_nsec = now.tv_nsec - timer->start_time.tv_nsec;
         if (elapsed.tv_nsec < 0) {
             elapsed.tv_sec--;
             elapsed.tv_nsec += 1000000000;
         }
 
         /* Calculate remaining time */
         curr_value->it_value.tv_sec = timer->current_value.it_value.tv_sec - elapsed.tv_sec;
         curr_value->it_value.tv_nsec = timer->current_value.it_value.tv_nsec - elapsed.tv_nsec;
         if (curr_value->it_value.tv_nsec < 0) {
             curr_value->it_value.tv_sec--;
             curr_value->it_value.tv_nsec += 1000000000;
         }
 
         /* If time has already expired, report zero */
         if (curr_value->it_value.tv_sec < 0) {
             curr_value->it_value.tv_sec = 0;
             curr_value->it_value.tv_nsec = 0;
         }
     }
     
     return 0;
 }
 
 int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value) {
     __chkabort();
     
     if (new_value == NULL) {
         errno = EINVAL;
         return -1;
     }
 
     struct amiga_timer *timer = _find_timer(timerid);
     if (timer == NULL) {
         errno = EINVAL;
         return -1;
     }
 
     if (old_value != NULL) {
         *old_value = timer->current_value;
     }
 
     if (timer->active) {
         SetConductorState(timer->conductor, CLOCKSTATE_STOPPED, 0);
         timer->active = 0;
     }
     
     timer->current_value = *new_value;
     
     /* Disarm timer if value is zero */
     if (new_value->it_value.tv_sec == 0 && new_value->it_value.tv_nsec == 0) {
         return 0;
     }
 
     /* Delete old player if it exists */
     if (timer->player) {
         DeletePlayer(timer->player);
         timer->player = NULL;
     }
 
     /* NOTE: POSIX interval timers (it_interval > 0) are not supported by this
        implementation, as realtime.library does not provide an automatic
        re-arming mechanism. The timer will fire only once. */
     ULONG ticks = new_value->it_value.tv_sec * 50 + new_value->it_value.tv_nsec / 20000000;
 
     if (ticks > 0) {
         char player_name[32];
         sprintf(player_name, "posix_player_%ld", timer->timer_id);
         
         timer->player = CreatePlayer(
             PLAYER_Name, player_name,
             PLAYER_Conductor, timer->conductor_name,
             TAG_END);
 
         if (timer->player == NULL) {
             errno = ENOMEM;
             return -1;
         }
 
         SetPlayerAttrs(timer->player, PLAYER_AlarmTime, ticks, TAG_END);
         SetConductorState(timer->conductor, CLOCKSTATE_RUNNING, 0);
         SetPlayerAttrs(timer->player, PLAYER_Ready, TRUE, TAG_END);
         
         timer->active = 1;
         clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
     }
 
     return 0;
 }
 
 int clock_gettime(clockid_t clockid, struct timespec *tp) {
     __chkabort();
 
     if (tp == NULL) {
         errno = EINVAL;
         return -1;
     }
 
     struct timeval tv;
     switch (clockid) {
         case CLOCK_REALTIME:
         case CLOCK_MONOTONIC:
             /* Amiga's system time (timer.device) is monotonic since boot.
                gettimeofday is based on it but adds a boot time offset. */
             gettimeofday(&tv, NULL);
             tp->tv_sec = tv.tv_sec;
             tp->tv_nsec = tv.tv_usec * 1000;
             return 0;
             
         case CLOCK_PROCESS_CPUTIME_ID:
         case CLOCK_THREAD_CPUTIME_ID:
             /* Unsupported on AmigaOS */
             errno = EINVAL;
             return -1;
             
         default:
             errno = EINVAL;
             return -1;
     }
 }
 
 int clock_settime(clockid_t clockid, const struct timespec *tp) {
     __chkabort();
     
     if (tp == NULL) {
         errno = EINVAL;
         return -1;
     }
     
     if (clockid == CLOCK_REALTIME) {
         struct timeval tv;
         tv.tv_sec = tp->tv_sec;
         tv.tv_usec = tp->tv_nsec / 1000;
         if (settimeofday(&tv, NULL) != 0) {
             /* errno should be set by settimeofday */
             return -1;
         }
         return 0;
     }
     
     /* Other clocks cannot be set */
     errno = EPERM;
     return -1;
 }
 
 int clock_getres(clockid_t clockid, struct timespec *res) {
     __chkabort();
     
     if (res == NULL) {
         errno = EINVAL;
         return -1;
     }
 
     switch (clockid) {
         case CLOCK_REALTIME:
         case CLOCK_MONOTONIC:
             /* Resolution is based on timer.device, typically 1 microsecond */
             res->tv_sec = 0;
             res->tv_nsec = 1000; /* 1 microsecond */
             return 0;
         
         case CLOCK_PROCESS_CPUTIME_ID:
         case CLOCK_THREAD_CPUTIME_ID:
             errno = EINVAL;
             return -1;
 
         default:
             errno = EINVAL;
             return -1;
     }
 }
 
 int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *remain) {
     __chkabort();
     
     if (request == NULL || request->tv_nsec < 0 || request->tv_nsec >= 1000000000 || request->tv_sec < 0) {
         errno = EINVAL;
         return -1;
     }
     
     if (clockid != CLOCK_REALTIME && clockid != CLOCK_MONOTONIC) {
         errno = EINVAL;
         return -1;
     }
 
     struct timespec relative_request;
     if (flags & TIMER_ABSTIME) {
         struct timespec now;
         clock_gettime(clockid, &now);
         
         if (now.tv_sec > request->tv_sec || (now.tv_sec == request->tv_sec && now.tv_nsec >= request->tv_nsec)) {
              return 0; /* Already past the target time */
         }
 
         /* Calculate relative time */
         relative_request.tv_sec = request->tv_sec - now.tv_sec;
         relative_request.tv_nsec = request->tv_nsec - now.tv_nsec;
         if (relative_request.tv_nsec < 0) {
             relative_request.tv_sec--;
             relative_request.tv_nsec += 1000000000;
         }
     } else {
         relative_request = *request;
     }
     
     if (_init_timer_device() != 0) {
         errno = ENOSYS;
         return -1;
     }
 
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
         /* Timer finished normally */
         WaitIO((struct IORequest *)TimerInterface.req);
         if (remain) {
             remain->tv_sec = 0;
             remain->tv_nsec = 0;
         }
         return 0;
     } else {
         /* Interrupted by a signal */
         gettimeofday(&end_tv, NULL);
         AbortIO((struct IORequest *)TimerInterface.req);
         WaitIO((struct IORequest *)TimerInterface.req);
 
         if (remain) {
             long elapsed_s = end_tv.tv_sec - start_tv.tv_sec;
             long elapsed_us = end_tv.tv_usec - start_tv.tv_usec;
             if (elapsed_us < 0) {
                 elapsed_s--;
                 elapsed_us += 1000000;
             }
 
             long remain_s = relative_request.tv_sec - elapsed_s;
             long remain_ns = relative_request.tv_nsec - (elapsed_us * 1000);
             if (remain_ns < 0) {
                 remain_s--;
                 remain_ns += 1000000000;
             }
 
             if (remain_s < 0) {
                 remain->tv_sec = 0;
                 remain->tv_nsec = 0;
             } else {
                 remain->tv_sec = remain_s;
                 remain->tv_nsec = remain_ns;
             }
         }
         errno = EINTR;
         return -1;
     }
 }
 
 int nanosleep(const struct timespec *request, struct timespec *remain) {
     __chkabort();
     
     /* This function is now a simple wrapper around the more-capable clock_nanosleep */
     return clock_nanosleep(CLOCK_MONOTONIC, 0, request, remain);
 }
 
 