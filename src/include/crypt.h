/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 *
 * POSIX crypt header for Amiga
 */

#ifndef _CRYPT_H
#define _CRYPT_H

/* Reentrant crypt data structure */
struct crypt_data {
    char initialized;
    char result[64];
    char __buf[256];
};

/* Crypt functions */
char *crypt(const char *key, const char *salt);
char *crypt_r(const char *key, const char *salt, struct crypt_data *data);

/* Obsolete functions for compatibility */
void setkey(const char *key);
void encrypt(char block[64], int flag);

#endif /* _CRYPT_H */
