/*
 * nice.c - change process priority (POSIX compliant)
 *
 * This function adds inc to the nice value of the calling process.
 * A higher nice value means a lower priority. On AmigaOS, this
 * uses SetTaskPri() to adjust the task priority.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include "amiga.h"
#include <unistd.h>
#include <errno.h>
#include <proto/exec.h>

int nice(int inc)
{
    LONG current_pri, new_pri;
    
    chkabort();
    
    /* Get current task priority */
    current_pri = FindTask(NULL)->tc_Node.ln_Pri;
    
    /* Calculate new priority (higher nice = lower priority) */
    new_pri = current_pri - inc;
    
    /* Set new priority */
    if (SetTaskPri(FindTask(NULL), new_pri)) {
        return 0;
    } else {
        errno = EPERM;
        return -1;
    }
}
