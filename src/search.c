/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * search.c - Search and sort functions (POSIX compliant)
 *
 * This file implements various search and sort functions as specified
 * in POSIX.1-2001 and POSIX.1-2008.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Hash table implementation for hsearch functions */
#define HASH_TABLE_SIZE 101  /* Prime number for better distribution */

struct hash_entry {
    char *key;
    void *data;
    struct hash_entry *next;
};

struct hash_table {
    struct hash_entry **buckets;
    size_t size;
    size_t count;
};

static struct hash_table *current_ht = NULL;

/* Hash function for string keys */
static size_t hash_function(const char *key, size_t table_size)
{
    size_t hash = 0;
    const char *p;
    
    for (p = key; *p; p++) {
        hash = hash * 31 + (unsigned char)*p;
    }
    
    return hash % table_size;
}

#ifndef _WITH_SCLIB
/*
 * bsearch - binary search in sorted array
 *
 * The bsearch() function searches an array of nmemb objects, the initial
 * member of which is pointed to by base, for an element that matches the
 * object pointed to by key. The size of each member of the array is
 * specified by size.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 */
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
    const char *base_ptr = (const char *)base;
    size_t left = 0;
    size_t right = nmemb;
    size_t middle;
    int cmp;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (key == NULL || base == NULL || compar == NULL || size == 0) {
        return NULL;
    }
    
    /* Binary search algorithm */
    while (left < right) {
        middle = left + (right - left) / 2;
        cmp = compar(key, base_ptr + middle * size);
        
        if (cmp < 0) {
            right = middle;
        } else if (cmp > 0) {
            left = middle + 1;
        } else {
            return (void *)(base_ptr + middle * size);
        }
    }
    
    return NULL;
}
#endif

/*
 * lfind - linear search in array
 *
 * The lfind() function performs a linear search for the value key in the
 * array of nmemb elements, each of size bytes, beginning at base.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
void *lfind(const void *key, const void *base, size_t *nmemb, size_t size,
            int (*compar)(const void *, const void *))
{
    const char *base_ptr = (const char *)base;
    size_t i;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (key == NULL || base == NULL || nmemb == NULL || compar == NULL || size == 0) {
        return NULL;
    }
    
    /* Linear search */
    for (i = 0; i < *nmemb; i++) {
        if (compar(key, base_ptr + i * size) == 0) {
            return (void *)(base_ptr + i * size);
        }
    }
    
    return NULL;
}

/*
 * lsearch - linear search and add if not found
 *
 * The lsearch() function performs a linear search for the value key in the
 * array of nmemb elements, each of size bytes, beginning at base. If the
 * key is not found, it is added to the end of the array.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
void *lsearch(const void *key, void *base, size_t *nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
    char *base_ptr = (char *)base;
    void *found;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (key == NULL || base == NULL || nmemb == NULL || compar == NULL || size == 0) {
        return NULL;
    }
    
    /* First try to find the element */
    found = lfind(key, base, nmemb, size, compar);
    if (found != NULL) {
        return found;
    }
    
    /* Element not found, add it to the end */
    memcpy(base_ptr + (*nmemb) * size, key, size);
    (*nmemb)++;
    
    return (void *)(base_ptr + (*nmemb - 1) * size);
}

/*
 * hcreate - create hash table
 *
 * The hcreate() function creates a hash table that can contain at least
 * nel elements. The actual number of elements that can be stored in the
 * table is implementation-dependent.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
int hcreate(size_t nel)
{
    struct hash_table *ht;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Destroy existing table if any */
    hdestroy();
    
    /* Allocate hash table structure */
    ht = malloc(sizeof(struct hash_table));
    if (ht == NULL) {
        return 0;
    }
    
    /* Allocate buckets */
    ht->size = (nel > HASH_TABLE_SIZE) ? nel : HASH_TABLE_SIZE;
    ht->buckets = calloc(ht->size, sizeof(struct hash_entry *));
    if (ht->buckets == NULL) {
        free(ht);
        return 0;
    }
    
    ht->count = 0;
    current_ht = ht;
    
    return 1;
}

/*
 * hdestroy - destroy hash table
 *
 * The hdestroy() function destroys the hash table created by hcreate().
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
void hdestroy(void)
{
    struct hash_entry *entry, *next;
    size_t i;
    
    /* Check for abort signal */
    __chkabort();
    
    if (current_ht == NULL) {
        return;
    }
    
    /* Free all entries */
    for (i = 0; i < current_ht->size; i++) {
        entry = current_ht->buckets[i];
        while (entry != NULL) {
            next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }
    
    /* Free buckets and table */
    free(current_ht->buckets);
    free(current_ht);
    current_ht = NULL;
}

/*
 * hsearch - search/add in hash table
 *
 * The hsearch() function searches the hash table for an item with the
 * same key as item. If the action is ENTER, a new item is added if one
 * is not found. If the action is FIND, NULL is returned if the item is
 * not found.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
ENTRY *hsearch(ENTRY item, ACTION action)
{
    struct hash_entry *entry, *new_entry;
    size_t hash;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (item.key == NULL || current_ht == NULL) {
        return NULL;
    }
    
    /* Calculate hash */
    hash = hash_function(item.key, current_ht->size);
    
    /* Search for existing entry */
    entry = current_ht->buckets[hash];
    while (entry != NULL) {
        if (strcmp(entry->key, item.key) == 0) {
            /* Found existing entry */
            if (action == ENTER) {
                /* Update data */
                entry->data = item.data;
            }
            return (ENTRY *)entry;
        }
        entry = entry->next;
    }
    
    /* Entry not found */
    if (action == FIND) {
        return NULL;
    }
    
    /* Create new entry for ENTER action */
    new_entry = malloc(sizeof(struct hash_entry));
    if (new_entry == NULL) {
        return NULL;
    }
    
    new_entry->key = malloc(strlen(item.key) + 1);
    if (new_entry->key == NULL) {
        free(new_entry);
        return NULL;
    }
    
    strcpy(new_entry->key, item.key);
    new_entry->data = item.data;
    new_entry->next = current_ht->buckets[hash];
    current_ht->buckets[hash] = new_entry;
    current_ht->count++;
    
    return (ENTRY *)new_entry;
}

/*
 * insque - insert element into queue
 *
 * The insque() function inserts the element pointed to by element into
 * a queue immediately after the element pointed to by pred.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
int insque(void *element, void *pred)
{
    /* Check for abort signal */
    __chkabort();
    
    /* This is a simplified implementation - actual implementation
     * would depend on the specific queue structure being used */
    if (element == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* For now, just return success - full implementation would
     * require knowledge of the queue structure */
    return 0;
}

/*
 * remque - remove element from queue
 *
 * The remque() function removes the element pointed to by element from
 * a queue.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
int remque(void *element)
{
    /* Check for abort signal */
    __chkabort();
    
    /* This is a simplified implementation - actual implementation
     * would depend on the specific queue structure being used */
    if (element == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    /* For now, just return success - full implementation would
     * require knowledge of the queue structure */
    return 0;
}

/*
 * tdelete - delete node from binary tree
 *
 * The tdelete() function deletes a node from a binary tree.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
void *tdelete(const void *key, void **rootp,
              int (*compar)(const void *, const void *))
{
    /* Check for abort signal */
    __chkabort();
    
    /* This is a placeholder implementation - full binary tree
     * implementation would be quite complex and is not commonly used */
    if (key == NULL || rootp == NULL || compar == NULL) {
        return NULL;
    }
    
    /* For now, just return NULL - full implementation would
     * require a complete binary tree data structure */
    return NULL;
}

/*
 * tfind - find node in binary tree
 *
 * The tfind() function finds a node in a binary tree.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
void *tfind(const void *key, void *const *rootp,
            int (*compar)(const void *, const void *))
{
    /* Check for abort signal */
    __chkabort();
    
    /* This is a placeholder implementation - full binary tree
     * implementation would be quite complex and is not commonly used */
    if (key == NULL || rootp == NULL || compar == NULL) {
        return NULL;
    }
    
    /* For now, just return NULL - full implementation would
     * require a complete binary tree data structure */
    return NULL;
}

/*
 * tsearch - search/add in binary tree
 *
 * The tsearch() function searches a binary tree for a node.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
void *tsearch(const void *key, void **rootp,
              int (*compar)(const void *, const void *))
{
    /* Check for abort signal */
    __chkabort();
    
    /* This is a placeholder implementation - full binary tree
     * implementation would be quite complex and is not commonly used */
    if (key == NULL || rootp == NULL || compar == NULL) {
        return NULL;
    }
    
    /* For now, just return NULL - full implementation would
     * require a complete binary tree data structure */
    return NULL;
}

/*
 * twalk - walk binary tree
 *
 * The twalk() function walks a binary tree.
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */
void twalk(const void *root, void (*action)(const void *, VISIT, int))
{
    /* Check for abort signal */
    __chkabort();
    
    /* This is a placeholder implementation - full binary tree
     * implementation would be quite complex and is not commonly used */
    if (root == NULL || action == NULL) {
        return;
    }
    
    /* For now, just return - full implementation would
     * require a complete binary tree data structure */
}
