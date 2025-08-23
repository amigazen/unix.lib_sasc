#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#include <sys/types.h> /* For mode_t, off_t, size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* --- Protections for memory regions --- */
#define PROT_NONE       0x00    /* Page can not be accessed. */
#define PROT_READ       0x01    /* Page can be read. */
#define PROT_WRITE      0x02    /* Page can be written. */
#define PROT_EXEC       0x04    /* Page can be executed. */

/* --- Flags for mmap() --- */
#define MAP_SHARED      0x001   /* Share changes. */
#define MAP_PRIVATE     0x002   /* Changes are private. */
#define MAP_FIXED       0x010   /* Interpret addr exactly. */

/* Non-POSIX, but common extension for anonymous memory */
#define MAP_ANONYMOUS   0x020
#define MAP_ANON        MAP_ANONYMOUS

/* Return value of mmap on failure. */
#define MAP_FAILED      ((void *)-1)

/* --- C89-compliant function prototypes --- */
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int   munmap(void *addr, size_t len);
int   mprotect(void *addr, size_t len, int prot);
int   msync(void *addr, size_t len, int flags);

#ifdef __cplusplus
}
#endif

#endif /* !_SYS_MMAN_H_ */
