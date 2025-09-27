#include "amiga.h"
#include <signal.h>

extern void _close_all(void);
extern void __regargs __chkabort(void);

void __saveds abort(void)
{
    __chkabort();
    _close_all();
    kill(getpid(), SIGIOT);
}
