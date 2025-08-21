#ifndef _INTTYPES_H_
#define _INTTYPES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Printf format macro definitions --- */

/* Signed Integers */
#define PRId8       "d"
#define PRId16      "d"
#define PRId32      "ld"
#define PRId64      "lld"

#define PRIi8       "i"
#define PRIi16      "i"
#define PRIi32      "li"
#define PRIi64      "lli"

/* Unsigned Integers */
#define PRIu8       "u"
#define PRIu16      "u"
#define PRIu32      "lu"
#define PRIu64      "llu"

#define PRIo8       "o"
#define PRIo16      "o"
#define PRIo32      "lo"
#define PRIo64      "llo"

#define PRIx8       "x"
#define PRIx16      "x"
#define PRIx32      "lx"
#define PRIx64      "llx"

#define PRIX8       "X"
#define PRIX16      "X"
#define PRIX32      "lX"
#define PRIX64      "llX"

/* Pointer types */
#define PRIdPTR     "ld"
#define PRIiPTR     "li"
#define PRIuPTR     "lu"
#define PRIoPTR     "lo"
#define PRIxPTR     "lx"
#define PRIXPTR     "lX"

#ifdef __cplusplus
}
#endif

#endif /* !_INTTYPES_H_ */