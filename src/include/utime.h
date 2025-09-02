struct utimbuf { time_t actime, modtime; };

int utime(const char *path, const struct utimbuf *times);
