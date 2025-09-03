*
* SPDX-License-Identifier: BSD-2-Clause
* 
* Copyright (c) 2025 amigazen project
*
* Simple thread entry point for pthread implementation
*

        XREF    _ThreadEntry
        XDEF    _thread_entry

        SECTION "CODE",CODE

*
* Thread entry point - called by ASyncRun
* This is much simpler than the previous version
*
_thread_entry:
        * Set up A4 register (critical for SAS/C)
        move.l  d0,a4
        
        * Call the C thread entry function
        jsr     _ThreadEntry
        
        * Should not be reached, but is good practice
        rts

        END