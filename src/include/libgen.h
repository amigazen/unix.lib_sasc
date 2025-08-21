#ifndef _LIBGEN_H_
#define _LIBGEN_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NOTE ON POSIX COMPLIANCE AND SAFETY:
 *
 * The functions declared in this header, basename() and dirname(), are
 * permitted by the POSIX standard to modify the contents of the string
 * passed to them.
 *
 * They may also return a pointer to internal static memory that will be
 * overwritten by a subsequent call to either function.
 *
 * To use these functions safely, you should pass a duplicate of your
 * path string (e.g., one created with strdup()) if you need to
 * preserve the original string.
 */

char *basename(char *path);
char *dirname(char *path);

#ifdef __cplusplus
}
#endif

#endif /* !_LIBGEN_H_ */