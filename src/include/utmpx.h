/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * utmpx.h - User accounting
 * 
 * This header provides user accounting functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _UTMPX_H
#define _UTMPX_H 1

#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* User accounting types */
#define EMPTY           0   /* No valid user accounting information */
#define RUN_LVL         1   /* System run level */
#define BOOT_TIME       2   /* System boot time */
#define NEW_TIME        3   /* System clock changed */
#define OLD_TIME        4   /* System clock changed */
#define INIT_PROCESS    5   /* Process spawned by init */
#define LOGIN_PROCESS   6   /* Session leader process for user login */
#define USER_PROCESS    7   /* Normal user process */
#define DEAD_PROCESS    8   /* Terminated process */
#define ACCOUNTING      9   /* Accounting */

/* User accounting structure */
struct utmpx {
    char ut_user[32];       /* User login name */
    char ut_id[4];          /* Inittab ID */
    char ut_line[32];       /* Device name */
    pid_t ut_pid;           /* Process ID */
    short ut_type;          /* Type of entry */
    struct timeval ut_tv;   /* Time entry was made */
    char ut_host[256];      /* Host name */
    char ut_pad[64];        /* Reserved for future use */
};

/* Function prototypes */
void setutxent(void);
struct utmpx *getutxent(void);
struct utmpx *getutxid(const struct utmpx *id);
struct utmpx *getutxline(const struct utmpx *line);
struct utmpx *pututxline(const struct utmpx *utmpx);
void endutxent(void);
int utmpxname(const char *file);

/* Legacy compatibility */
#define ut_name ut_user
#define ut_xtime ut_tv.tv_sec
#define ut_addr ut_pad

/* Convenience macros */
#define UTMPX_FILE "/var/log/utx.log"
#define WTMPX_FILE "/var/log/wtmp"

#ifdef __cplusplus
}
#endif

#endif /* _UTMPX_H */
