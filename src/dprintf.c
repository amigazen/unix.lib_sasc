/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * dprintf.c - Debug printf implementation using KPrintF()
 * 
 * This function provides debug output using Amiga's KPrintF() function
 * for kernel/debug output.
 * 
 * C89 compliant for AmigaOS compatibility
 */

#include "amiga.h"
#include <clib/debug_protos.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

/* Forward declaration */
extern int _kprintf(const char *format, ...);

/*
 * dprintf - Debug printf using KPrintF()
 * 
 * This function writes formatted output to the kernel debug output
 * using Amiga's KPrintF() function.
 * 
 * @param fd File descriptor (ignored, always uses kernel debug output)
 * @param format Format string (same as printf)
 * @param ... Variable arguments
 * @return Number of characters written, or -1 on error
 */
int dprintf(int fd, const char *format, ...)
{
    va_list args;
    int result;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate format string */
    if (format == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Use _kprintf for the actual formatting and output */
    va_start(args, format);
    result = _kprintf(format, args);
    va_end(args);
    
    return result;
}
