/*
 *
 *  alloca(), setjmp(), longjmp() implementation for SAS/C
 *
 *  Copyright (c) 2025 amigazen project
 *  Based on work by Stefan Proels
 *  
 *  SPDX-License-Identifier: BSD-2-Clause
 *
 */

#ifndef _ALLOCA_H
#define _ALLOCA_H 1

#ifndef __SASC
#error Wrong compiler (SAS/C required)
#endif

/* do not include setjmp.h later */
#ifndef _SETJMP_H
#define _SETJMP_H 1
#endif

#include <stddef.h>


/*
 * jmp_buf structure for setjmp/longjmp
 * 
 * Note: This implementation stores the current __alloca_virtual_SP value
 * in jmp_buf[39] to properly restore the virtual stack state when
 * longjmp is called.
 */
typedef unsigned long jmp_buf[40];


/* Virtual stack pointer - tracks current call depth for alloca cleanup */
extern unsigned long __alloca_virtual_SP;

/* SAS/C function entry/exit hooks for automatic alloca cleanup */
extern void __asm _EPILOG(register __a0 char *);

/* Core setjmp/longjmp functions */
extern int __setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

/* Stack-based memory allocation function */
extern void *_alloca(size_t size);

/*
 * POSIX-compliant alloca macro
 * 
 * POSIX requires alloca to be a macro rather than a function to allow
 * compiler optimization and potential inlining. This macro calls our
 * internal _alloca implementation.
 */
#define alloca(size) _alloca(size)


/*
 * setjmp macro - stores current virtual stack pointer state
 * 
 * This macro saves the current __alloca_virtual_SP value in jmp_buf[39]
 * before calling the actual __setjmp function, ensuring the virtual
 * stack state is preserved for restoration.
 */
#define setjmp(jb)      __setjmp(((jb)[39] = __alloca_virtual_SP, (jb)))

/*
 * longjmp macro - restores virtual stack pointer and cleans up alloca blocks
 * 
 * This macro:
 * 1. Restores the __alloca_virtual_SP to the saved value + 1
 * 2. Calls _EPILOG to clean up any alloca blocks that are now out of scope
 * 3. Calls the actual longjmp function to perform the jump
 * 
 * This ensures proper cleanup of alloca memory when using setjmp/longjmp.
 */
#define longjmp(jb,rc)  do { __alloca_virtual_SP = (jb)[39] + 1; _EPILOG(NULL); longjmp((jb), (rc)); } while (0)


#endif /* _ALLOCA_H */
