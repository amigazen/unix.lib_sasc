/*
 * SPDX-License-Identifier: BSD-2-Clause
 * wildcard_pattern.c - Wildcard pattern matching and list filling
 *
 * Copyright (C) 2025 by amigazen project
 *
 * This file provides functions for wildcard pattern matching and list management.
 */

#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <pragmas/dos_pragmas.h>
#include "include/amiga.h"



/*
 * Fill the list with program arguments starting from a specific index
 *
 * Parameters:
 *   argv      - Array of command line arguments
 *   argstart  - Starting index in argv
 *   argcount  - Number of arguments to process
 *   strl      - Pointer to string list structure
 */
void wildcard_fill_list(char **argv, int argstart, int argcount, wildcard_strlist *strl)
{
    int i;
    
    wildcard_init_list(strl);
    
    for (i = argstart; i < argcount; i++) {
        wildcard_scan_pattern(argv[i], strl);
    }
}

/*
 * Scan a pattern and expand wildcards using AmigaOS native APIs
 *
 * This function uses AmigaOS MatchFirst() and MatchNext() functions
 * for efficient wildcard expansion. If the pattern doesn't contain
 * wildcards, it's added to the list as-is.
 *
 * Parameters:
 *   source - Pattern string to scan (may contain wildcards)
 *   strl   - Pointer to string list structure to fill
 *
 * Returns:
 *   0 on success, non-zero on failure
 */
int wildcard_scan_pattern(char *source, wildcard_strlist *strl)
{
    int ret;
    struct AnchorPath *ap;
    
    /* Allocate anchor path structure */
    ap = (struct AnchorPath *)AllocMem(sizeof(struct AnchorPath), MEMF_PUBLIC | MEMF_CLEAR);
    
    if (!ap) {
        return -1; /* Memory allocation failed */
    }
    
    /* Try to match the pattern */
    ret = MatchFirst(source, ap);
    
    if (!ret) {
        /* Pattern matched, add all matching files */
        do {
            wildcard_push_element(ap->ap_Info.fib_FileName, strl);
        } while (!MatchNext(ap));
        
        MatchEnd(ap);
    } else {
        /* No match or pattern doesn't contain wildcards, add as-is */
        wildcard_push_element(source, strl);
    }
    
    /* Clean up */
    FreeMem(ap, sizeof(struct AnchorPath));
    
    return ret;
}
