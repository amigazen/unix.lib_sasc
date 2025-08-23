#ifndef _SYSLOG_H_
#define _SYSLOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* --- Priorities (severities) --- */
#define LOG_EMERG   0 /* System is unusable */
#define LOG_ALERT   1 /* Action must be taken immediately */
#define LOG_CRIT    2 /* Critical conditions */
#define LOG_ERR     3 /* Error conditions */
#define LOG_WARNING 4 /* Warning conditions */
#define LOG_NOTICE  5 /* Normal but significant condition */
#define LOG_INFO    6 /* Informational */
#define LOG_DEBUG   7 /* Debug-level messages */

/* --- Facilities --- */
#define LOG_KERN    (0<<3)  /* Kernel messages */
#define LOG_USER    (1<<3)  /* Random user-level messages */
#define LOG_MAIL    (2<<3)  /* Mail system */
#define LOG_DAEMON  (3<<3)  /* System daemons */
#define LOG_AUTH    (4<<3)  /* Security/authorization messages */
#define LOG_SYSLOG  (5<<3)  /* Messages generated internally by syslogd */
#define LOG_LPR     (6<<3)  /* Line printer subsystem */
#define LOG_NEWS    (7<<3)  /* Network news subsystem */
#define LOG_UUCP    (8<<3)  /* UUCP subsystem */
#define LOG_CRON    (9<<3)  /* Clock daemon */
#define LOG_LOCAL0  (16<<3) /* Reserved for local use */
#define LOG_LOCAL1  (17<<3)
#define LOG_LOCAL2  (18<<3)
#define LOG_LOCAL3  (19<<3)
#define LOG_LOCAL4  (20<<3)
#define LOG_LOCAL5  (21<<3)
#define LOG_LOCAL6  (22<<3)
#define LOG_LOCAL7  (23<<3)

/* --- Options for openlog() --- */
#define LOG_PID     0x01    /* Log the pid with each message. */
#define LOG_CONS    0x02    /* Log to console if errors connecting to daemon. */
#define LOG_NDELAY  0x08    /* Connect immediately. */
#define LOG_NOWAIT  0x10    /* Don't wait for child processes. */

/* --- C89-compliant function prototypes --- */
void closelog(void);
void openlog(const char *ident, int option, int facility);
int  setlogmask(int mask);
void syslog(int priority, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* !_SYSLOG_H_ */
