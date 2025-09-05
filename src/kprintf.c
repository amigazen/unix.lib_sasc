/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * kprintf.c - Kernel printf implementation using KPrintF()
 * 
 * This function provides kernel debug output using Amiga's KPrintF() function
 * from debug.lib, which is a formatted printf function for kernel debug output.
 * 
 * C89 compliant for AmigaOS compatibility
 */

#include "amiga.h"
#include <clib/debug_protos.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * _kprintf - Kernel printf using KPrintF()
 * 
 * This function provides formatted output to the kernel debug output
 * using Amiga's KPrintF() function from debug.lib.
 * 
 * @param format Format string (same as printf)
 * @param ... Variable arguments
 * @return Number of characters written, or -1 on error
 */
int _kprintf(const char *format, ...)
{
    va_list args;
    int result = 0;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate format string */
    if (format == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Use KPrintF() directly - it's a formatted printf function */
    /* KPrintF() returns void, so we can't get the actual character count */
    va_start(args, format);
    KPrintF((CONST_STRPTR)format, args);
    va_end(args);
    
    /* Since KPrintF() doesn't return character count, we estimate it */
    /* This is a limitation of the Amiga API */
    result = strlen(format); /* Rough estimate */
    
    return result;
}

/*
 * vkprintf - Kernel printf with va_list
 * 
 * This function provides formatted output to the kernel debug output
 * using Amiga's KPrintF() function with a va_list argument.
 * 
 * @param format Format string (same as printf)
 * @param args Variable arguments list
 * @return Number of characters written, or -1 on error
 */
int vkprintf(const char *format, va_list args)
{
    int result = 0;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate format string */
    if (format == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* Use KPrintF() directly - it's a formatted printf function */
    /* KPrintF() returns void, so we can't get the actual character count */
    KPrintF((CONST_STRPTR)format, args);
    
    /* Since KPrintF() doesn't return character count, we estimate it */
    /* This is a limitation of the Amiga API */
    result = strlen(format); /* Rough estimate */
    
    return result;
}