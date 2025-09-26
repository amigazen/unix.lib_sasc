/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * search.h - Search functions
 * 
 * This header provides search and sort functions for AmigaOS.
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _SEARCH_H
#define _SEARCH_H 1

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Action codes for hsearch() */
typedef enum {
    FIND,
    ENTER
} ACTION;

/* Visit codes for twalk() */
typedef enum {
    preorder,
    postorder,
    endorder,
    leaf
} VISIT;

/* Entry structure for hsearch() */
typedef struct entry {
    char *key;
    void *data;
} ENTRY;

/* Function prototypes */
#ifndef _WITH_SCLIB
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
#endif
void *lfind(const void *key, const void *base, size_t *nmemb, size_t size,
            int (*compar)(const void *, const void *));
void *lsearch(const void *key, void *base, size_t *nmemb, size_t size,
              int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
int hcreate(size_t nel);
void hdestroy(void);
ENTRY *hsearch(ENTRY item, ACTION action);
int insque(void *element, void *pred);
int remque(void *element);
void *tdelete(const void *key, void **rootp,
              int (*compar)(const void *, const void *));
void *tfind(const void *key, void *const *rootp,
            int (*compar)(const void *, const void *));
void *tsearch(const void *key, void **rootp,
              int (*compar)(const void *, const void *));
void twalk(const void *root, void (*action)(const void *, VISIT, int));

#ifdef __cplusplus
}
#endif

#endif /* _SEARCH_H */
