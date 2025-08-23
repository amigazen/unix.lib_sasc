/*
 * dirname.c - extract directory from path using AmigaOS native functions
 * 
 * This implementation uses the AmigaOS dos.library PathPart() function
 * which properly handles Amiga path conventions like "Volume:Folder/File"
 * 
 * Enhanced with robust error handling and static buffer safety
 * Note: This version returns a static buffer instead of modifying input
 */

#include <proto/dos.h>
#include <string.h>
#include "include/libgen.h"

/**
 * @brief Extract the directory from a path using AmigaOS native functions
 * @param path The path to extract directory from
 * @return Pointer to the directory part (statically allocated)
 * 
 * Enhanced version: This function returns a pointer to a static buffer
 * containing the directory part, providing better memory safety.
 * 
 * This function uses AmigaOS dos.library PathPart() which properly handles:
 * - Unix-style paths: "/usr/local/bin/file" -> "/usr/local/bin"
 * - Amiga-style paths: "Volume:Folder/Subfolder/File" -> "Volume:Folder/Subfolder"
 * - Mixed paths: "Volume:Folder/Subfolder/File" -> "Volume:Folder/Subfolder"
 * - Edge cases like empty paths, single components, etc.
 */
char *dirname(char *path)
{
    static char result[256];
    char *dir_part;
    size_t dir_len;
    
    if (path == NULL || *path == '\0') {
        strcpy(result, ".");
        return result;
    }
    
    /* Use AmigaOS native PathPart() function for proper path handling */
    dir_part = PathPart(path);
    
    /* If PathPart returns the beginning of the string, it means there's no directory */
    if (dir_part == path) {
        strcpy(result, ".");
        return result;
    }
    
    /* Calculate the length from start to dir_part */
    dir_len = dir_part - path;
    
    /* Check if the result fits in our buffer */
    if (dir_len >= sizeof(result)) {
        strcpy(result, ".");
        return result;
    }
    
    /* Copy the directory part to our static buffer */
    strncpy(result, path, dir_len);
    result[dir_len] = '\0';
    
    /* Handle edge case where we end up with just "Volume:" */
    if (dir_len > 0 && result[dir_len - 1] == ':') {
        /* Keep the colon for volume names */
        result[dir_len] = ':';
        result[dir_len + 1] = '\0';
    }
    
    /* Handle root directory case */
    if (strlen(result) == 0) {
        strcpy(result, "/");
    }
    
    return result;
}
