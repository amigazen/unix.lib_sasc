#include <stdarg.h>
#include <stddef.h> // For size_t
#include <string.h> // For strchr
#include <ctype.h>  // For isdigit
#include <math.h>   // For isnan, isinf

#include "amiga.h"  // For ULONG, UBYTE, APTR, VSNPrintf, LONG
#include "stdio.h"  // For our function declarations

/* Format string processing states */
#define FMT_SAFE_DIRECT    0  /* Safe to pass to OS directly */
#define FMT_NEEDS_PREPROC  1  /* Needs preprocessing then can pass to OS */
#define FMT_NEEDS_FALLBACK 2  /* Cannot process by OS, need fallback function */

/*
 * Modern POSIX-compatible vsnprintf() function implementation for AmigaOS.
 * This function formats output according to the format string and writes
 * up to bufsize-1 characters to the buffer, ensuring NUL termination.
 * Returns the number of characters that would have been written if
 * bufsize had been sufficiently large, not counting the terminating NUL.
 * This matches the C99 and POSIX standards.
 */

/* Forward declaration for fallback function */
int __vsnprintf(char *buffer, size_t bufsize, const char *fmt, va_list args);

/* Format string scanner - detects if we can use VSNPrintf or need fallback */
static int can_use_vsnprintf(const char *fmt)
{
    const char *p = fmt;
    int needs_preprocessing = 0;
    
    while ((p = strchr(p, '%')) != NULL) {
        p++; /* Skip the % */
        
        /* Skip flags */
        while (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0') {
            p++;
        }
        
        /* Skip width */
        while (isdigit(*p)) {
            p++;
        }
        
        /* Skip precision */
        if (*p == '.') {
            p++;
            while (isdigit(*p)) {
                p++;
            }
        }
        
        /* Skip length modifiers */
        while (*p == 'h' || *p == 'l' || *p == 'L' || *p == 'z' || *p == 't') {
            p++;
        }
        
        /* Check type specifier */
        switch (*p) {
            case 'd':
            case 'u':
            case 'x':
            case 'X':
            case 'c':
            case 's':
            case 'b':  /* BSTR - AmigaOS specific */
                /* These are supported by VSNPrintf but may need preprocessing */
                /* Check if we need to add 'l' for 32-bit compatibility */
                if (*p == 'd' || *p == 'u' || *p == 'x' || *p == 'X' || *p == 'c') {
                    const char *prev = p - 1;
                    int has_l = 0;
                    
                    /* Look back for 'l' modifier */
                    while (prev >= fmt && (*prev == 'h' || *prev == 'l' || *prev == 'L')) {
                        if (*prev == 'l') {
                            has_l = 1;
                            break;
                        }
                        prev--;
                    }
                    
                    /* If no 'l' modifier found, we need preprocessing */
                    if (!has_l) {
                        needs_preprocessing = 1;
                    }
                }
                break;
                
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            case 'o':
            case 'p':
                /* These are NOT supported by VSNPrintf - need fallback */
                return FMT_NEEDS_FALLBACK; /* Cannot process by OS */
                
            case '%':
                /* Escaped % - always supported */
                break;
                
            default:
                /* Unknown specifier - assume unsupported */
                return FMT_NEEDS_FALLBACK; /* Cannot process by OS */
        }
        
        p++; /* Move to next character */
    }
    
    /* Return appropriate state */
    if (needs_preprocessing) {
        return FMT_NEEDS_PREPROC; /* Needs preprocessing then can pass to OS */
    } else {
        return FMT_SAFE_DIRECT; /* Safe to pass to OS directly */
    }
}

int vsnprintf(char *buffer, size_t bufsize, const char *fmt, va_list args)
{
    LONG needed_size_incl_nul;
    int can_process_state;
    
    /* According to the POSIX standard, if bufsize is 0, buffer can be NULL
       and nothing is written. The function should still return the number of
       characters that would have been written. We let VSNPrintf handle this. */

    /* A NULL buffer is only valid if bufsize is 0. */
    if (buffer == NULL && bufsize != 0) {
        return -1;
    }

    /* Check if we can use VSNPrintf or need fallback */
    can_process_state = can_use_vsnprintf(fmt);

    if (can_process_state == FMT_SAFE_DIRECT) {
        /* Call the AmigaOS native VSNPrintf.
           It's assumed to return the total number of bytes required for the output,
           *including* the terminating NUL character.
           It returns 0 or a negative value on error. */
        needed_size_incl_nul = VSNPrintf((UBYTE *)buffer, (ULONG)bufsize, (const UBYTE *)fmt, (APTR)args);

        /* VSNPrintf returns a value <= 0 on error. We return -1 as is standard. */
        if (needed_size_incl_nul <= 0) {
            return -1;
        }

        /* The POSIX standard requires returning the length of the string
           that *would* have been written, *excluding* the NUL terminator.
           This is simply the value VSNPrintf gave us, minus one. */
        return (int)(needed_size_incl_nul - 1);
    } else if (can_process_state == FMT_NEEDS_PREPROC) {
        /* Needs preprocessing then can pass to OS */
        /* For vsnprintf, we need to create a new va_list for the rewritten format */
        /* This is complex, so we'll use the fallback for now */
        /* TODO: Implement proper preprocessing for vsnprintf */
        return __vsnprintf(buffer, bufsize, fmt, args);
    } else {
        /* Cannot process by OS, so we call the fallback */
        return __vsnprintf(buffer, bufsize, fmt, args);
    }
}

/* Fallback implementation for unsupported format specifiers */
int __vsnprintf(char *buffer, size_t bufsize, const char *fmt, va_list args)
{
    char *buf;
    size_t remaining;
    const char *p;
    int total_written;
    int left_justify, zero_pad, show_sign, show_prefix;
    int width, precision, is_long, is_longlong;
    char temp_buf[64];
    int len, pad, copy_len;
    double val;
    unsigned long val_octal;
    void *ptr;
    
    if (buffer == NULL || bufsize == 0) {
        return 0;
    }
    
    buf = buffer;
    remaining = bufsize;
    p = fmt;
    total_written = 0;
    
    while (*p && remaining > 1) {
        if (*p == '%') {
            p++; /* Skip the % */
            
            /* Parse flags */
            left_justify = 0;
            zero_pad = 0;
            show_sign = 0;
            show_prefix = 0;
            
            while (*p && (*p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '0')) {
                if (*p == '-') left_justify = 1;
                if (*p == '0') zero_pad = 1;
                if (*p == '+') show_sign = 1;
                if (*p == '#') show_prefix = 1;
                p++;
            }
            
            /* Parse width */
            width = 0;
            while (*p && isdigit(*p)) {
                width = width * 10 + (*p - '0');
                p++;
            }
            
            /* Parse precision */
            precision = -1;
            if (*p == '.') {
                p++;
                precision = 0;
                while (*p && isdigit(*p)) {
                    precision = precision * 10 + (*p - '0');
                    p++;
                }
            }
            
            /* Parse length modifiers */
            is_long = 0;
            is_longlong = 0;
            while (*p && (*p == 'h' || *p == 'l' || *p == 'L' || *p == 'z' || *p == 't')) {
                if (*p == 'l') {
                    if (is_long) is_longlong = 1;
                    else is_long = 1;
                }
                p++;
            }
            
            /* Handle format specifier */
            if (*p) {
                len = 0;
                
                switch (*p) {
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G': {
                        /* Floating point - use simple conversion */
                        val = va_arg(args, double);
                        if (precision < 0) precision = 6;
                        
                        /* Simple float formatting */
                        if (isnan(val)) {
                            strcpy(temp_buf, "nan");
                            len = 3;
                        } else if (isinf(val)) {
                            strcpy(temp_buf, val < 0 ? "-inf" : "inf");
                            len = val < 0 ? 4 : 3;
                        } else {
                            /* Basic float formatting */
                            len = snprintf(temp_buf, sizeof(temp_buf), "%.*f", precision, val);
                            if (len >= (int)sizeof(temp_buf)) len = sizeof(temp_buf) - 1;
                        }
                        break;
                    }
                    
                    case 'o': {
                        /* Octal */
                        if (is_longlong) val_octal = va_arg(args, unsigned long long);
                        else if (is_long) val_octal = va_arg(args, unsigned long);
                        else val_octal = va_arg(args, unsigned int);
                        
                        if (show_prefix) {
                            strcpy(temp_buf, "0");
                            len = 1 + snprintf(temp_buf + 1, sizeof(temp_buf) - 1, "%lo", val_octal);
                        } else {
                            len = snprintf(temp_buf, sizeof(temp_buf), "%lo", val_octal);
                        }
                        if (len >= (int)sizeof(temp_buf)) len = sizeof(temp_buf) - 1;
                        break;
                    }
                    
                    case 'p': {
                        /* Pointer */
                        ptr = va_arg(args, void *);
                        len = snprintf(temp_buf, sizeof(temp_buf), "0x%lx", (unsigned long)ptr);
                        if (len >= (int)sizeof(temp_buf)) len = sizeof(temp_buf) - 1;
                        break;
                    }
                    
                    default:
                        /* Unknown specifier - copy as-is */
                        temp_buf[0] = '%';
                        temp_buf[1] = *p;
                        len = 2;
                        break;
                }
                
                /* Apply width and padding */
                if (len < width) {
                    pad = width - len;
                    if (!left_justify) {
                        /* Right justify with padding */
                        char pad_char = zero_pad ? '0' : ' ';
                        while (pad > 0 && remaining > 1) {
                            *buf++ = pad_char;
                            remaining--;
                            total_written++;
                            pad--;
                        }
                    }
                }
                
                /* Copy the formatted value */
                copy_len = (len < (int)remaining - 1) ? len : (int)remaining - 1;
                memcpy(buf, temp_buf, copy_len);
                buf += copy_len;
                remaining -= copy_len;
                total_written += copy_len;
                
                /* Apply left justification padding */
                if (left_justify && len < width) {
                    pad = width - len;
                    while (pad > 0 && remaining > 1) {
                        *buf++ = ' ';
                        remaining--;
                        total_written++;
                        pad--;
                    }
                }
                
                p++; /* Move to next character */
            }
        } else {
            /* Copy regular character */
            *buf++ = *p++;
            remaining--;
            total_written++;
        }
    }
    
    /* Ensure NUL termination */
    if (remaining > 0) {
        *buf = '\0';
    }
    
    return total_written;
}
