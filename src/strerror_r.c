/*
 * strerror_r.c - thread-safe error message conversion (POSIX.1-2001)
 * 
 * This function converts an error number to a string message and stores
 * it in the provided buffer, making it thread-safe unlike strerror().
 * 
 * POSIX.1-2001: thread-safe error message conversion
 */

#include <string.h>
#include <errno.h>

/**
 * @brief Thread-safe error message conversion
 * @param errnum Error number
 * @param buf Buffer to store error message
 * @param buflen Size of buffer
 * @return 0 on success, -1 on failure
 * 
 * This function converts the error number errnum to a string message
 * and stores it in the buffer buf. The buffer must be at least
 * buflen characters long.
 * 
 * The function:
 * - Is thread-safe (no static state)
 * - Returns 0 on success, -1 on failure
 * - Always null-terminates the result
 * - Truncates long messages to fit buffer
 * - Handles unknown error numbers gracefully
 */
int strerror_r(int errnum, char *buf, size_t buflen)
{
    const char *msg;
    
    if (buf == NULL || buflen == 0) {
        return -1;
    }
    
    /* Get the error message */
    switch (errnum) {
        case EPERM:           msg = "Operation not permitted"; break;
        case ENOENT:          msg = "No such file or directory"; break;
        case ESRCH:           msg = "No such process"; break;
        case EINTR:           msg = "Interrupted system call"; break;
        case EIO:             msg = "Input/output error"; break;
        case ENXIO:           msg = "Device not configured"; break;
        case E2BIG:           msg = "Argument list too long"; break;
        case ENOEXEC:         msg = "Exec format error"; break;
        case EBADF:           msg = "Bad file descriptor"; break;
        case ECHILD:          msg = "No child processes"; break;
        case EDEADLK:         msg = "Resource deadlock avoided"; break;
        case ENOMEM:          msg = "Cannot allocate memory"; break;
        case EACCES:          msg = "Permission denied"; break;
        case EFAULT:          msg = "Bad address"; break;
        case ENOTBLK:         msg = "Block device required"; break;
        case EBUSY:           msg = "Device busy"; break;
        case EEXIST:          msg = "File exists"; break;
        case EXDEV:           msg = "Cross-device link"; break;
        case ENODEV:          msg = "Operation not supported by device"; break;
        case ENOTDIR:         msg = "Not a directory"; break;
        case EISDIR:          msg = "Is a directory"; break;
        case EINVAL:          msg = "Invalid argument"; break;
        case ENFILE:          msg = "Too many open files in system"; break;
        case EMFILE:          msg = "Too many open files"; break;
        case ENOTTY:          msg = "Inappropriate ioctl for device"; break;
        case ETXTBSY:         msg = "Text file busy"; break;
        case EFBIG:           msg = "File too large"; break;
        case ENOSPC:          msg = "No space left on device"; break;
        case ESPIPE:          msg = "Illegal seek"; break;
        case EROFS:           msg = "Read-only file system"; break;
        case EMLINK:          msg = "Too many links"; break;
        case EPIPE:           msg = "Broken pipe"; break;
        case EDOM:            msg = "Numerical argument out of domain"; break;
        case ERANGE:          msg = "Result too large"; break;
        case EAGAIN:          msg = "Resource temporarily unavailable"; break;
        case EINPROGRESS:     msg = "Operation now in progress"; break;
        case EALREADY:        msg = "Operation already in progress"; break;
        case ENOTSOCK:        msg = "Socket operation on non-socket"; break;
        case EDESTADDRREQ:    msg = "Destination address required"; break;
        case EMSGSIZE:        msg = "Message too long"; break;
        case EPROTOTYPE:      msg = "Protocol wrong type for socket"; break;
        case ENOPROTOOPT:     msg = "Protocol not available"; break;
        case EPROTONOSUPPORT: msg = "Protocol not supported"; break;
        case ESOCKTNOSUPPORT: msg = "Socket type not supported"; break;
        case EOPNOTSUPP:      msg = "Operation not supported"; break;
        case EPFNOSUPPORT:    msg = "Protocol family not supported"; break;
        case EAFNOSUPPORT:    msg = "Address family not supported by protocol family"; break;
        case EADDRINUSE:      msg = "Address already in use"; break;
        case EADDRNOTAVAIL:   msg = "Can't assign requested address"; break;
        case ENETDOWN:        msg = "Network is down"; break;
        case ENETUNREACH:     msg = "Network is unreachable"; break;
        case ENETRESET:       msg = "Network dropped connection on reset"; break;
        case ECONNABORTED:    msg = "Software caused connection abort"; break;
        case ECONNRESET:      msg = "Connection reset by peer"; break;
        case ENOBUFS:         msg = "No buffer space available"; break;
        case EISCONN:         msg = "Socket is already connected"; break;
        case ENOTCONN:        msg = "Socket is not connected"; break;
        case ESHUTDOWN:       msg = "Can't send after socket shutdown"; break;
        case ETOOMANYREFS:    msg = "Too many references: can't splice"; break;
        case ETIMEDOUT:       msg = "Operation timed out"; break;
        case ECONNREFUSED:    msg = "Connection refused"; break;
        case ELOOP:           msg = "Too many levels of symbolic links"; break;
        case ENAMETOOLONG:    msg = "File name too long"; break;
        case EHOSTDOWN:       msg = "Host is down"; break;
        case EHOSTUNREACH:    msg = "No route to host"; break;
        case ENOTEMPTY:       msg = "Directory not empty"; break;
        case EPROCLIM:        msg = "Too many processes"; break;
        case EUSERS:          msg = "Too many users"; break;
        case EDQUOT:          msg = "Disc quota exceeded"; break;
        case ESTALE:          msg = "Stale NFS file handle"; break;
        case EREMOTE:         msg = "Too many levels of remote in path"; break;
        case EBADRPC:         msg = "RPC struct is bad"; break;
        case ERPCMISMATCH:    msg = "RPC version wrong"; break;
        case EPROGUNAVAIL:    msg = "RPC prog. not avail"; break;
        case EPROGMISMATCH:   msg = "Program version wrong"; break;
        case EPROCUNAVAIL:    msg = "Bad procedure for program"; break;
        case ENOLCK:          msg = "No locks available"; break;
        case ENOSYS:          msg = "Function not implemented"; break;
        case EFTYPE:          msg = "Inappropriate file type or format"; break;
        case EAUTH:           msg = "Authentication error"; break;
        case ENEEDAUTH:       msg = "Need authenticator"; break;
        default:              msg = "Unknown error"; break;
    }
    
    /* Copy message to buffer, ensuring null termination */
    if (strlen(msg) >= buflen) {
        /* Truncate message to fit buffer */
        strncpy(buf, msg, buflen - 1);
        buf[buflen - 1] = '\0';
    } else {
        /* Copy entire message */
        strcpy(buf, msg);
    }
    
    return 0;
}
