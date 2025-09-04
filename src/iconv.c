/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 */

#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>  /* For SAS/C multibyte functions */
#include <wchar.h>  /* For wchar_t */
#include <proto/exec.h>
#include <proto/locale.h>
#include <libraries/locale.h>

/* Internal conversion descriptor structure */
typedef struct {
    const char *from_code;
    const char *to_code;
    int conversion_type;  /* Type of conversion to perform */
    struct Locale *locale;  /* AmigaOS locale for character conversion */
    struct LocaleBase *locale_base;  /* locale.library base */
} iconv_desc_t;

/* Conversion type constants */
#define CONV_IDENTITY    0  /* No conversion needed (same encoding) */
#define CONV_ASCII_TO_UTF8   1  /* ASCII to UTF-8 */
#define CONV_UTF8_TO_ASCII   2  /* UTF-8 to ASCII */
#define CONV_LATIN1_TO_UTF8  3  /* ISO-8859-1 to UTF-8 */
#define CONV_UTF8_TO_LATIN1  4  /* UTF-8 to ISO-8859-1 */
#define CONV_LATIN1_TO_ASCII 5  /* ISO-8859-1 to ASCII */
#define CONV_ASCII_TO_LATIN1 6  /* ASCII to ISO-8859-1 */
#define CONV_LOCALE_TO_UTF8  7  /* Locale charset to UTF-8 */
#define CONV_UTF8_TO_LOCALE  8  /* UTF-8 to locale charset */
#define CONV_LOCALE_TO_ASCII 9  /* Locale charset to ASCII */
#define CONV_ASCII_TO_LOCALE 10 /* ASCII to locale charset */

/* Helper function to normalize encoding names */
static const char *normalize_encoding(const char *encoding)
{
    if (!encoding) {
        return NULL;
    }
    
    /* Return as-is - we'll use stricmp for case-insensitive comparison */
    return encoding;
}

/* Helper function to get system locale */
static struct Locale *get_system_locale(struct LocaleBase **locale_base)
{
    struct LocaleBase *base;
    struct Locale *locale;
    
    /* Open locale.library */
    base = (struct LocaleBase *)OpenLibrary("locale.library", 0);
    if (!base) {
        return NULL;
    }
    
    /* Get system locale */
    locale = OpenLocale(NULL);
    if (!locale) {
        CloseLibrary((struct Library *)base);
        return NULL;
    }
    
    *locale_base = base;
    return locale;
}

/* Helper function to get locale charset name */
static const char *get_locale_charset(struct Locale *locale)
{
    if (!locale) {
        return "ISO-8859-1";  /* Default fallback */
    }
    
    /* Check locale's character set */
    switch (locale->loc_CodeSet) {
        case 0:  /* Legacy compatibility */
        case 4:  /* ISO-8859-1 */
            return "ISO-8859-1";
        case 106:  /* UTF-8 */
            return "UTF-8";
        case 1252:  /* Windows-1252 */
            return "CP1252";
        default:
            return "ISO-8859-1";  /* Default fallback */
    }
}

/* Helper function to convert character using locale.library */
static ULONG convert_char_with_locale(struct Locale *locale, ULONG character, int to_upper)
{
    if (!locale) {
        return character;  /* No conversion if no locale */
    }
    
    if (to_upper) {
        return ConvToUpper(locale, character);
    } else {
        return ConvToLower(locale, character);
    }
}

/* Helper function to check if character is valid in locale */
static int is_valid_locale_char(struct Locale *locale, ULONG character)
{
    if (!locale) {
        /* Without locale, assume all 8-bit characters are valid */
        return (character <= 0xFF) ? 1 : 0;
    }
    
    /* Use locale.library character classification functions */
    /* Check if character is printable (not control character) */
    return IsPrint(locale, character) ? 1 : 0;
}

/* Helper function to convert string case using locale.library */
static void convert_string_case(struct Locale *locale, const char *input, char *output, 
                               size_t input_len, int to_upper)
{
    size_t i;
    
    for (i = 0; i < input_len; i++) {
        ULONG input_char = (unsigned char)input[i];
        ULONG converted_char = convert_char_with_locale(locale, input_char, to_upper);
        output[i] = (char)(converted_char & 0xFF);
    }
}

/* Helper functions to leverage SAS/C multibyte character functions */

/* Check if current locale supports multibyte characters */
static int locale_supports_multibyte(void)
{
    /* Check if locale has state-dependent encodings */
    return (mblen(NULL, 0) != 0);
}

/* Convert multibyte string to wide character string using SAS/C */
static size_t convert_mb_to_wc(const char *mb_str, wchar_t *wc_str, size_t max_wc)
{
    if (!locale_supports_multibyte()) {
        /* Fallback: treat as single-byte characters */
        size_t i;
        for (i = 0; i < max_wc && mb_str[i] != '\0'; i++) {
            wc_str[i] = (wchar_t)(unsigned char)mb_str[i];
        }
        return i;
    }
    
    /* Use SAS/C's mbstowcs function */
    return mbstowcs(wc_str, mb_str, max_wc);
}

/* Convert wide character string to multibyte string using SAS/C */
static size_t convert_wc_to_mb(char *mb_str, const wchar_t *wc_str, size_t max_mb)
{
    if (!locale_supports_multibyte()) {
        /* Fallback: treat as single-byte characters */
        size_t i;
        for (i = 0; i < max_mb && wc_str[i] != WCHAR_NULL; i++) {
            mb_str[i] = (char)wc_str[i];
        }
        return i;
    }
    
    /* Use SAS/C's wcstombs function */
    return wcstombs(mb_str, wc_str, max_mb);
}

/* Get length of multibyte character using SAS/C */
static int get_mb_char_length(const char *mb_str)
{
    if (!locale_supports_multibyte()) {
        /* Fallback: single-byte character */
        return 1;
    }
    
    /* Use SAS/C's mblen function */
    return mblen(mb_str, MB_CUR_MAX);
}

/* Convert single multibyte character to wide character using SAS/C */
static int convert_mb_char_to_wc(const char *mb_char, wchar_t *wc_char)
{
    if (!locale_supports_multibyte()) {
        /* Fallback: single-byte character */
        *wc_char = (wchar_t)(unsigned char)*mb_char;
        return 1;
    }
    
    /* Use SAS/C's mbtowc function */
    return mbtowc(wc_char, mb_char, MB_CUR_MAX);
}

/* Convert single wide character to multibyte character using SAS/C */
static int convert_wc_char_to_mb(char *mb_char, wchar_t wc_char)
{
    if (!locale_supports_multibyte()) {
        /* Fallback: single-byte character */
        *mb_char = (char)wc_char;
        return 1;
    }
    
    /* Use SAS/C's wctomb function */
    return wctomb(mb_char, wc_char);
}

/* Helper function to determine conversion type */
static int get_conversion_type(const char *from, const char *to)
{
    const char *from_norm = normalize_encoding(from);
    const char *to_norm = normalize_encoding(to);
    
    if (!from_norm || !to_norm) {
        return -1;
    }
    
    /* Check for identity conversion - case insensitive */
    if (stricmp(from_norm, to_norm) == 0) {
        return CONV_IDENTITY;
    }
    
    /* ASCII conversions - case insensitive */
    if (stricmp(from_norm, "ASCII") == 0 && stricmp(to_norm, "UTF-8") == 0) {
        return CONV_ASCII_TO_UTF8;
    }
    if (stricmp(from_norm, "UTF-8") == 0 && stricmp(to_norm, "ASCII") == 0) {
        return CONV_UTF8_TO_ASCII;
    }
    if (stricmp(from_norm, "ASCII") == 0 && stricmp(to_norm, "ISO-8859-1") == 0) {
        return CONV_ASCII_TO_LATIN1;
    }
    if (stricmp(from_norm, "ISO-8859-1") == 0 && stricmp(to_norm, "ASCII") == 0) {
        return CONV_LATIN1_TO_ASCII;
    }
    
    /* UTF-8 conversions - case insensitive */
    if (stricmp(from_norm, "ISO-8859-1") == 0 && stricmp(to_norm, "UTF-8") == 0) {
        return CONV_LATIN1_TO_UTF8;
    }
    if (stricmp(from_norm, "UTF-8") == 0 && stricmp(to_norm, "ISO-8859-1") == 0) {
        return CONV_UTF8_TO_LATIN1;
    }
    
    /* Locale-aware conversions - case insensitive */
    if (stricmp(from_norm, "LOCALE") == 0 && stricmp(to_norm, "UTF-8") == 0) {
        return CONV_LOCALE_TO_UTF8;
    }
    if (stricmp(from_norm, "UTF-8") == 0 && stricmp(to_norm, "LOCALE") == 0) {
        return CONV_UTF8_TO_LOCALE;
    }
    if (stricmp(from_norm, "LOCALE") == 0 && stricmp(to_norm, "ASCII") == 0) {
        return CONV_LOCALE_TO_ASCII;
    }
    if (stricmp(from_norm, "ASCII") == 0 && stricmp(to_norm, "LOCALE") == 0) {
        return CONV_ASCII_TO_LOCALE;
    }
    
    /* Unsupported conversion */
    return -1;
}

iconv_t iconv_open(const char *tocode, const char *fromcode)
{
    iconv_desc_t *desc;
    int conv_type;
    struct Locale *locale = NULL;
    struct LocaleBase *locale_base = NULL;
    
    /* Validate input parameters */
    if (!tocode || !fromcode) {
        errno = EINVAL;
        return (iconv_t)-1;
    }
    
    /* Determine conversion type */
    conv_type = get_conversion_type(fromcode, tocode);
    if (conv_type < 0) {
        errno = EINVAL;
        return (iconv_t)-1;
    }
    
    /* Get system locale for locale-aware conversions */
    if (conv_type >= CONV_LOCALE_TO_UTF8) {
        locale = get_system_locale(&locale_base);
        if (!locale) {
            /* Fall back to non-locale conversion if locale.library unavailable */
            if (conv_type == CONV_LOCALE_TO_UTF8) {
                conv_type = CONV_LATIN1_TO_UTF8;
            } else if (conv_type == CONV_UTF8_TO_LOCALE) {
                conv_type = CONV_UTF8_TO_LATIN1;
            } else if (conv_type == CONV_LOCALE_TO_ASCII) {
                conv_type = CONV_LATIN1_TO_ASCII;
            } else if (conv_type == CONV_ASCII_TO_LOCALE) {
                conv_type = CONV_ASCII_TO_LATIN1;
            }
        }
    }
    
    /* Allocate conversion descriptor */
    desc = (iconv_desc_t *)malloc(sizeof(iconv_desc_t));
    if (!desc) {
        if (locale_base) {
            CloseLocale(locale);
            CloseLibrary((struct Library *)locale_base);
        }
        errno = ENOMEM;
        return (iconv_t)-1;
    }
    
    /* Initialize descriptor */
    desc->from_code = fromcode;
    desc->to_code = tocode;
    desc->conversion_type = conv_type;
    desc->locale = locale;
    desc->locale_base = locale_base;
    
    return (iconv_t)desc;
}

size_t iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
             char **outbuf, size_t *outbytesleft)
{
    iconv_desc_t *desc = (iconv_desc_t *)cd;
    const char *inptr;
    char *outptr;
    size_t inleft, outleft;
    size_t converted = 0;
    
    /* Validate conversion descriptor */
    if (!desc) {
        errno = EBADF;
        return (size_t)-1;
    }
    
    /* Handle NULL input buffer case (reset conversion state) */
    if (!inbuf || !*inbuf) {
        /* For stateless conversions, this is a no-op */
        return 0;
    }
    
    /* Get current buffer positions and sizes */
    inptr = *inbuf;
    outptr = *outbuf;
    inleft = *inbytesleft;
    outleft = *outbytesleft;
    
    /* Perform conversion based on type */
    switch (desc->conversion_type) {
        case CONV_IDENTITY:
            /* No conversion needed - just copy bytes */
            {
                size_t copy_len = (inleft < outleft) ? inleft : outleft;
                memcpy(outptr, inptr, copy_len);
                inptr += copy_len;
                outptr += copy_len;
                inleft -= copy_len;
                outleft -= copy_len;
                converted = copy_len;
            }
            break;
            
        case CONV_ASCII_TO_UTF8:
            /* ASCII to UTF-8: each ASCII byte becomes a UTF-8 byte */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                if (c < 0x80) {
                    /* ASCII character - direct copy */
                    *outptr = c;
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                } else {
                    /* Invalid ASCII character */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
            }
            break;
            
        case CONV_UTF8_TO_ASCII:
            /* UTF-8 to ASCII: only 7-bit characters allowed */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                if (c < 0x80) {
                    /* Valid ASCII character */
                    *outptr = c;
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                } else {
                    /* Invalid for ASCII output */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
            }
            break;
            
        case CONV_LATIN1_TO_UTF8:
            /* ISO-8859-1 to UTF-8: convert to UTF-8 encoding using SAS/C multibyte functions */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                wchar_t wc = (wchar_t)c;
                
                /* Convert wide character to multibyte (UTF-8) */
                char mb_buf[MB_CUR_MAX];
                int mb_len = convert_wc_char_to_mb(mb_buf, wc);
                if (mb_len <= 0) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                if (outleft < mb_len) {
                    errno = E2BIG;
                    break;  /* Convert as much as possible before stopping */
                }
                
                /* Copy the multibyte character to output */
                memcpy(outptr, mb_buf, mb_len);
                inptr++;
                outptr += mb_len;
                inleft--;
                outleft -= mb_len;
                converted++;
            }
            break;
            
        case CONV_UTF8_TO_LATIN1:
            /* 
             * UTF-8 to ISO-8859-1: convert from UTF-8 encoding
             * Enhanced to use SAS/C multibyte functions for better UTF-8 handling
             */
            while (inleft > 0 && outleft > 0) {
                /* Get length of current multibyte character */
                int mb_len = get_mb_char_length(inptr);
                if (mb_len <= 0) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                if (inleft < mb_len) {
                    errno = EINVAL;
                    return (size_t)-1;
                }
                
                /* Convert multibyte character to wide character */
                wchar_t wc;
                int wc_len = convert_mb_char_to_wc(inptr, &wc);
                if (wc_len <= 0) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                /* Check if wide character fits in Latin-1 range */
                if (wc > 0xFF) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                /* Output the Latin-1 character */
                *outptr = (char)wc;
                inptr += mb_len;
                outptr++;
                inleft -= mb_len;
                outleft--;
                converted++;
            }
            break;
            
        case CONV_LATIN1_TO_ASCII:
            /* ISO-8859-1 to ASCII: only 7-bit characters allowed */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                if (c < 0x80) {
                    /* Valid ASCII character */
                    *outptr = c;
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                } else {
                    /* Invalid for ASCII output */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
            }
            break;
            
        case CONV_ASCII_TO_LATIN1:
            /* ASCII to ISO-8859-1: direct copy (ASCII is subset of Latin-1) */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                if (c < 0x80) {
                    /* Valid ASCII character */
                    *outptr = c;
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                } else {
                    /* Invalid ASCII character */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
            }
            break;
            
        case CONV_LOCALE_TO_UTF8:
            /* 
             * Locale charset to UTF-8 using SAS/C multibyte functions and locale.library
             * This implementation uses locale.library for character validation
             * and SAS/C multibyte functions for UTF-8 conversion.
             */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                ULONG locale_char = c;
                
                /* Use locale.library to validate character */
                if (desc->locale && !is_valid_locale_char(desc->locale, locale_char)) {
                    /* Invalid character for this locale */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                /* Convert to wide character and then to multibyte (UTF-8) */
                wchar_t wc = (wchar_t)c;
                char mb_buf[MB_CUR_MAX];
                int mb_len = convert_wc_char_to_mb(mb_buf, wc);
                if (mb_len <= 0) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                if (outleft < mb_len) {
                    errno = E2BIG;
                    break;  /* Convert as much as possible before stopping */
                }
                
                /* Copy the multibyte character to output */
                memcpy(outptr, mb_buf, mb_len);
                inptr++;
                outptr += mb_len;
                inleft--;
                outleft -= mb_len;
                converted++;
            }
            break;
            
        case CONV_UTF8_TO_LOCALE:
            /* 
             * UTF-8 to locale charset using SAS/C multibyte functions and locale.library
             * Enhanced to use SAS/C multibyte functions for better UTF-8 handling
             */
            while (inleft > 0 && outleft > 0) {
                /* Get length of current multibyte character */
                int mb_len = get_mb_char_length(inptr);
                if (mb_len <= 0) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                if (inleft < mb_len) {
                    errno = EINVAL;
                    return (size_t)-1;
                }
                
                /* Convert multibyte character to wide character */
                wchar_t wc;
                int wc_len = convert_mb_char_to_wc(inptr, &wc);
                if (wc_len <= 0) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                /* Check if wide character fits in 8-bit range */
                if (wc > 0xFF) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                /* Validate character with locale.library */
                ULONG locale_char = (ULONG)wc;
                if (desc->locale && !is_valid_locale_char(desc->locale, locale_char)) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                /* Output the locale character */
                *outptr = (char)wc;
                inptr += mb_len;
                outptr++;
                inleft -= mb_len;
                outleft--;
                converted++;
            }
            break;
            
        case CONV_LOCALE_TO_ASCII:
            /* Locale charset to ASCII using locale.library */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                ULONG locale_char = c;
                
                /* Use locale.library to validate character */
                if (desc->locale && !is_valid_locale_char(desc->locale, locale_char)) {
                    errno = EILSEQ;
                    return (size_t)-1;
                }
                
                if (c < 0x80) {
                    /* Valid ASCII character */
                    *outptr = c;
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                } else {
                    /* Non-ASCII character - convert to '?' or similar */
                    *outptr = '?';
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                }
            }
            break;
            
        case CONV_ASCII_TO_LOCALE:
            /* ASCII to locale charset using locale.library */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                if (c < 0x80) {
                    /* Valid ASCII character - validate with locale */
                    ULONG locale_char = c;
                    if (desc->locale && !is_valid_locale_char(desc->locale, locale_char)) {
                        errno = EILSEQ;
                        return (size_t)-1;
                    }
                    *outptr = c;
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                } else {
                    /* Invalid ASCII character */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
            }
            break;
            
        default:
            errno = EINVAL;
            return (size_t)-1;
    }
    
    /* Update buffer pointers and sizes */
    *inbuf = inptr;
    *outbuf = outptr;
    *inbytesleft = inleft;
    *outbytesleft = outleft;
    
    return converted;
}

int iconv_close(iconv_t cd)
{
    iconv_desc_t *desc = (iconv_desc_t *)cd;
    
    /* Validate conversion descriptor */
    if (!desc) {
        errno = EBADF;
        return -1;
    }
    
    /* Clean up locale resources */
    if (desc->locale) {
        CloseLocale(desc->locale);
    }
    if (desc->locale_base) {
        CloseLibrary((struct Library *)desc->locale_base);
    }
    
    /* Free the descriptor */
    free(desc);
    
    return 0;
}
