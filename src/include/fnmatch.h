#ifndef _FNMATCH_H_
#define _FNMATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* --- Flags for the fnmatch() function --- */

/* If set, a slash '/' in the string must be explicitly matched by a slash in the pattern. */
#define FNM_PATHNAME    0x01

/* If set, disables backslash escaping. A backslash is treated as a normal character. */
#define FNM_NOESCAPE    0x02

/* If set, a leading period '.' in the string must be explicitly matched by a period in the pattern. */
#define FNM_PERIOD      0x04


/* --- Return Values --- */

/* The return value from fnmatch() if the string does not match the pattern. */
#define FNM_NOMATCH     1

int fnmatch(const char *pattern, const char *string, int flags);


#ifdef __cplusplus
}
#endif

#endif /* !_FNMATCH_H_ */