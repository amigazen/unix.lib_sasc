#ifndef _GLOB_H_
#define _GLOB_H_

#include <sys/types.h> /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The glob_t structure is used to pass arguments to glob()
 * and to store the results of a successful call.
 */
typedef struct {
    size_t   gl_pathc;    /* Count of matched pathnames. */
    char   **gl_pathv;    /* Pointer to a list of matched pathname strings. */
    size_t   gl_offs;     /* Slots to reserve at the beginning of gl_pathv. */
} glob_t;


/* --- Bit flags for the `flags` argument of glob() --- */

/* Append pathnames from this call to the results of a previous call. */
#define GLOB_APPEND     0x0001

/* Make use of the gl_offs field to reserve empty slots at the beginning of the gl_pathv list. */
#define GLOB_DOOFFS     0x0002

/* Cause glob() to return on a directory read error, instead of just continuing. */
#define GLOB_ERR        0x0004

/* Append a slash '/' to each pathname that is a directory. */
#define GLOB_MARK       0x0008

/* If the pattern matches no files, return the original pattern as the only result. */
#define GLOB_NOCHECK    0x0010

/* By default, results are sorted. This flag disables sorting for a speed increase. */
#define GLOB_NOSORT     0x0020


/* --- Error return values from glob() --- */

#define GLOB_NOSPACE    1   /* An attempt to allocate memory failed. */
#define GLOB_ABORTED    2   /* The scan was stopped by the error function. */
#define GLOB_NOMATCH    3   /* The pattern did not match any existing files. */


/* --- C89-compliant function prototypes --- */

int glob(const char *pattern, int flags,
         int (*errfunc)(const char *epath, int eerrno),
         glob_t *pglob);

void globfree(glob_t *pglob);


#ifdef __cplusplus
}
#endif

#endif /* !_GLOB_H_ */