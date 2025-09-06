/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * spawn.h - POSIX process spawning interface
 * 
 * This header defines the POSIX.1-2008 posix_spawn() interface
 * for creating new processes on AmigaOS.
 * 
 * POSIX.1-2008 compliant for AmigaOS compatibility
 * C89 compliant for AmigaOS compatibility
 */

#ifndef _SPAWN_H
#define _SPAWN_H

#include "amiga.h"
#include <sys/types.h>
#include <signal.h>
#include <sched.h>

/* Define restrict as nothing for C89 compatibility */
#ifndef restrict
#define restrict
#endif

/* POSIX spawn flags */
#define POSIX_SPAWN_RESETIDS        0x01
#define POSIX_SPAWN_SETPGROUP       0x02
#define POSIX_SPAWN_SETSIGMASK      0x04
#define POSIX_SPAWN_SETSIGDEF       0x08
#define POSIX_SPAWN_SETSID          0x10
#define POSIX_SPAWN_USEVFORK        0x20
#define POSIX_SPAWN_SETSCHEDPARAM   0x40
#define POSIX_SPAWN_SETSCHEDULER    0x80

/* POSIX spawn file actions */
typedef struct {
    int action;
    int fildes;
    int oflag;
    mode_t mode;
    char *path;
    int newfildes;
    struct spawn_file_actions *next;
} spawn_file_action_t;

typedef struct spawn_file_actions {
    spawn_file_action_t *head;
    spawn_file_action_t *tail;
} posix_spawn_file_actions_t;

/* POSIX spawn attributes */
typedef struct {
    short flags;
    pid_t pgroup;
    sigset_t sigmask;
    sigset_t sigdefault;
    struct sched_param schedparam;
    int schedpolicy;
    posix_spawn_file_actions_t *file_actions;
} posix_spawnattr_t;

/* Function prototypes */
int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict],
                char *const envp[restrict]);

int posix_spawnp(pid_t *restrict pid, const char *restrict file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *restrict attrp,
                 char *const argv[restrict],
                 char *const envp[restrict]);

/* File action functions */
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions);
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *file_actions,
                                     int fildes, const char *path, int oflag,
                                     mode_t mode);
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions,
                                      int fildes);
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions,
                                     int fildes, int newfildes);

/* Attribute functions */
int posix_spawnattr_init(posix_spawnattr_t *attrp);
int posix_spawnattr_destroy(posix_spawnattr_t *attrp);
int posix_spawnattr_getflags(const posix_spawnattr_t *restrict attrp,
                             short *restrict flags);
int posix_spawnattr_setflags(posix_spawnattr_t *attrp, short flags);
int posix_spawnattr_getpgroup(const posix_spawnattr_t *restrict attrp,
                              pid_t *restrict pgroup);
int posix_spawnattr_setpgroup(posix_spawnattr_t *attrp, pid_t pgroup);
int posix_spawnattr_getsigmask(const posix_spawnattr_t *restrict attrp,
                               sigset_t *restrict sigmask);
int posix_spawnattr_setsigmask(posix_spawnattr_t *attrp,
                               const sigset_t *sigmask);
int posix_spawnattr_getsigdefault(const posix_spawnattr_t *restrict attrp,
                                  sigset_t *restrict sigdefault);
int posix_spawnattr_setsigdefault(posix_spawnattr_t *attrp,
                                  const sigset_t *sigdefault);
int posix_spawnattr_getschedparam(const posix_spawnattr_t *restrict attrp,
                                  struct sched_param *restrict schedparam);
int posix_spawnattr_setschedparam(posix_spawnattr_t *attrp,
                                  const struct sched_param *schedparam);
int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *restrict attrp,
                                   int *restrict schedpolicy);
int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attrp, int schedpolicy);

#endif /* _SPAWN_H */
