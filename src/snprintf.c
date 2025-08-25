#include <stdarg.h>
#include <stddef.h>  // For size_t
#include <string.h>  // For strchr, strncpy
#include <ctype.h>   // For isdigit
#include <math.h>    // For isnan, isinf

#include "amiga.h"  // For LONG, UBYTE, VSNPrintf, APTR
#include "stdio.h"  // For our function declarations

/* Format string processing states */
#define FMT_SAFE_DIRECT    0  /* Safe to pass to OS directly */
#define FMT_NEEDS_PREPROC  1  /* Needs preprocessing then can pass to OS */
#define FMT_NEEDS_FALLBACK 2  /* Cannot process by OS, need fallback function */

/*
 * Modern POSIX-compatible snprintf() function implementation for AmigaOS.
 * This function formats output according to the format string and writes
 * up to bufsize-1 characters to the buffer, ensuring NUL termination.
 * 
 * Returns the number of characters that would have been written if
 * bufsize had been sufficiently large, not counting the terminating NUL.
 */

/* Forward declarations for fallback functions */
int __snprintf(char *buffer, size_t bufsize, const char *fmt, ...);

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

/* Format string rewriter - converts standard C format to VSNPrintf compatible */
static int rewrite_format_string(char *new_fmt, size_t new_fmt_size, const char *fmt)
{
    const char *src = fmt;
    char *dst = new_fmt;
    size_t remaining = new_fmt_size - 1; /* Leave room for NUL */
    
    while (*src && remaining > 0) {
        if (*src == '%') {
            *dst++ = *src++; /* Copy the % */
            remaining--;
            
            /* Skip flags */
            while (*src && remaining > 0 && 
                   (*src == '-' || *src == '+' || *src == ' ' || *src == '#' || *src == '0')) {
                *dst++ = *src++;
                remaining--;
            }
            
            /* Skip width */
            while (*src && remaining > 0 && isdigit(*src)) {
                *dst++ = *src++;
                remaining--;
            }
            
            /* Skip precision */
            if (*src == '.' && remaining > 0) {
                *dst++ = *src++;
                remaining--;
                while (*src && remaining > 0 && isdigit(*src)) {
                    *dst++ = *src++;
                    remaining--;
                }
            }
            
            /* Handle length modifiers - convert to VSNPrintf compatible */
            if (*src && remaining > 0) {
                if (*src == 'd' || *src == 'u' || *src == 'x' || *src == 'c') {
                    /* Check if we need to add 'l' for 32-bit compatibility */
                    const char *prev = dst - 1;
                    int has_l = 0;
                    
                    /* Look back for 'l' modifier */
                    while (prev >= new_fmt && (*prev == 'h' || *prev == 'l' || *prev == 'L')) {
                        if (*prev == 'l') {
                            has_l = 1;
                            break;
                        }
                        prev--;
                    }
                    
                    /* Add 'l' if not present for 32-bit compatibility */
                    if (!has_l && remaining > 0) {
                        *dst++ = 'l';
                        remaining--;
                    }
                }
            }
            
            /* Copy the type specifier */
            if (*src && remaining > 0) {
                *dst++ = *src++;
                remaining--;
            }
        } else {
            /* Copy regular character */
            *dst++ = *src++;
            remaining--;
        }
    }
    
    *dst = '\0'; /* NUL terminate */
    return (dst - new_fmt);
}

int snprintf(char *buffer, size_t bufsize, const char *fmt, ...)
{
    va_list args;
    int result;
    int can_process;
    char new_fmt[256]; /* Buffer for rewritten format string */
    
    /* Check for valid parameters */
    if (buffer == NULL && bufsize != 0) {
        return -1;
    }
    
    /* Initialize variable argument list */
    va_start(args, fmt);
    
    /* Check if we can use VSNPrintf or need fallback */
    can_process = can_use_vsnprintf(fmt);
    
    if (can_process == FMT_SAFE_DIRECT) {
        /* Safe to pass to OS directly - no preprocessing needed */
        result = vsnprintf(buffer, bufsize, fmt, args);
    } else if (can_process == FMT_NEEDS_PREPROC) {
        /* Needs preprocessing then can pass to OS */
        if (rewrite_format_string(new_fmt, sizeof(new_fmt), fmt) > 0) {
            result = vsnprintf(buffer, bufsize, new_fmt, args);
        } else {
            /* Fallback if format rewriting fails */
            result = __snprintf(buffer, bufsize, fmt, args);
        }
    } else {
        /* Cannot process by OS - use fallback implementation */
        result = __snprintf(buffer, bufsize, fmt, args);
    }
    
    /* Clean up variable argument list */
    va_end(args);
    
    return result;
}

/* Fallback implementation for unsupported format specifiers */
int __snprintf(char *buffer, size_t bufsize, const char *fmt, ...)
{
    va_list args;
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
    
    va_start(args, fmt);
    
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
    
    va_end(args);
    return total_written;
}
