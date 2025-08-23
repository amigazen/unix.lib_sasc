/*
 * basename.c - extract filename from path using AmigaOS native functions
 * 
 * This implementation uses the AmigaOS dos.library FilePart() function
 * which properly handles Amiga path conventions like "Volume:Folder/File"
 * 
 * POSIX compliant: returns pointer to filename part
 * Enhanced with robust error handling and static buffer safety
 */

#include <proto/dos.h>
#include <string.h>
#include "include/libgen.h"

/**
 * @brief Extract the basename from a path using AmigaOS native functions
 * @param path The path to extract basename from
 * @return Pointer to the filename part (statically allocated)
 * 
 * POSIX compliant: This function returns a pointer to the filename part
 * of the path. The returned pointer points to a static buffer.
 * 
 * This function uses AmigaOS dos.library FilePart() which properly handles:
 * - Unix-style paths: "/usr/local/bin/file" -> "file"
 * - Amiga-style paths: "Volume:Folder/Subfolder/File" -> "File"
 * - Mixed paths: "Volume:Folder/Subfolder/File" -> "File"
 * - Edge cases like empty paths, single components, etc.
 */
char *basename(char *path)
{
    static char result[256];
    char *filename;
    
    if (path == NULL || *path == '\0') {
        strcpy(result, ".");
        return result;
    }
    
    /* Use AmigaOS native FilePart() function for proper path handling */
    filename = FilePart(path);
    
    /* Copy to our static buffer for safety and consistency */
    if (filename == NULL || strlen(filename) >= sizeof(result)) {
        strcpy(result, ".");
    } else {
        strcpy(result, filename);
    }
    
    return result;
}
