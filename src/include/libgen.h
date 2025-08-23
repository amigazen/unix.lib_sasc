/*
 * libgen.h - POSIX pathname manipulation functions
 * 
 * This header provides declarations for basename() and dirname() functions
 * that use AmigaOS native dos.library functions for proper path handling.
 * 
 * Enhanced with robust error handling and static buffer safety
 */

#ifndef _LIBGEN_H_
#define _LIBGEN_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extract the filename from a path
 * @param path The path to extract filename from
 * @return Pointer to the filename part (statically allocated)
 * 
 * This function uses AmigaOS dos.library FilePart() for proper
 * Amiga path handling including "Volume:Folder/File" conventions.
 * 
 * Returns a pointer to a static buffer containing the filename.
 * The buffer is shared between calls, so the result should be used
 * immediately or copied if needed later.
 */
char *basename(char *path);

/**
 * @brief Extract the directory from a path
 * @param path The path to extract directory from
 * @return Pointer to the directory part (statically allocated)
 * 
 * This function uses AmigaOS dos.library PathPart() for proper
 * Amiga path handling including "Volume:Folder/Subfolder/File" conventions.
 * 
 * Returns a pointer to a static buffer containing the directory path.
 * The buffer is shared between calls, so the result should be used
 * immediately or copied if needed later.
 */
char *dirname(char *path);

#ifdef __cplusplus
}
#endif

#endif /* _LIBGEN_H_ */
