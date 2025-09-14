/*
 * SPDX-License-Identifier: BSD-2-Clause
 * malloc.c - Memory management functions for standalone builds
 *
 * Based on PDC malloc.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <stdlib.h>

/* According to X3J11, the response to a request for zero bytes is
   implementation defined.  For PDC, the intention is to return a
   successful result, but one that will cause a trap if it is ever
   dereferenced.  Under most present-day Amiga's, 0xFFFFFFFF (-1L)
   fills the bill just fine.  This may have problems, though, on
   systems with MMU's and very high memory addresses.
 */

#define ZERO_REQUEST -1L

typedef struct memchunk {
    struct memchunk    *next;
    struct memchunk    *prev;
    long                size;
} MEMCHUNK;

extern void (*_freeall)(void);

static MEMCHUNK sentinel = {&sentinel, &sentinel, 0};

/*
 * Called by exit() to free any allocated memory.
 */
static void freeall(void)
{
    MEMCHUNK    *mp, *mp1;

    for (mp = sentinel.prev; mp != &sentinel; ) {
        mp1 = mp->prev;
        FreeMem((APTR)mp, (ULONG)mp->size);
        mp = mp1;
    }
}

/**
 * @brief Allocate memory block
 * @param size Size of memory block to allocate
 * @return Pointer to allocated memory or NULL on failure
 * 
 * The malloc() function allocates a block of memory of the specified size.
 * The allocated memory is not initialized.
 * 
 * This implementation:
 * - Uses Amiga AllocMem() for memory allocation
 * - Maintains a linked list of allocated chunks
 * - Returns NULL on failure
 * - Handles zero-size requests (returns special value)
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void *malloc(size_t size)
{
    MEMCHUNK *mp;
    ULONG chunksize;

    if (size == 0) {
        return (void *)ZERO_REQUEST;
    }

    chunksize = size + sizeof(MEMCHUNK);
    mp = AllocMem(chunksize, (ULONG)MEMF_CLEAR);
    if (mp == NULL) {
        return NULL;
    }

    /* Keep the forward and backward links */
    sentinel.prev->next = mp;
    mp->prev = sentinel.prev;
    sentinel.prev = mp;

    mp->next = &sentinel;
    mp->size = chunksize;
    _freeall = &freeall;
    return (void *)(++mp);
}

/**
 * @brief Free allocated memory block
 * @param p Pointer to memory block to free
 * 
 * The free() function deallocates the memory block pointed to by p.
 * If p is NULL, no operation is performed.
 * 
 * This implementation:
 * - Uses Amiga FreeMem() for memory deallocation
 * - Maintains linked list integrity
 * - Performs sanity checks on the memory block
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void free(void *p)
{
    MEMCHUNK *mp, *prevmp, *nextmp;

    if (p == NULL) {
        return;
    }

    mp = (MEMCHUNK *)((char *)p - sizeof(MEMCHUNK));

    /* Sanity check: the prev link should point to us. Do nothing if bad. */
    prevmp = mp->prev;
    nextmp = mp->next;
    if (prevmp->next != mp) {
        return;
    }

    FreeMem((APTR)mp, (ULONG)mp->size);
    prevmp->next = nextmp;
    nextmp->prev = prevmp;
}

/**
 * @brief Allocate and clear memory block
 * @param nelem Number of elements
 * @param elsize Size of each element
 * @return Pointer to allocated memory or NULL on failure
 * 
 * The calloc() function allocates memory for an array of nelem elements,
 * each of which is elsize bytes long. The memory is initialized to zero.
 * 
 * This implementation:
 * - Uses malloc() for allocation
 * - Uses bzero() to clear the memory
 * - Handles overflow in size calculation
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void *calloc(size_t nelem, size_t elsize)
{
    char *newmem;
    size_t totsize;

    totsize = nelem * elsize;
    if (nelem != 0 && totsize / nelem != elsize) {
        /* Overflow in size calculation */
        return NULL;
    }

    newmem = malloc(totsize);
    if (newmem != NULL) {
        bzero(newmem, totsize);
    }

    return newmem;
}

/**
 * @brief Reallocate memory block
 * @param ptr Pointer to previously allocated memory
 * @param size New size for the memory block
 * @return Pointer to reallocated memory or NULL on failure
 * 
 * The realloc() function changes the size of the memory block pointed to
 * by ptr to size bytes. The contents will be unchanged in the range from
 * the start of the region up to the minimum of the old and new sizes.
 * 
 * This implementation:
 * - Uses malloc() for new allocation
 * - Uses bcopy() to copy existing data
 * - Frees old memory block
 * - Handles NULL pointer (treats as malloc)
 * - Is C89 compliant for SAS/C compiler compatibility
 */
void *realloc(void *ptr, size_t size)
{
    MEMCHUNK *mp, *prevmp;
    char *newmem;
    size_t oldsize;

    if (ptr == NULL) {
        return malloc(size);
    }

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    mp = (MEMCHUNK *)((char *)ptr - sizeof(MEMCHUNK));

    /* Sanity check: the prev link should point to us. Do nothing if bad. */
    prevmp = mp->prev;
    if (prevmp->next != mp) {
        return NULL;
    }

    newmem = malloc(size);
    if (newmem == NULL) {
        return NULL;
    }

    oldsize = mp->size - sizeof(MEMCHUNK);
    if (size < oldsize) {
        oldsize = size;
    }

    bcopy(ptr, newmem, oldsize);
    free(ptr);

    return newmem;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
