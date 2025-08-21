#ifndef _UTIME_H
#define _UTIME_H

struct utimbuf { time_t actime, modtime; };

int utime(char *path, struct utimbuf *times);

#endif /* _UTIME_H */
