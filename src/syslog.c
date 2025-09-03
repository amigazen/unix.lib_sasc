/*
 * syslog.c - system logging (POSIX compliant)
 *
 * This function provides system logging functionality by wrapping
 * the AmigaOS bsdsocket.library syslog functions.
 *
 * POSIX.1-2001, POSIX.1-2008, BSD 4.3
 */

#include "amiga.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>

/* Global variables for openlog/closelog/setlogmask */
static int log_facility = LOG_USER;
static int log_mask = 0xff;  /* Log all priorities */
static char *log_ident = NULL;
static int log_options = 0;

/* Facility constants - these should match sys/syslog.h */
#ifndef LOG_KERN
#define LOG_KERN     (0<<3)  /* kernel messages */
#define LOG_USER     (1<<3)  /* random user-level messages */
#define LOG_MAIL     (2<<3)  /* mail system */
#define LOG_DAEMON   (3<<3)  /* system daemons */
#define LOG_AUTH     (4<<3)  /* security/authorization messages */
#define LOG_SYSLOG   (5<<3)  /* messages generated internally by syslogd */
#define LOG_LPR      (6<<3)  /* line printer subsystem */
#define LOG_NEWS     (7<<3)  /* network news subsystem */
#define LOG_UUCP     (8<<3)  /* UUCP subsystem */
#define LOG_CRON     (9<<3)  /* clock daemon */
#define LOG_AUTHPRIV (10<<3) /* security/authorization messages (private) */
#define LOG_FTP      (11<<3) /* ftp daemon */
#define LOG_LOCAL0   (16<<3) /* reserved for local use */
#define LOG_LOCAL1   (17<<3) /* reserved for local use */
#define LOG_LOCAL2   (18<<3) /* reserved for local use */
#define LOG_LOCAL3   (19<<3) /* reserved for local use */
#define LOG_LOCAL4   (20<<3) /* reserved for local use */
#define LOG_LOCAL5   (21<<3) /* reserved for local use */
#define LOG_LOCAL6   (22<<3) /* reserved for local use */
#define LOG_LOCAL7   (23<<3) /* reserved for local use */
#endif

/* Option constants */
#ifndef LOG_PID
#define LOG_PID      0x01    /* log the pid with each message */
#define LOG_CONS     0x02    /* log on the console if errors in sending */
#define LOG_ODELAY   0x04    /* delay open until first syslog() (default) */
#define LOG_NDELAY   0x08    /* don't delay open */
#define LOG_NOWAIT   0x10    /* don't wait for console forks: DEPRECATED */
#define LOG_PERROR   0x20    /* log to stderr as well */
#endif

/* Priority mask macros */
#ifndef LOG_MASK
#define LOG_MASK(pri)    (1 << (pri))
#define LOG_UPTO(pri)    ((1 << ((pri)+1)) - 1)
#endif

/* Priority and facility extraction macros */
#ifndef LOG_PRI
#define LOG_PRI(p)   ((p) & LOG_PRIMASK)
#define LOG_FAC(p)   (((p) & LOG_FACMASK) >> 3)
#define LOG_PRIMASK  0x07
#define LOG_FACMASK  0x03f8
#endif

void openlog(const char *ident, int option, int facility)
{
    __chkabort();
    
    if (ident) {
        if (log_ident) {
            free(log_ident);
        }
        log_ident = strdup(ident);
    }
    
    log_options = option;
    log_facility = facility;
}

void closelog(void)
{
    __chkabort();
    
    if (log_ident) {
        free(log_ident);
        log_ident = NULL;
    }
}

int setlogmask(int mask)
{
    int old_mask = log_mask;
    log_mask = mask;
    return old_mask;
}

void syslog(int priority, const char *format, ...)
{
    va_list args;
    int final_priority;
    
    __chkabort();
    
    /* Check if this priority should be logged */
    if (!(LOG_MASK(LOG_PRI(priority)) & log_mask)) {
        return;
    }
    
    /* If no facility specified, use the default from openlog */
    if ((priority & LOG_FACMASK) == 0) {
        final_priority = priority | log_facility;
    } else {
        final_priority = priority;
    }
    
    /* Call the AmigaOS bsdsocket.library syslog function */
    va_start(args, format);
    vsyslog(final_priority, (char *)format, args);
    va_end(args);
}
