#include <stdlib.h>
#include <time.h>
#include <utime.h>

#include "amiga.h"
#include "timeconvert.h"

int utime(char *file, struct utimbuf *times)
{
  struct DateStamp date;
  time_t mtime;

  chkabort();
  if (times) mtime = times->modtime;
  else mtime = time(0);

  if (mtime == -1) return 0;

  _gmt2amiga(mtime, &date);

  if (SetFileDate(file, &date)) return 0;
  ERROR;
}
