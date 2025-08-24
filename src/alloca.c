
/*
 *
 *  alloca() implementation for SAS/C
 *
 *  Copyright (c) 2025 amigazen project
 *  Based on work by Stefan Proels
 *  
 *  SPDX-License-Identifier: BSD-2-Clause
 *
 */

#include <stddef.h>
#include <stdlib.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/alib_protos.h>
#include <constructor.h>

#ifndef __SASC
#error Wrong compiler (SAS/C required)
#endif

#ifdef _PROFILE
#error This file must NOT be compiled with PROFILE enabled
#endif


/*
 * each alloca()ted block is preceded by this header
 * 
 * This implementation cleverly simulates stack allocation without using the actual
 * hardware stack. Instead, it creates its own "virtual stack" using a managed
 * pool of memory from AmigaOS's exec.library.
 */
struct block
  {
    struct block *down;    /* linked list pointer to previous block */
    ULONG size;            /* total size including this header */
    ULONG level;           /* call depth when this block was allocated */
    /* alloca()ted memory below */
  };

static APTR pool = NULL;           /* AmigaOS memory pool for efficient allocation */
static struct block *top = NULL;   /* top of the linked list of allocated blocks */
ULONG __alloca_virtual_SP = 0;    /* virtual stack pointer - tracks call depth */


/*
 * SAS/C autoinitialization function
 * 
 * Creates the memory pool using AmigaOS's LibCreatePool for efficient
 * management of many small allocations. This reduces system call overhead
 * and memory fragmentation compared to calling AllocMem for each request.
 */
PROFILE_CONSTRUCTOR(Sprof)
{
  if (pool = LibCreatePool(MEMF_ANY, 4096, 2048))
    return 0;
  return -1;
}

/*
 * SAS/C autotermination function
 */
PROFILE_DESTRUCTOR(Sprof)
{
  if (pool)
    LibDeletePool(pool);
}


/*
 * called whenever a function is entered
 * 
 * This is the magic of SAS/C - the compiler automatically calls _PROLOG
 * upon entering any function. We simply increment our call depth counter.
 */
void __asm _PROLOG(register __a0 char *dummy)
{
  __alloca_virtual_SP++;
}

/*
 * called on return from a function
 * 
 * This is the cleanup crew. SAS/C automatically calls _EPILOG just before
 * returning from any function. We check all active allocations and free
 * any that were made at the current call depth (or deeper, which shouldn't
 * happen in normal execution).
 * 
 * WARNING: This implementation is NOT safe with setjmp/longjmp as longjmp
 * unwinds the C stack without calling _EPILOG functions, causing memory leaks.
 */
void __asm _EPILOG(register __a0 char *dummy)
{
  struct block *mem;

  for (mem = top; mem && mem->level >= __alloca_virtual_SP; mem = top)
    {
      top = mem->down;
      LibFreePooled(pool, mem, mem->size);
    }
  --__alloca_virtual_SP;
}


/*
 * that's what the whole thing is all about -- alloca()te memory
 * 
 * When alloca() is called:
 * 1. We request memory from our dedicated pool (much faster than AllocMem)
 * 2. We prepend a hidden struct block header to store metadata
 * 3. The header stores the block's size and current call depth
 * 4. The new block is added to the top of our singly-linked list
 * 5. We return a pointer to the memory AFTER the hidden header
 * 
 * Note: On allocation failure, we call abort() - this is a "fail-fast"
 * approach as running out of stack space is often considered fatal.
 */
void *_alloca(size_t size)
{
  struct block *mem;

  if (size <= 0)  /* garbage collection -- not required */
    return NULL;

  size += sizeof(struct block);
  if (!(mem = (struct block *)LibAllocPooled(pool, (ULONG)size)))
    {
      /* do whatever you want here */
      abort();
    }
  mem->down = top;
  mem->size = size;
  mem->level = __alloca_virtual_SP;
  top = mem;
  return ++mem;
}

