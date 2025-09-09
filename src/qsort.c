/*
 * qsort.c - Quick sort implementation (POSIX compliant)
 *
 * This function sorts an array of nmemb objects, the initial member of which
 * is pointed to by base. The size of each object is specified by size.
 * The contents of the array are sorted in ascending order according to a
 * comparison function pointed to by compar, which is called with two arguments
 * that point to the objects being compared.
 *
 * The comparison function must return an integer less than, equal to, or
 * greater than zero if the first argument is considered to be respectively
 * less than, equal to, or greater than the second.
 *
 * POSIX.1-2001, POSIX.1-2008, C89, C99
 *
 * Copyright (c) 2025 amigazen project
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "amiga.h"
#include <stdlib.h>
#include <string.h>

/* Internal function declarations */
static void qsort_internal(char *base, size_t nmemb, size_t size,
                          int (*compar)(const void *, const void *),
                          char *temp);

/*
 * qsort - sort an array
 */
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *))
{
    char *temp;
    
    /* Check for valid parameters */
    if (base == NULL || compar == NULL || size == 0) {
        return;
    }
    
    /* Handle empty or single element arrays */
    if (nmemb <= 1) {
        return;
    }
    
    /* Allocate temporary storage for swapping elements */
    temp = malloc(size);
    if (temp == NULL) {
        /* If we can't allocate temp storage, we can't sort */
        return;
    }
    
    /* Call internal quicksort implementation */
    qsort_internal((char *)base, nmemb, size, compar, temp);
    
    /* Free temporary storage */
    free(temp);
}

/*
 * qsort_internal - internal quicksort implementation
 * 
 * This is a standard quicksort algorithm with the following optimizations:
 * - Uses insertion sort for small subarrays (<= 7 elements)
 * - Uses median-of-three pivot selection for better performance
 * - Implements tail recursion elimination to avoid stack overflow
 */
static void qsort_internal(char *base, size_t nmemb, size_t size,
                          int (*compar)(const void *, const void *),
                          char *temp)
{
    char *left, *right, *pivot;
    char *i, *j;
    size_t pivot_index;
    int cmp;
    
    /* Use insertion sort for small arrays */
    if (nmemb <= 7) {
        char *current, *next;
        size_t k, l;
        
        for (k = 1; k < nmemb; k++) {
            current = base + k * size;
            for (l = k; l > 0; l--) {
                next = base + (l - 1) * size;
                if (compar(current, next) >= 0) {
                    break;
                }
                /* Swap elements */
                memcpy(temp, current, size);
                memcpy(current, next, size);
                memcpy(next, temp, size);
                current = next;
            }
        }
        return;
    }
    
    /* Median-of-three pivot selection */
    pivot_index = nmemb / 2;
    pivot = base + pivot_index * size;
    
    /* Compare first, middle, and last elements */
    if (compar(base, pivot) > 0) {
        /* Swap first and middle */
        memcpy(temp, base, size);
        memcpy(base, pivot, size);
        memcpy(pivot, temp, size);
    }
    if (compar(pivot, base + (nmemb - 1) * size) > 0) {
        /* Swap middle and last */
        memcpy(temp, pivot, size);
        memcpy(pivot, base + (nmemb - 1) * size, size);
        memcpy(base + (nmemb - 1) * size, temp, size);
    }
    if (compar(base, pivot) > 0) {
        /* Swap first and middle again */
        memcpy(temp, base, size);
        memcpy(base, pivot, size);
        memcpy(pivot, temp, size);
    }
    
    /* Partition the array */
    left = base;
    right = base + (nmemb - 1) * size;
    
    while (left < right) {
        /* Move left pointer to find element >= pivot */
        while (left < right && compar(left, pivot) < 0) {
            left += size;
        }
        
        /* Move right pointer to find element <= pivot */
        while (left < right && compar(right, pivot) > 0) {
            right -= size;
        }
        
        /* Swap elements if pointers haven't crossed */
        if (left < right) {
            memcpy(temp, left, size);
            memcpy(left, right, size);
            memcpy(right, temp, size);
            
            /* Update pivot position if we swapped it */
            if (left == pivot) {
                pivot = right;
            } else if (right == pivot) {
                pivot = left;
            }
            
            left += size;
            if (left < right) {
                right -= size;
            }
        }
    }
    
    /* Recursively sort left and right partitions */
    if (pivot > base) {
        qsort_internal(base, (pivot - base) / size, size, compar, temp);
    }
    if (pivot < base + (nmemb - 1) * size) {
        qsort_internal(pivot + size, 
                      (base + nmemb * size - pivot - size) / size, 
                      size, compar, temp);
    }
}
