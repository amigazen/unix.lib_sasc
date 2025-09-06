#include "amiga.h"
#include "signals.h"
#include "processes.h"
#include <proto/exec.h>

int getpid(void)
{
    struct Process *proc;
    
    __chkabort();
    
    /* Get current process and use address as PID (consistent with vfork) */
    proc = (struct Process *)FindTask(NULL);
    if (proc == NULL) {
        return _our_pid; /* Fallback to old method */
    }
    
    return (int)((ULONG)proc & 0xFFFF);
}
