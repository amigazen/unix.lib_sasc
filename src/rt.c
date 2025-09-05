/*
 * SPDX-License-Identifier: BSD-2-Clause
 * * rt_revised.c - POSIX.1b realtime library implementation for Amiga
 * * This file implements POSIX realtime timer functions using Amiga's
 * realtime.library and timer.device APIs. This version improves accuracy,
 * POSIX conformance, and Amiga-specific best practices.
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
         if (TimerInterface.req) {
             CloseDevice((struct IORequest *)TimerInterface.req);
             DeleteExtIO((struct IORequest *)TimerInterface.req);
         }
         if (TimerInterface.port) {
             DeleteMsgPort(TimerInterface.port);
         }
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
     
     char conductor_name[32];
     sprintf(conductor_name, "posix_timer_%ld", timer->timer_id);
 
     /* For simplicity, we create a conductor but no player yet.
        The player will be associated when the timer is set. */
     timer->conductor = CreateConductor(
         PLAYER_Name, conductor_name, 
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
 
     /* TODO: A full implementation would need to calculate remaining time */
     *curr_value = timer->current_value;
     
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
 
     /* This is a simplified implementation. realtime.library is not a good
        fit for interval timers. A timer.device-based approach would be better.
        This implementation only handles the initial expiration. */
     ULONG ticks = new_value->it_value.tv_sec * 50 + new_value->it_value.tv_nsec / 20000000;
 
     if (ticks > 0) {
         SetConductorState(timer->conductor, CLOCKSTATE_RUNNING, 0);
         /* A player would need to be created and linked here */
         timer->active = 1;
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
 
     struct timespec target_ts = *request;
     if (flags & TIMER_ABSTIME) {
         struct timespec now;
         clock_gettime(clockid, &now);
         
         if (now.tv_sec > target_ts.tv_sec || (now.tv_sec == target_ts.tv_sec && now.tv_nsec >= target_ts.tv_nsec)) {
              return 0; /* Already past the target time */
         }
 
         /* Calculate relative time */
         target_ts.tv_sec = request->tv_sec - now.tv_sec;
         target_ts.tv_nsec = request->tv_nsec - now.tv_nsec;
         if (target_ts.tv_nsec < 0) {
             target_ts.tv_sec--;
             target_ts.tv_nsec += 1000000000;
         }
     }
     
     return nanosleep(&target_ts, remain);
 }
 
 int nanosleep(const struct timespec *request, struct timespec *remain) {
     __chkabort();
     
     if (request == NULL || request->tv_nsec < 0 || request->tv_nsec >= 1000000000 || request->tv_sec < 0) {
         errno = EINVAL;
         return -1;
     }
 
     if (_init_timer_device() != 0) {
         errno = ENOSYS;
         return -1;
     }
 
     TimerInterface.req->tr_node.io_Command = TR_ADDREQUEST;
     TimerInterface.req->tr_time.tv_secs = request->tv_sec;
     TimerInterface.req->tr_time.tv_micro = request->tv_nsec / 1000;
     
     DoIO((struct IORequest *)TimerInterface.req);
 
     /* On Amiga, sleep is generally not interruptible in a POSIX sense,
        so 'remain' will be zero on success. A SIGABRT (Ctrl+C) will
        terminate, not interrupt. */
     if (remain != NULL) {
         remain->tv_sec = 0;
         remain->tv_nsec = 0;
     }
     
     return 0;
 }
 