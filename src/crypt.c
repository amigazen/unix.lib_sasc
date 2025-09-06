/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * crypt.c - POSIX password encryption functions using ACrypt()
 * 
 * This file implements POSIX crypt functions using Amiga's ACrypt() function
 * from amiga.lib for password encryption.
 * 
 * Functions implemented:
 * - crypt() - Encrypt a password
 * - crypt_r() - Reentrant version of crypt()
 * 
 * C89 compliant for AmigaOS compatibility
 */

#include "amiga.h"
#include <clib/alib_protos.h>
#include <string.h>
#include <errno.h>

/* Forward declaration for crypt_r data structure */
struct crypt_data {
    char initialized;
    char result[64];
    char __buf[256];
};

/*
 * crypt - Encrypt a password using ACrypt()
 * 
 * This function encrypts a password using Amiga's ACrypt() function.
 * Note: ACrypt() is not cryptographically secure and should only be used
 * for compatibility with existing systems.
 * 
 * @param key The password to encrypt
 * @param salt The salt string (ignored, ACrypt uses username)
 * @return Pointer to encrypted password, or NULL on error
 */
char *crypt(const char *key, const char *salt)
{
    static char result[16];
    UBYTE *encrypted;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate inputs */
    if (key == NULL || salt == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* ACrypt() requires username, we'll use "user" as default */
    /* Note: This is a limitation of using ACrypt() for POSIX crypt() */
    encrypted = ACrypt((UBYTE *)result, (UBYTE *)key, (UBYTE *)"user");
    
    if (encrypted == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    return result;
}

/*
 * crypt_r - Reentrant version of crypt()
 * 
 * This function provides a reentrant version of crypt() using the provided
 * data structure to avoid static storage.
 * 
 * @param key The password to encrypt
 * @param salt The salt string (ignored, ACrypt uses username)
 * @param data Pointer to crypt_data structure
 * @return Pointer to encrypted password, or NULL on error
 */
char *crypt_r(const char *key, const char *salt, struct crypt_data *data)
{
    UBYTE *encrypted;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate inputs */
    if (key == NULL || salt == NULL || data == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    /* Initialize data structure if needed */
    if (!data->initialized) {
        memset(data, 0, sizeof(struct crypt_data));
        data->initialized = 1;
    }
    
    /* ACrypt() requires username, we'll use "user" as default */
    /* Note: This is a limitation of using ACrypt() for POSIX crypt() */
    encrypted = ACrypt((UBYTE *)data->result, (UBYTE *)key, (UBYTE *)"user");
    
    if (encrypted == NULL) {
        errno = EINVAL;
        return NULL;
    }
    
    return data->result;
}

/*
 * setkey - Set encryption key (obsolete function)
 * 
 * This function is provided for compatibility but does nothing useful
 * with ACrypt() as it doesn't support key setting.
 * 
 * @param key The encryption key (ignored)
 */
void setkey(const char *key)
{
    /* Check for abort signal */
    __chkabort();
    
    /* ACrypt() doesn't support key setting, so this is a no-op */
    /* Provided for compatibility only */
    (void)key; /* Suppress unused parameter warning */
}

/*
 * encrypt - Encrypt/decrypt a block (obsolete function)
 * 
 * This function is provided for compatibility but does nothing useful
 * with ACrypt() as it doesn't support block encryption.
 * 
 * @param block The data block (ignored)
 * @param flag The operation flag (ignored)
 */
void encrypt(char block[64], int flag)
{
    /* Check for abort signal */
    __chkabort();
    
    /* ACrypt() doesn't support block encryption, so this is a no-op */
    /* Provided for compatibility only */
    (void)block; /* Suppress unused parameter warning */
    (void)flag;  /* Suppress unused parameter warning */
}

