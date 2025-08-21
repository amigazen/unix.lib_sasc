#ifndef _UNISTD_H_
#define _UNISTD_H_

/* For size_t, ssize_t, uid_t, gid_t, etc. */
#include <sys/types.h>

/* For NULL */
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Standard File Descriptors --- */
#define STDIN_FILENO   0   /* Standard input file descriptor */
#define STDOUT_FILENO  1   /* Standard output file descriptor */
#define STDERR_FILENO  2   /* Standard error file descriptor */

/* --- Constants for lseek() --- */
#define SEEK_SET    0   /* Set file offset to offset */
#define SEEK_CUR    1   /* Set file offset to current plus offset */
#define SEEK_END    2   /* Set file offset to EOF plus offset */

/* --- POSIX Version --- */
/* Identifies the version of the POSIX standard implemented (1990) */
#define _POSIX_VERSION 199009L

/* --- Constants for pathconf() and fpathconf() --- */
#define _PC_LINK_MAX        1
#define _PC_MAX_CANON       2
#define _PC_MAX_INPUT       3
#define _PC_NAME_MAX        4
#define _PC_PATH_MAX        5
#define _PC_PIPE_BUF        6
#define _PC_CHOWN_RESTRICTED 7
#define _PC_NO_TRUNC        8
#define _PC_VDISABLE        9

/* --- Constants for sysconf() --- */
#define _SC_ARG_MAX         1
#define _SC_CHILD_MAX       2
#define _SC_CLK_TCK         3
#define _SC_NGROUPS_MAX     4
#define _SC_OPEN_MAX        5
#define _SC_JOB_CONTROL     6
#define _SC_SAVED_IDS       7
#define _SC_VERSION         8

/* --- POSIX Standard Function Prototypes (C89 Style) --- */

/* Process Control */
void     _exit(int status);
pid_t    fork(void);
int      execl(const char *path, const char *arg0, ...);
int      execle(const char *path, const char *arg0, ...);
int      execlp(const char *file, const char *arg0, ...);
int      execv(const char *path, char *const argv[]);
int      execve(const char *path, char *const argv[], char *const envp[]);
int      execvp(const char *file, char *const argv[]);
pid_t    getpid(void);
pid_t    getppid(void);
pid_t    getpgrp(void);
int      setpgid(pid_t pid, pid_t pgid);
pid_t    setsid(void);

/* User/Group Management */
uid_t    getuid(void);
uid_t    geteuid(void);
gid_t    getgid(void);
gid_t    getegid(void);
int      setuid(uid_t uid);
int      setgid(gid_t gid);
int      getgroups(int gidsetsize, gid_t grouplist[]);
char    *getlogin(void);

/* File and Directory Management */
int      access(const char *path, int amode);
int      chdir(const char *path);
int      chown(const char *path, uid_t owner, gid_t group);
char    *getcwd(char *buf, size_t size);
int      rmdir(const char *path);
int      link(const char *oldpath, const char *newpath);
int      unlink(const char *path);

/* File Descriptor I/O */
int      close(int fildes);
off_t    lseek(int fildes, off_t offset, int whence);
ssize_t  read(int fildes, void *buf, size_t nbyte);
ssize_t  write(int fildes, const void *buf, size_t nbyte);
int      dup(int oldfildes);
int      dup2(int oldfildes, int newfildes);
int      pipe(int fildes[2]);
int      isatty(int fildes);
char    *ttyname(int fildes);

/* System Configuration */
long     fpathconf(int fildes, int name);
long     pathconf(const char *path, int name);
long     sysconf(int name);

/* Other */
unsigned int alarm(unsigned int seconds);
int          pause(void);
unsigned int sleep(unsigned int seconds);

#ifdef __cplusplus
}
#endif

#endif /* !_UNISTD_H_ */