/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2025 amigazen project
 */

#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
            /* ISO-8859-1 to UTF-8: convert to UTF-8 encoding */
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
                    /* Latin-1 character (0x80-0xFF) - convert to 2-byte UTF-8 */
                    if (outleft < 2) {
                        errno = E2BIG;
                        break;  /* Convert as much as possible before stopping */
                    }
                    *outptr = (char)(0xC0 | (c >> 6));
                    outptr++;
                    *outptr = (char)(0x80 | (c & 0x3F));
                    outptr++;
                    inptr++;
                    inleft--;
                    outleft -= 2;
                    converted++;
                }
            }
            break;
            
        case CONV_UTF8_TO_LATIN1:
            /* 
             * UTF-8 to ISO-8859-1: convert from UTF-8 encoding
             * LIMITATION: This implementation only handles 1-byte (ASCII) and
             * 2-byte UTF-8 sequences (U+0080 to U+07FF). Characters outside
             * this range (e.g., Euro sign € which is U+20AC, a 3-byte sequence)
             * will cause EILSEQ errors. This is a reasonable limitation for
             * a barebones implementation targeting ISO-8859-1.
             */
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
                } else if ((c & 0xE0) == 0xC0) {
                    /* 2-byte UTF-8 sequence (U+0080 to U+07FF) */
                    if (inleft < 2) {
                        errno = EINVAL;
                        return (size_t)-1;
                    }
                    unsigned char c2 = (unsigned char)inptr[1];
                    if ((c2 & 0xC0) != 0x80) {
                        errno = EILSEQ;
                        return (size_t)-1;
                    }
                    unsigned char latin1 = ((c & 0x1F) << 6) | (c2 & 0x3F);
                    if (latin1 < 0x80 || latin1 > 0xFF) {
                        errno = EILSEQ;
                        return (size_t)-1;
                    }
                    *outptr = (char)latin1;
                    inptr += 2;
                    outptr++;
                    inleft -= 2;
                    outleft--;
                    converted++;
                } else {
                    /* Invalid UTF-8 sequence for Latin-1 (3+ byte sequences) */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
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
             * Locale charset to UTF-8 using locale.library
             * This implementation uses locale.library for character validation
             * and case conversion, then converts to UTF-8.
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
                
                if (c < 0x80) {
                    /* ASCII character - direct copy */
                    *outptr = c;
                    inptr++;
                    outptr++;
                    inleft--;
                    outleft--;
                    converted++;
                } else {
                    /* Locale character (0x80-0xFF) - convert to 2-byte UTF-8 */
                    if (outleft < 2) {
                        errno = E2BIG;
                        break;  /* Convert as much as possible before stopping */
                    }
                    *outptr = (char)(0xC0 | (c >> 6));
                    outptr++;
                    *outptr = (char)(0x80 | (c & 0x3F));
                    outptr++;
                    inptr++;
                    inleft--;
                    outleft -= 2;
                    converted++;
                }
            }
            break;
            
        case CONV_UTF8_TO_LOCALE:
            /* 
             * UTF-8 to locale charset using locale.library
             * LIMITATION: This implementation only handles 1-byte (ASCII) and
             * 2-byte UTF-8 sequences (U+0080 to U+07FF). Characters outside
             * this range will cause EILSEQ errors. This is a reasonable
             * limitation for a barebones implementation.
             */
            while (inleft > 0 && outleft > 0) {
                unsigned char c = (unsigned char)*inptr;
                if (c < 0x80) {
                    /* ASCII character - validate with locale and copy */
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
                } else if ((c & 0xE0) == 0xC0) {
                    /* 2-byte UTF-8 sequence (U+0080 to U+07FF) */
                    if (inleft < 2) {
                        errno = EINVAL;
                        return (size_t)-1;
                    }
                    unsigned char c2 = (unsigned char)inptr[1];
                    if ((c2 & 0xC0) != 0x80) {
                        errno = EILSEQ;
                        return (size_t)-1;
                    }
                    unsigned char latin1 = ((c & 0x1F) << 6) | (c2 & 0x3F);
                    
                    /* Validate character with locale.library */
                    if (desc->locale && !is_valid_locale_char(desc->locale, latin1)) {
                        errno = EILSEQ;
                        return (size_t)-1;
                    }
                    
                    *outptr = (char)latin1;
                    inptr += 2;
                    outptr++;
                    inleft -= 2;
                    outleft--;
                    converted++;
                } else {
                    /* Invalid UTF-8 sequence for locale charset (3+ byte sequences) */
                    errno = EILSEQ;
                    return (size_t)-1;
                }
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
