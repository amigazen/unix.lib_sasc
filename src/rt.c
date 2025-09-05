/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * rt.c - POSIX.1b realtime library implementation for Amiga
 * 
 * This file implements POSIX realtime timer functions using Amiga's
 * realtime.library and timer.device APIs.
 * 
 * Functions implemented:
 * - timer_create, timer_delete, timer_gettime, timer_settime
 * - clock_gettime, clock_settime, clock_getres, clock_nanosleep
 * - nanosleep
 */

#include "amiga.h"
#include "amigatimer.h"
#include "longlong.h"
#include <sys/timer.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <proto/exec.h>
#include <proto/realtime.h>
#include <unistd.h>
#include <utility/tagitem.h>

/* RealTime library constants */
#ifndef CLOCKSTATE_STOPPED
#define CLOCKSTATE_STOPPED 0
#endif
#ifndef CLOCKSTATE_RUNNING
#define CLOCKSTATE_RUNNING 1
#endif

/* Player tag constants */
#ifndef PLAYER_AlarmTime
#define PLAYER_AlarmTime 0x80000001
#endif
#ifndef PLAYER_Ready
#define PLAYER_Ready 0x80000002
#endif
#ifndef PLAYER_Name
#define PLAYER_Name 0x80000003
#endif
#ifndef PLAYER_Conductor
#define PLAYER_Conductor 0x80000004
#endif

/* Internal timer structure */
struct amiga_timer {
    struct Player *player;             /* RealTime library player */
    struct Conductor *conductor;       /* RealTime library conductor */
    timer_t timer_id;                  /* POSIX timer ID */
    struct sigevent notification;      /* Notification method */
    int signal_bit;                    /* Signal bit for alarm notifications */
    int active;                        /* Timer active flag */
    struct itimerspec current_value;   /* Current timer value */
    struct itimerspec interval;        /* Timer interval */
};

/* Global timer management */
static struct amiga_timer *timers = NULL;
static int timer_count = 0;
static int timer_capacity = 0;
static timer_t next_timer_id = 1;

/* RealTime library interface */

/* Forward declarations */
static int _init_realtime_library(void);
static void _cleanup_realtime_library(void);
static struct amiga_timer *_find_timer(timer_t timerid);
static int _allocate_timer_slot(void);
static void _free_timer_slot(int index);
static int _convert_timespec_to_ticks(const struct timespec *ts);
static void _convert_ticks_to_timespec(int ticks, struct timespec *ts);
static int _clockid_to_amiga_unit(clockid_t clockid);

/* Initialize RealTime library interface */
static int _init_realtime_library(void)
{
    if (RealTimeBase == NULL) {
        RealTimeBase = OpenLibrary("realtime.library", 0);
        if (RealTimeBase == NULL) {
            return -1;
        }
    }
    return 0;
}

/* Cleanup RealTime library interface */
static void _cleanup_realtime_library(void)
{
    if (RealTimeBase != NULL) {
        CloseLibrary(RealTimeBase);
        RealTimeBase = NULL;
    }
}

/* Find timer by ID */
static struct amiga_timer *_find_timer(timer_t timerid)
{
    int i;
    
    for (i = 0; i < timer_count; i++) {
        if (timers[i].timer_id == timerid) {
            return &timers[i];
        }
    }
    return NULL;
}

/* Allocate a new timer slot */
static int _allocate_timer_slot(void)
{
    int i;
    struct amiga_timer *new_timers;
    
    /* Find empty slot */
    for (i = 0; i < timer_count; i++) {
        if (timers[i].timer_id == 0) {
            return i;
        }
    }
    
    /* Need to expand array */
    if (timer_count >= timer_capacity) {
        timer_capacity = timer_capacity ? timer_capacity * 2 : 8;
        new_timers = realloc(timers, timer_capacity * sizeof(struct amiga_timer));
        if (new_timers == NULL) {
            return -1;
        }
        timers = new_timers;
    }
    
    /* Initialize new slot */
    memset(&timers[timer_count], 0, sizeof(struct amiga_timer));
    return timer_count++;
}

/* Free a timer slot */
static void _free_timer_slot(int index)
{
    if (index >= 0 && index < timer_count) {
        memset(&timers[index], 0, sizeof(struct amiga_timer));
    }
}

/* Convert timespec to Amiga ticks (600Hz) */
static int _convert_timespec_to_ticks(const struct timespec *ts)
{
    long long total_usec;
    int ticks;
    
    /* Convert to microseconds */
    total_usec = (long long)ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
    
    /* Convert to 600Hz ticks (1 tick = 1666.67 microseconds) */
    ticks = (int)(total_usec / 1667);
    
    return ticks;
}

/* Convert Amiga ticks to timespec */
static void _convert_ticks_to_timespec(int ticks, struct timespec *ts)
{
    long long total_usec;
    
    /* Convert ticks to microseconds */
    total_usec = (long long)ticks * 1667;
    
    ts->tv_sec = total_usec / 1000000;
    ts->tv_nsec = (total_usec % 1000000) * 1000;
}

/* Convert clockid to Amiga timer device unit */
static int _clockid_to_amiga_unit(clockid_t clockid)
{
    switch (clockid) {
        case CLOCK_REALTIME:
        case CLOCK_MONOTONIC:
            return UNIT_VBLANK;  /* Use VBLANK for system time */
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_THREAD_CPUTIME_ID:
            return UNIT_MICROHZ; /* Use microsecond timer for CPU time */
        default:
            return -1;
    }
}

/* POSIX timer_create implementation */
int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid)
{
    struct amiga_timer *timer;
    int slot_index;
    char conductor_name[32];
    char player_name[32];
    
    chkabort();
    
    if (timerid == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Initialize RealTime library if needed */
    if (_init_realtime_library() < 0) {
        errno = ENOSYS;
        return -1;
    }
    
    /* Allocate timer slot */
    slot_index = _allocate_timer_slot();
    if (slot_index < 0) {
        errno = ENOMEM;
        return -1;
    }
    
    timer = &timers[slot_index];
    timer->timer_id = next_timer_id++;
    
    /* Create unique names */
    sprintf(conductor_name, "posix_timer_%d", timer->timer_id);
    sprintf(player_name, "posix_player_%d", timer->timer_id);
    
    /* Create Player with RealTime library */
    timer->player = CreatePlayer(
        PLAYER_Name, player_name,
        PLAYER_Conductor, conductor_name,
        TAG_END);
    
    if (timer->player == NULL) {
        _free_timer_slot(slot_index);
        errno = ENOMEM;
        return -1;
    }
    
    /* Store notification method */
    if (sevp != NULL) {
        timer->notification = *sevp;
    } else {
        /* Default notification: SIGALRM */
        timer->notification.sigev_notify = SIGEV_SIGNAL;
        timer->notification.sigev_signo = SIGALRM;
        timer->notification.sigev_value.sival_int = timer->timer_id;
    }
    
    /* Allocate signal bit for alarm notifications */
    if (timer->notification.sigev_notify == SIGEV_SIGNAL) {
        timer->signal_bit = AllocSignal(-1);
        if (timer->signal_bit == -1) {
            DeletePlayer(timer->player);
            _free_timer_slot(slot_index);
            errno = ENOMEM;
            return -1;
        }
    }
    
    /* Initialize timer values */
    timer->current_value.it_value.tv_sec = 0;
    timer->current_value.it_value.tv_nsec = 0;
    timer->current_value.it_interval.tv_sec = 0;
    timer->current_value.it_interval.tv_nsec = 0;
    timer->active = 0;
    
    *timerid = timer->timer_id;
    return 0;
}

/* POSIX timer_delete implementation */
int timer_delete(timer_t timerid)
{
    struct amiga_timer *timer;
    int i;
    
    chkabort();
    
    timer = _find_timer(timerid);
    if (timer == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Stop timer if active */
    if (timer->active) {
        SetConductorState(timer->player, CLOCKSTATE_STOPPED, 0);
    }
    
    /* Free signal bit */
    if (timer->signal_bit != -1) {
        FreeSignal(timer->signal_bit);
    }
    
    /* Delete PlayerInfo */
    DeletePlayer(timer->player);
    
    /* Find and free slot */
    for (i = 0; i < timer_count; i++) {
        if (&timers[i] == timer) {
            _free_timer_slot(i);
            break;
        }
    }
    
    return 0;
}

/* POSIX timer_gettime implementation */
int timer_gettime(timer_t timerid, struct itimerspec *curr_value)
{
    struct amiga_timer *timer;
    
    chkabort();
    
    if (curr_value == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    timer = _find_timer(timerid);
    if (timer == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Return current timer value */
    *curr_value = timer->current_value;
    
    return 0;
}

/* POSIX timer_settime implementation */
int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value, struct itimerspec *old_value)
{
    struct amiga_timer *timer;
    int alarm_ticks;
    int result;
    
    chkabort();
    
    if (new_value == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    timer = _find_timer(timerid);
    if (timer == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Return old value if requested */
    if (old_value != NULL) {
        *old_value = timer->current_value;
    }
    
    /* Stop current timer if active */
    if (timer->active) {
        SetConductorState(timer->player, CLOCKSTATE_STOPPED, 0);
        timer->active = 0;
    }
    
    /* Update current value */
    timer->current_value = *new_value;
    timer->interval.it_interval.tv_sec = new_value->it_interval.tv_sec;
    timer->interval.it_interval.tv_nsec = new_value->it_interval.tv_nsec;
    timer->interval.it_value.tv_sec = new_value->it_value.tv_sec;
    timer->interval.it_value.tv_nsec = new_value->it_value.tv_nsec;
    
    /* Start new timer if value is non-zero */
    if (new_value->it_value.tv_sec > 0 || new_value->it_value.tv_nsec > 0) {
        /* Start conductor */
        result = SetConductorState(timer->player, CLOCKSTATE_RUNNING, 0);
        if (result != 0) {
            errno = EINVAL;
            return -1;
        }
        
        /* Convert to ticks and set alarm */
        alarm_ticks = _convert_timespec_to_ticks(&new_value->it_value);
        if (alarm_ticks > 0) {
            result = SetPlayerAttrs(timer->player,
                PLAYER_AlarmTime, alarm_ticks,
                PLAYER_Ready, TRUE,
                TAG_END);
            if (result) {
                timer->active = 1;
            } else {
                errno = EINVAL;
                return -1;
            }
        }
    }
    
    return 0;
}

/* POSIX clock_gettime implementation */
int clock_gettime(clockid_t clockid, struct timespec *tp)
{
    struct timeval tv;
    struct EClockVal ec;
    ULONG eclock_freq;
    int result;
    
    chkabort();
    
    if (tp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    switch (clockid) {
        case CLOCK_REALTIME:
            /* Use gettimeofday for system time */
            result = gettimeofday(&tv, NULL);
            if (result == 0) {
                tp->tv_sec = tv.tv_sec;
                tp->tv_nsec = tv.tv_usec * 1000;
            }
            return result;
            
        case CLOCK_MONOTONIC:
            /* Use E-Clock for monotonic time (highest precision) */
            if (timer_device_geteclock(&ec, &eclock_freq) == 0) {
                /* Convert E-Clock to nanoseconds */
                /* E-Clock frequency is in ticks per second */
                long_long_t ev_lo_ll, freq_ll, billion_ll, result_ll;
                
                tp->tv_sec = ec.ev_hi;
                
                /* Convert to longlong_t for calculation */
                ev_lo_ll = long_to_long_long(ec.ev_lo);
                freq_ll = long_to_long_long(eclock_freq);
                billion_ll = long_to_long_long(1000000000L);
                
                /* Calculate: (ec.ev_lo * 1000000000) / eclock_freq */
                result_ll = long_long_div(long_long_mul(ev_lo_ll, billion_ll), freq_ll);
                tp->tv_nsec = long_long_to_long(result_ll);
                
                return 0;
            }
            /* Fallback to system time */
            result = gettimeofday(&tv, NULL);
            if (result == 0) {
                tp->tv_sec = tv.tv_sec;
                tp->tv_nsec = tv.tv_usec * 1000;
            }
            return result;
            
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_THREAD_CPUTIME_ID:
            /* AmigaOS doesn't provide CPU time, return system time */
            result = gettimeofday(&tv, NULL);
            if (result == 0) {
                tp->tv_sec = tv.tv_sec;
                tp->tv_nsec = tv.tv_usec * 1000;
            }
            return result;
            
        default:
            errno = EINVAL;
            return -1;
    }
}

/* POSIX clock_settime implementation */
int clock_settime(clockid_t clockid, const struct timespec *tp)
{
    struct timeval tv;
    
    chkabort();
    
    if (tp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    switch (clockid) {
        case CLOCK_REALTIME:
            /* Convert to timeval and use settimeofday */
            tv.tv_sec = tp->tv_sec;
            tv.tv_usec = tp->tv_nsec / 1000;
            return settimeofday(&tv, NULL);
            
        case CLOCK_MONOTONIC:
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_THREAD_CPUTIME_ID:
            /* These clocks cannot be set */
            errno = EINVAL;
            return -1;
            
        default:
            errno = EINVAL;
            return -1;
    }
}

/* POSIX clock_getres implementation */
int clock_getres(clockid_t clockid, struct timespec *res)
{
    struct EClockVal ec;
    ULONG eclock_freq;
    
    chkabort();
    
    if (res == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    switch (clockid) {
        case CLOCK_REALTIME:
            /* AmigaOS system time resolution is 1 second */
            res->tv_sec = 1;
            res->tv_nsec = 0;
            break;
            
        case CLOCK_MONOTONIC:
            /* Use E-Clock resolution for highest precision */
            if (timer_device_geteclock(&ec, &eclock_freq) == 0) {
                /* E-Clock resolution is 1 tick */
                long_long_t freq_ll, billion_ll, result_ll;
                
                res->tv_sec = 0;
                
                /* Calculate: 1000000000 / eclock_freq */
                freq_ll = long_to_long_long(eclock_freq);
                billion_ll = long_to_long_long(1000000000L);
                result_ll = long_long_div(billion_ll, freq_ll);
                res->tv_nsec = long_long_to_long(result_ll);
            } else {
                /* Fallback to system time resolution */
                res->tv_sec = 1;
                res->tv_nsec = 0;
            }
            break;
            
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_THREAD_CPUTIME_ID:
            /* Use E-Clock resolution for CPU time */
            if (timer_device_geteclock(&ec, &eclock_freq) == 0) {
                long_long_t freq_ll, billion_ll, result_ll;
                
                res->tv_sec = 0;
                
                /* Calculate: 1000000000 / eclock_freq */
                freq_ll = long_to_long_long(eclock_freq);
                billion_ll = long_to_long_long(1000000000L);
                result_ll = long_long_div(billion_ll, freq_ll);
                res->tv_nsec = long_long_to_long(result_ll);
            } else {
                /* Fallback to microsecond resolution */
                res->tv_sec = 0;
                res->tv_nsec = 1000;  /* 1 microsecond */
            }
            break;
            
        default:
            errno = EINVAL;
            return -1;
    }
    
    return 0;
}

/* POSIX clock_nanosleep implementation */
int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *remain)
{
    struct timeval tv;
    int result;
    
    chkabort();
    
    if (request == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Convert to timeval for usleep */
    tv.tv_sec = request->tv_sec;
    tv.tv_usec = request->tv_nsec / 1000;
    
    /* Handle absolute time */
    if (flags & TIMER_ABSTIME) {
        struct timeval now;
        struct timeval target;
        
        if (clockid != CLOCK_REALTIME) {
            errno = EINVAL;
            return -1;
        }
        
        gettimeofday(&now, NULL);
        target.tv_sec = request->tv_sec;
        target.tv_usec = request->tv_nsec / 1000;
        
        if (timercmp(&target, &now, <=)) {
            return 0;  /* Already past target time */
        }
        
        tv.tv_sec = target.tv_sec - now.tv_sec;
        tv.tv_usec = target.tv_usec - now.tv_usec;
        if (tv.tv_usec < 0) {
            tv.tv_sec--;
            tv.tv_usec += 1000000;
        }
    }
    
    /* Use usleep for the actual sleep */
    result = usleep(tv.tv_sec * 1000000 + tv.tv_usec);
    
    /* Set remain to zero on success */
    if (result == 0 && remain != NULL) {
        remain->tv_sec = 0;
        remain->tv_nsec = 0;
    }
    
    return result;
}

/* POSIX nanosleep implementation */
int nanosleep(const struct timespec *request, struct timespec *remain)
{
    struct timeval tv;
    int result;
    
    /* Check for abort signal */
    __chkabort();
    
    if (request == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Convert to timeval for timer device */
    tv.tv_sec = request->tv_sec;
    tv.tv_usec = request->tv_nsec / 1000;
    
    /* Use E-Clock timer device for highest precision */
    result = timer_device_sleep(tv.tv_sec, tv.tv_usec, UNIT_ECLOCK);
    
    /* Set remain to zero on success */
    if (result == 0 && remain != NULL) {
        remain->tv_sec = 0;
        remain->tv_nsec = 0;
    }
    
    return result;
}

