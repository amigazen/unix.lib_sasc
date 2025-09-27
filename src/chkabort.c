/*
 * SPDX-License-Identifier: BSD-2-Clause
 * chkabort.c - Check for abort signals
 *
 * Based on PDC Chk_Abort.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <libraries/dos.h>
#include <proto/dos.h>

#define ABORTSTATE (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D)

int Enable_Abort = 1;

/**
 * @brief Internal function to check for abort signals
 * 
 * This is the internal implementation that actually checks for break signals.
 * Based on PDC IOLib/misc/Chk_Abort.c
 * 
 * Note: This function returns void as per sclib.guide specification
 */
void __regargs __chkabort(void)
{
    long status;
    extern long SetSignal(long, long);

    /* Check for break signals using SetSignal */
    status = SetSignal(0L, ABORTSTATE);
    
    /* If break signal detected and enabled, call abort */
    if ((status & ABORTSTATE) && Enable_Abort) {
        extern void abort(void);
        abort();
    }
}

/**
 * @brief Check for abort signals (public API)
 * 
 * The Chk_Abort() function checks for break signals (Ctrl+C, Ctrl+D).
 * If a break signal is detected and Enable_Abort is true, it calls abort().
 * 
 * This implementation:
 * - Uses SetSignal() directly for signal checking
 * - Calls abort() if break signal detected and enabled
 * - Returns void as per sclib.guide specification
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void Chk_Abort(void)
{
    long status;
    extern long SetSignal(long, long);
    extern void abort(void);

    /* Check for break signals using SetSignal */
    status = SetSignal(0L, ABORTSTATE);
    
    if ((status & ABORTSTATE) && Enable_Abort) {
        abort();
    }
}

/**
 * @brief Check for abort signals (public API)
 * 
 * The chkabort() function is the newer name for Chk_Abort().
 * It provides the same functionality but with a simpler interface.
 */
void chkabort(void)
{
    Chk_Abort();
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
