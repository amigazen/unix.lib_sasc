/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * posix_spawn.c - POSIX process spawning implementation
 * 
 * This file provides POSIX.1-2008 posix_spawn() and posix_spawnp() functions
 * by wrapping the existing exec() function for AmigaOS compatibility.
 * 
 * POSIX.1-2008, C89 compliant for AmigaOS compatibility
 */

#include "amiga.h"
#include <spawn.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>

/* Forward declarations */
static int build_command_line(char **argv, char *buffer, int bufsize);
static int apply_file_actions(const posix_spawn_file_actions_t *file_actions);
static int apply_attributes(const posix_spawnattr_t *attrp);
static int find_executable(const char *file, char *path, size_t pathsize);
static void cleanup_file_actions(posix_spawn_file_actions_t *file_actions);

/*
 * posix_spawn - spawn a new process
 * 
 * Creates a new process by executing the specified program.
 * This function wraps the existing exec() function for AmigaOS compatibility.
 */
int posix_spawn(pid_t *restrict pid, const char *restrict path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *restrict attrp,
                char *const argv[restrict],
                char *const envp[restrict])
{
    char command_line[1024];
    char *program_path;
    int result;
    int input_fd = -1;
    int output_fd = -1;
    int error_fd = -1;
    char *work_dir = NULL;
    int stack_size = 0;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (pid == NULL || path == NULL || argv == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Apply file actions if provided */
    if (file_actions != NULL) {
        result = apply_file_actions(file_actions);
        if (result != 0) {
            return result;
        }
    }
    
    /* Apply attributes if provided */
    if (attrp != NULL) {
        result = apply_attributes(attrp);
        if (result != 0) {
            return result;
        }
    }
    
    /* Build command line from argv */
    if (build_command_line((char **)argv, command_line, sizeof(command_line)) != 0) {
        errno = E2BIG;
        return -1;
    }
    
    /* Allocate memory for program path */
    program_path = (char *)AllocMem(strlen(path) + 1, MEMF_PUBLIC);
    if (program_path == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    strcpy(program_path, path);
    
    /* Execute the program using exec() */
    result = exec(program_path, argv, input_fd, output_fd, work_dir, stack_size);
    
    /* Free allocated memory */
    FreeMem(program_path, strlen(path) + 1);
    
    if (result == -1) {
        /* exec() failed, return appropriate error */
        return errno;
    }
    
    /* Store the process ID */
    *pid = result;
    
    return 0;
}

/*
 * posix_spawnp - spawn a new process using PATH search
 * 
 * Creates a new process by executing the specified program,
 * searching for it in the PATH environment variable.
 */
int posix_spawnp(pid_t *restrict pid, const char *restrict file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *restrict attrp,
                 char *const argv[restrict],
                 char *const envp[restrict])
{
    char full_path[512];
    int result;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (pid == NULL || file == NULL || argv == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* If file contains '/', use it as-is */
    if (strchr(file, '/') != NULL) {
        return posix_spawn(pid, file, file_actions, attrp, argv, envp);
    }
    
    /* Search for executable in PATH */
    result = find_executable(file, full_path, sizeof(full_path));
    if (result != 0) {
        return result;
    }
    
    /* Spawn the found executable */
    return posix_spawn(pid, full_path, file_actions, attrp, argv, envp);
}

/*
 * build_command_line - Build command line string from argv
 */
static int build_command_line(char **argv, char *buffer, int bufsize)
{
    char *bp = buffer;
    int remaining = bufsize - 1;
    int i;
    
    /* Start with empty string */
    *bp = '\0';
    
    /* Build command line by concatenating argv elements */
    for (i = 0; argv[i] != NULL; i++) {
        int len;
        len = strlen(argv[i]);
        
        /* Check if we have enough space */
        if (len + 2 > remaining) { /* +2 for space and null terminator */
            return -1;
        }
        
        /* Add space between arguments (except for first) */
        if (i > 0) {
            *bp++ = ' ';
            remaining--;
        }
        
        /* Add the argument */
        strcpy(bp, argv[i]);
        bp += len;
        remaining -= len;
    }
    
    return 0;
}

/*
 * apply_file_actions - Apply file actions to current process
 */
static int apply_file_actions(const posix_spawn_file_actions_t *file_actions)
{
    spawn_file_action_t *action;
    
    if (file_actions == NULL) {
        return 0;
    }
    
    /* Process each file action */
    for (action = file_actions->head; action != NULL; action = action->next) {
        switch (action->action) {
            case 1: /* ADD_OPEN */
                /* Open file and duplicate to specified fd */
                {
                    int fd;
                    fd = open(action->path, action->oflag, action->mode);
                    if (fd == -1) {
                        return errno;
                    }
                    if (dup2(fd, action->fildes) == -1) {
                        close(fd);
                        return errno;
                    }
                    close(fd);
                }
                break;
                
            case 2: /* ADD_CLOSE */
                /* Close specified file descriptor */
                if (close(action->fildes) == -1) {
                    return errno;
                }
                break;
                
            case 3: /* ADD_DUP2 */
                /* Duplicate file descriptor */
                if (dup2(action->fildes, action->newfildes) == -1) {
                    return errno;
                }
                break;
                
            default:
                errno = EINVAL;
                return -1;
        }
    }
    
    return 0;
}

/*
 * apply_attributes - Apply spawn attributes to current process
 */
static int apply_attributes(const posix_spawnattr_t *attrp)
{
    if (attrp == NULL) {
        return 0;
    }
    
    /* Apply signal mask if specified */
    if (attrp->flags & POSIX_SPAWN_SETSIGMASK) {
        if (sigprocmask(SIG_SETMASK, &attrp->sigmask, NULL) == -1) {
            return errno;
        }
    }
    
    /* Apply signal default actions if specified */
    if (attrp->flags & POSIX_SPAWN_SETSIGDEF) {
        /* On AmigaOS, we can't easily set signal defaults */
        /* This would require more sophisticated signal handling */
        errno = ENOSYS;
        return -1;
    }
    
    /* Apply process group if specified */
    if (attrp->flags & POSIX_SPAWN_SETPGROUP) {
        if (setpgid(0, attrp->pgroup) == -1) {
            return errno;
        }
    }
    
    /* Apply scheduling parameters if specified */
    if (attrp->flags & POSIX_SPAWN_SETSCHEDPARAM) {
        if (sched_setparam(0, &attrp->schedparam) == -1) {
            return errno;
        }
    }
    
    /* Apply scheduler policy if specified */
    if (attrp->flags & POSIX_SPAWN_SETSCHEDULER) {
        if (sched_setscheduler(0, attrp->schedpolicy, &attrp->schedparam) == -1) {
            return errno;
        }
    }
    
    return 0;
}

/*
 * find_executable - Find executable in PATH
 */
static int find_executable(const char *file, char *path, size_t pathsize)
{
    char *path_env;
    char *path_copy;
    char *dir;
    char *next;
    int result = ENOENT;
    
    /* Get PATH environment variable */
    path_env = getenv("PATH");
    if (path_env == NULL) {
        /* Default PATH if not set */
        path_env = "/bin:/usr/bin:/usr/local/bin";
    }
    
    /* Make a copy of PATH for tokenization */
    path_copy = (char *)AllocMem(strlen(path_env) + 1, MEMF_PUBLIC);
    if (path_copy == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    strcpy(path_copy, path_env);
    
    /* Search each directory in PATH */
    dir = path_copy;
    while (dir != NULL) {
        next = strchr(dir, ':');
        if (next != NULL) {
            *next = '\0';
            next++;
        }
        
        /* Build full path */
        if (strlen(dir) + strlen(file) + 2 < pathsize) {
            strcpy(path, dir);
            if (path[strlen(path) - 1] != '/') {
                strcat(path, "/");
            }
            strcat(path, file);
            
            /* Check if file exists and is executable */
            if (access(path, X_OK) == 0) {
                result = 0;
                break;
            }
        }
        
        dir = next;
    }
    
    FreeMem(path_copy, strlen(path_env) + 1);
    
    if (result != 0) {
        errno = result;
        return -1;
    }
    
    return 0;
}

/*
 * posix_spawn_file_actions_init - Initialize file actions structure
 */
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions)
{
    if (file_actions == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    file_actions->head = NULL;
    file_actions->tail = NULL;
    
    return 0;
}

/*
 * posix_spawn_file_actions_destroy - Destroy file actions structure
 */
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions)
{
    if (file_actions == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    cleanup_file_actions(file_actions);
    
    return 0;
}

/*
 * posix_spawn_file_actions_addopen - Add open action to file actions
 */
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *file_actions,
                                     int fildes, const char *path, int oflag,
                                     mode_t mode)
{
    spawn_file_action_t *action;
    
    if (file_actions == NULL || path == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Allocate new action */
    action = (spawn_file_action_t *)AllocMem(sizeof(spawn_file_action_t), MEMF_PUBLIC);
    if (action == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Initialize action */
    action->action = 1; /* ADD_OPEN */
    action->fildes = fildes;
    action->oflag = oflag;
    action->mode = mode;
    action->path = (char *)AllocMem(strlen(path) + 1, MEMF_PUBLIC);
    if (action->path == NULL) {
        FreeMem(action, sizeof(spawn_file_action_t));
        errno = ENOMEM;
        return -1;
    }
    strcpy(action->path, path);
    action->next = NULL;
    
    /* Add to list */
    if (file_actions->head == NULL) {
        file_actions->head = action;
        file_actions->tail = action;
    } else {
        file_actions->tail->next = action;
        file_actions->tail = action;
    }
    
    return 0;
}

/*
 * posix_spawn_file_actions_addclose - Add close action to file actions
 */
int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions,
                                      int fildes)
{
    spawn_file_action_t *action;
    
    if (file_actions == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Allocate new action */
    action = (spawn_file_action_t *)AllocMem(sizeof(spawn_file_action_t), MEMF_PUBLIC);
    if (action == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Initialize action */
    action->action = 2; /* ADD_CLOSE */
    action->fildes = fildes;
    action->next = NULL;
    
    /* Add to list */
    if (file_actions->head == NULL) {
        file_actions->head = action;
        file_actions->tail = action;
    } else {
        file_actions->tail->next = action;
        file_actions->tail = action;
    }
    
    return 0;
}

/*
 * posix_spawn_file_actions_adddup2 - Add dup2 action to file actions
 */
int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions,
                                     int fildes, int newfildes)
{
    spawn_file_action_t *action;
    
    if (file_actions == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Allocate new action */
    action = (spawn_file_action_t *)AllocMem(sizeof(spawn_file_action_t), MEMF_PUBLIC);
    if (action == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    /* Initialize action */
    action->action = 3; /* ADD_DUP2 */
    action->fildes = fildes;
    action->newfildes = newfildes;
    action->next = NULL;
    
    /* Add to list */
    if (file_actions->head == NULL) {
        file_actions->head = action;
        file_actions->tail = action;
    } else {
        file_actions->tail->next = action;
        file_actions->tail = action;
    }
    
    return 0;
}

/*
 * cleanup_file_actions - Clean up file actions structure
 */
static void cleanup_file_actions(posix_spawn_file_actions_t *file_actions)
{
    spawn_file_action_t *action;
    spawn_file_action_t *next;
    
    if (file_actions == NULL) {
        return;
    }
    
    action = file_actions->head;
    while (action != NULL) {
        next = action->next;
        
        if (action->path != NULL) {
            FreeMem(action->path, strlen(action->path) + 1);
        }
        
        FreeMem(action, sizeof(spawn_file_action_t));
        action = next;
    }
    
    file_actions->head = NULL;
    file_actions->tail = NULL;
}

/*
 * posix_spawnattr_init - Initialize spawn attributes structure
 */
int posix_spawnattr_init(posix_spawnattr_t *attrp)
{
    if (attrp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    attrp->flags = 0;
    attrp->pgroup = 0;
    sigemptyset(&attrp->sigmask);
    sigemptyset(&attrp->sigdefault);
    attrp->schedparam.sched_priority = 0;
    attrp->schedpolicy = SCHED_OTHER;
    attrp->file_actions = NULL;
    
    return 0;
}

/*
 * posix_spawnattr_destroy - Destroy spawn attributes structure
 */
int posix_spawnattr_destroy(posix_spawnattr_t *attrp)
{
    if (attrp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Clean up file actions if they exist */
    if (attrp->file_actions != NULL) {
        posix_spawn_file_actions_destroy(attrp->file_actions);
        FreeMem(attrp->file_actions, sizeof(posix_spawn_file_actions_t));
        attrp->file_actions = NULL;
    }
    
    return 0;
}

/*
 * posix_spawnattr_getflags - Get spawn flags
 */
int posix_spawnattr_getflags(const posix_spawnattr_t *restrict attrp,
                             short *restrict flags)
{
    if (attrp == NULL || flags == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    *flags = attrp->flags;
    return 0;
}

/*
 * posix_spawnattr_setflags - Set spawn flags
 */
int posix_spawnattr_setflags(posix_spawnattr_t *attrp, short flags)
{
    if (attrp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    attrp->flags = flags;
    return 0;
}

/*
 * posix_spawnattr_getpgroup - Get process group
 */
int posix_spawnattr_getpgroup(const posix_spawnattr_t *restrict attrp,
                              pid_t *restrict pgroup)
{
    if (attrp == NULL || pgroup == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    *pgroup = attrp->pgroup;
    return 0;
}

/*
 * posix_spawnattr_setpgroup - Set process group
 */
int posix_spawnattr_setpgroup(posix_spawnattr_t *attrp, pid_t pgroup)
{
    if (attrp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    attrp->pgroup = pgroup;
    return 0;
}

/*
 * posix_spawnattr_getsigmask - Get signal mask
 */
int posix_spawnattr_getsigmask(const posix_spawnattr_t *restrict attrp,
                               sigset_t *restrict sigmask)
{
    if (attrp == NULL || sigmask == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    *sigmask = attrp->sigmask;
    return 0;
}

/*
 * posix_spawnattr_setsigmask - Set signal mask
 */
int posix_spawnattr_setsigmask(posix_spawnattr_t *attrp,
                               const sigset_t *sigmask)
{
    if (attrp == NULL || sigmask == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    attrp->sigmask = *sigmask;
    return 0;
}

/*
 * posix_spawnattr_getsigdefault - Get signal default actions
 */
int posix_spawnattr_getsigdefault(const posix_spawnattr_t *restrict attrp,
                                  sigset_t *restrict sigdefault)
{
    if (attrp == NULL || sigdefault == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    *sigdefault = attrp->sigdefault;
    return 0;
}

/*
 * posix_spawnattr_setsigdefault - Set signal default actions
 */
int posix_spawnattr_setsigdefault(posix_spawnattr_t *attrp,
                                  const sigset_t *sigdefault)
{
    if (attrp == NULL || sigdefault == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    attrp->sigdefault = *sigdefault;
    return 0;
}

/*
 * posix_spawnattr_getschedparam - Get scheduling parameters
 */
int posix_spawnattr_getschedparam(const posix_spawnattr_t *restrict attrp,
                                  struct sched_param *restrict schedparam)
{
    if (attrp == NULL || schedparam == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    *schedparam = attrp->schedparam;
    return 0;
}

/*
 * posix_spawnattr_setschedparam - Set scheduling parameters
 */
int posix_spawnattr_setschedparam(posix_spawnattr_t *attrp,
                                  const struct sched_param *schedparam)
{
    if (attrp == NULL || schedparam == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    attrp->schedparam = *schedparam;
    return 0;
}

/*
 * posix_spawnattr_getschedpolicy - Get scheduling policy
 */
int posix_spawnattr_getschedpolicy(const posix_spawnattr_t *restrict attrp,
                                   int *restrict schedpolicy)
{
    if (attrp == NULL || schedpolicy == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    *schedpolicy = attrp->schedpolicy;
    return 0;
}

/*
 * posix_spawnattr_setschedpolicy - Set scheduling policy
 */
int posix_spawnattr_setschedpolicy(posix_spawnattr_t *attrp, int schedpolicy)
{
    if (attrp == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    attrp->schedpolicy = schedpolicy;
    return 0;
}
