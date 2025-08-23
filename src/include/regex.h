#ifndef _REGEX_H_
#define _REGEX_H_

#include <sys/types.h> /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* --- Types --- */
typedef long int regoff_t;

typedef struct {
    void *__internal; /* Opaque pointer for implementation details */
    size_t re_nsub;   /* Number of parenthesized subexpressions. */
} regex_t;

typedef struct {
    regoff_t rm_so;   /* Byte offset from start of string to start of substring. */
    regoff_t rm_eo;   /* Byte offset from start of string to end of substring. */
} regmatch_t;

/* --- Flags for regcomp() --- */
#define REG_EXTENDED    0x01 /* Use Extended Regular Expressions. */
#define REG_ICASE       0x02 /* Ignore case in matches. */
#define REG_NOSUB       0x04 /* Report only success/fail in regexec(). */
#define REG_NEWLINE     0x08 /* Treat newline as a special character. */

/* --- Flags for regexec() --- */
#define REG_NOTBOL      0x01 /* The beginning of the string is not the beginning of a line. */
#define REG_NOTEOL      0x02 /* The end of the string is not the end of a line. */

/* --- Error Codes --- */
#define REG_NOMATCH     1  /* No match found. */
#define REG_BADPAT      2  /* Invalid regular expression. */
#define REG_ECOLLATE    3  /* Invalid collating element. */
#define REG_ECTYPE      4  /* Invalid character class. */
#define REG_EESCAPE     5  /* Trailing backslash. */
#define REG_ESUBREG     6  /* Invalid backreference. */
#define REG_EBRACK      7  /* Unmatched bracket. */
#define REG_EPAREN      8  /* Unmatched parenthesis. */
#define REG_EBRACE      9  /* Unmatched brace. */
#define REG_BADBR       10 /* Invalid content of a brace expression. */
#define REG_ERANGE      11 /* Invalid range end. */
#define REG_ESPACE      12 /* Out of memory. */
#define REG_BADRPT      13 /* Invalid repetition operator. */

/* --- C89-compliant function prototypes --- */
int    regcomp(regex_t *preg, const char *pattern, int cflags);
int    regexec(const regex_t *preg, const char *string, size_t nmatch,
               regmatch_t pmatch[], int eflags);
size_t regerror(int errcode, const regex_t *preg, char *errbuf,
                size_t errbuf_size);
void   regfree(regex_t *preg);

#ifdef __cplusplus
}
#endif

#endif /* !_REGEX_H_ */
