#ifndef _SYS_UTSNAME_H_
#define _SYS_UTSNAME_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The POSIX standard does not specify the size of the arrays.
 * 256 is a common and safe size used by many systems.
 */
#define _SYS_NMLN 256

/*
 * Structure for holding system identification information,
 * as populated by the uname() function.
 */
struct utsname {
    char sysname[_SYS_NMLN];   /* Operating system name (e.g., "AmigaOS") */
    char nodename[_SYS_NMLN];  /* Name of this node on the network (hostname) */
    char release[_SYS_NMLN];   /* OS release level (e.g., "3.2.2") */
    char version[_SYS_NMLN];   /* OS version level (e.g., "47.8") */
    char machine[_SYS_NMLN];   /* Hardware identifier (e.g., "m68k") */
};

/*
 * Function to get system identification information.
 * It populates the provided utsname structure.
 * Returns a non-negative integer on success, or -1 on failure.
 */
int uname(struct utsname *name);


#ifdef __cplusplus
}
#endif

#endif /* !_SYS_UTSNAME_H_ */