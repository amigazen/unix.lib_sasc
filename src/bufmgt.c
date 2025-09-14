/*
 * SPDX-License-Identifier: BSD-2-Clause
 * bufmgt.c - Buffer management for stdio streams
 *
 * Based on PDC bufmgt.c by J.A. Lydiatt
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _WITH_SCLIB

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "amiga.h"

/* Global stream list */
static FILE *_stream_list = NULL;

/**
 * @brief Allocate a new FILE stream
 * @return FILE* stream or NULL on error
 * 
 * Allocates and initializes a new FILE stream structure.
 * This is used by fopen(), fdopen(), and other stream creation functions.
 */
FILE *addStream(void)
{
    FILE *fp;
    
    /* Allocate new FILE structure */
    fp = (FILE *)malloc(sizeof(FILE));
    if (fp == NULL) {
        return NULL;
    }
    
    /* Initialize all fields to safe defaults */
    fp->_file = -1;           /* Invalid file descriptor */
    fp->_flag = 0;            /* No flags set */
    fp->_base = NULL;         /* No buffer allocated */
    fp->_ptr = NULL;          /* No current position */
    fp->_rcnt = 0;            /* No read count */
    fp->_wcnt = 0;            /* No write count */
    fp->_size = 0;            /* No buffer size */
    fp->_cbuff = 0;           /* No character buffer */
    fp->_next = NULL;         /* No next stream */
    
    /* Add to stream list for cleanup */
    fp->_next = _stream_list;
    _stream_list = fp;
    
    return fp;
}

/**
 * @brief Allocate buffer for a stream
 * @param fp FILE stream to allocate buffer for
 * @return 0 on success, -1 on error
 * 
 * Allocates a buffer for the given stream if needed.
 */
int addStreamBuf(FILE *fp)
{
    char *buffer;
    
    if (fp == NULL) {
        return -1;
    }
    
    /* Only allocate if no buffer exists */
    if (fp->_base == NULL) {
        buffer = (char *)malloc(BUFSIZ);
        if (buffer == NULL) {
            /* Use single character buffer as fallback */
            fp->_base = &fp->_cbuff;
            fp->_size = 1;
        } else {
            fp->_base = buffer;
            fp->_size = BUFSIZ;
            fp->_flag |= _IOMYBUF;  /* We own this buffer */
        }
    }
    
    /* Initialize buffer pointers */
    fp->_ptr = fp->_base;
    fp->_rcnt = 0;
    fp->_wcnt = 0;
    
    return 0;
}

/**
 * @brief Close all streams
 * 
 * Closes all open streams. This is called at program exit.
 */
void _fcloseall_internal(void)
{
    FILE *fp, *next;
    
    fp = _stream_list;
    while (fp != NULL) {
        next = fp->_next;
        if (fp->_file >= 0) {
            fclose(fp);
        }
        fp = next;
    }
    _stream_list = NULL;
}

#else
/* Empty - functions provided by SAS/C sc.lib */
#endif
