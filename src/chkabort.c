/*
 * SPDX-License-Identifier: BSD-2-Clause
 * chkabort.c - Check for abort signals
 *
 * Based on PDC Chk_Abort.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <libraries/dos.h>

#define ABORTSTATE (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D)

int Enable_Abort = 1;

/**
 * @brief Check for abort signals
 * @return Signal status
 * 
 * The Chk_Abort() function checks for break signals (Ctrl+C, Ctrl+D).
 * If a break signal is detected and Enable_Abort is true, it calls abort().
 * 
 * This implementation:
 * - Uses Amiga SetSignal() to check for break signals
 * - Calls abort() if break signal detected and enabled
 * - Returns the current signal status
 * - Is C89 compliant for SAS/C compiler compatibility
 */
long Chk_Abort(void)
{
    long status;
    extern long SetSignal(long, long);
    extern void abort(void);

    if (((status = SetSignal(0L, ABORTSTATE)) & ABORTSTATE) && Enable_Abort) {
        abort();
    }

    return status;
}

#else
/* Empty - function provided by SAS/C sc.lib */
#endif
