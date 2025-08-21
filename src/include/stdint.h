#ifndef _STDINT_H_
#define _STDINT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* --- Exact-width integer types --- */
typedef signed char         int8_t;
typedef short int           int16_t;
typedef long int            int32_t;
/* SAS/C supports long long */
typedef long long int       int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned long int   uint32_t;
typedef unsigned long long int uint64_t;

/* --- Minimum-width integer types --- */
typedef int8_t              int_least8_t;
typedef int16_t             int_least16_t;
typedef int32_t             int_least32_t;
typedef int64_t             int_least64_t;

typedef uint8_t             uint_least8_t;
typedef uint16_t            uint_least16_t;
typedef uint32_t            uint_least32_t;
typedef uint64_t            uint_least64_t;

/* --- Fastest minimum-width integer types --- */
typedef int                 int_fast8_t;
typedef int                 int_fast16_t;
typedef long int            int_fast32_t;
typedef long long int       int_fast64_t;

typedef unsigned int        uint_fast8_t;
typedef unsigned int        uint_fast16_t;
typedef unsigned long int   uint_fast32_t;
typedef unsigned long long int uint_fast64_t;

/* --- Integer types capable of holding object pointers --- */
typedef long int            intptr_t;
typedef unsigned long int   uintptr_t;

/* --- Greatest-width integer types --- */
typedef long long int       intmax_t;
typedef unsigned long long int uintmax_t;


/* --- Limits of exact-width integer types --- */
#define INT8_MIN            (-128)
#define INT16_MIN           (-32768)
#define INT32_MIN           (-2147483647L - 1)
#define INT64_MIN           (-9223372036854775807LL - 1)

#define INT8_MAX            (127)
#define INT16_MAX           (32767)
#define INT32_MAX           (2147483647L)
#define INT64_MAX           (9223372036854775807LL)

#define UINT8_MAX           (255U)
#define UINT16_MAX          (65535U)
#define UINT32_MAX          (4294967295UL)
#define UINT64_MAX          (18446744073709551615ULL)

/* --- Limits of other integer types --- */
#define INTPTR_MIN          INT32_MIN
#define INTPTR_MAX          INT32_MAX
#define UINTPTR_MAX         UINT32_MAX

#define SIZE_MAX            UINT32_MAX

#ifdef __cplusplus
}
#endif

#endif /* !_STDINT_H_ */