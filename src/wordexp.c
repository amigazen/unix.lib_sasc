/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * wordexp.c - Word expansion functions (POSIX compliant)
 *
 * This file implements word expansion functions as specified
 * in POSIX.1-2001 and POSIX.1-2008.
 *
 * Copyright (C) 2025 by amigazen project
 */

#include "amiga.h"
#include <wordexp.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/* Internal function prototypes */
static int parse_word(const char **input, char **output, int flags);
static int expand_variable(const char **input, char **output, int flags);
static int expand_tilde(const char **input, char **output, int flags);
static int expand_glob(const char **input, char **output, int flags);
static int add_word(wordexp_t *pwordexp, const char *word);
static void free_words(wordexp_t *pwordexp);

/*
 * wordexp - perform word expansion
 *
 * The wordexp() function performs word expansion on the string words,
 * storing the result in the structure pointed to by pwordexp.
 *
 * POSIX.1-2001, POSIX.1-2008, XSI
 */
int wordexp(const char *words, wordexp_t *pwordexp, int flags)
{
    const char *input;
    char *output;
    int result;
    
    /* Check for abort signal */
    __chkabort();
    
    /* Validate parameters */
    if (words == NULL || pwordexp == NULL) {
        return WRDE_SYNTAX;
    }
    
    /* Initialize result structure */
    if (!(flags & WRDE_REUSE)) {
        pwordexp->we_wordc = 0;
        pwordexp->we_wordv = NULL;
        pwordexp->we_offs = 0;
    }
    
    /* Set offset if WRDE_DOOFFS is specified */
    if (flags & WRDE_DOOFFS) {
        pwordexp->we_offs = 0; /* Simplified - always use 0 */
    }
    
    /* Allocate output buffer */
    output = malloc(strlen(words) + 1);
    if (output == NULL) {
        return WRDE_NOSPACE;
    }
    
    input = words;
    result = 0;
    
    /* Skip leading whitespace */
    while (isspace((unsigned char)*input)) {
        input++;
    }
    
    /* Parse words */
    while (*input != '\0' && result == 0) {
        char *word = output;
        
        /* Parse one word */
        result = parse_word(&input, &word, flags);
        if (result != 0) {
            break;
        }
        
        /* Add word to result if not empty */
        if (word > output) {
            *word = '\0';
            if (strlen(output) > 0) {
                result = add_word(pwordexp, output);
                if (result != 0) {
                    break;
                }
            }
        }
        
        /* Skip whitespace between words */
        while (isspace((unsigned char)*input)) {
            input++;
        }
    }
    
    free(output);
    
    /* Handle errors */
    if (result != 0) {
        if (!(flags & WRDE_REUSE)) {
            wordfree(pwordexp);
        }
        return result;
    }
    
    return 0;
}

/*
 * wordfree - free word expansion result
 *
 * The wordfree() function frees the memory allocated by wordexp().
 *
 * POSIX.1-2001, POSIX.1-2008, XSI
 */
void wordfree(wordexp_t *pwordexp)
{
    /* Check for abort signal */
    __chkabort();
    
    if (pwordexp == NULL) {
        return;
    }
    
    free_words(pwordexp);
    
    pwordexp->we_wordc = 0;
    pwordexp->we_wordv = NULL;
    pwordexp->we_offs = 0;
}

/*
 * parse_word - parse a single word from input
 *
 * This function parses one word from the input string, handling
 * quotes, variable expansion, tilde expansion, and globbing.
 */
static int parse_word(const char **input, char **output, int flags)
{
    const char *in = *input;
    char *out = *output;
    int in_quotes = 0;
    int quote_char = 0;
    
    while (*in != '\0' && (!isspace((unsigned char)*in) || in_quotes)) {
        if (!in_quotes) {
            /* Check for quote start */
            if (*in == '"' || *in == '\'') {
                in_quotes = 1;
                quote_char = *in;
                in++;
                continue;
            }
            
            /* Check for variable expansion */
            if (*in == '$' && (in[1] == '{' || isalnum((unsigned char)in[1]))) {
                int result = expand_variable(&in, &out, flags);
                if (result != 0) {
                    return result;
                }
                continue;
            }
            
            /* Check for tilde expansion */
            if (*in == '~' && (in == *input || isspace((unsigned char)in[-1]))) {
                int result = expand_tilde(&in, &out, flags);
                if (result != 0) {
                    return result;
                }
                continue;
            }
            
            /* Check for globbing */
            if (*in == '*' || *in == '?' || *in == '[') {
                int result = expand_glob(&in, &out, flags);
                if (result != 0) {
                    return result;
                }
                continue;
            }
        } else {
            /* Inside quotes */
            if (*in == quote_char) {
                in_quotes = 0;
                in++;
                continue;
            }
        }
        
        /* Regular character */
        *out++ = *in++;
    }
    
    *input = in;
    *output = out;
    
    return 0;
}

/*
 * expand_variable - expand shell variable
 *
 * This function expands shell variables like $VAR or ${VAR}.
 */
static int expand_variable(const char **input, char **output, int flags)
{
    const char *in = *input;
    char *out = *output;
    char var_name[256];
    char *var_value;
    int i = 0;
    
    /* Skip $ */
    in++;
    
    /* Check for ${VAR} syntax */
    if (*in == '{') {
        in++;
        while (*in != '}' && *in != '\0' && i < 255) {
            var_name[i++] = *in++;
        }
        if (*in == '}') {
            in++;
        } else {
            return WRDE_SYNTAX;
        }
    } else {
        /* $VAR syntax */
        while (isalnum((unsigned char)*in) && i < 255) {
            var_name[i++] = *in++;
        }
    }
    
    var_name[i] = '\0';
    
    /* Get environment variable */
    var_value = getenv(var_name);
    if (var_value == NULL) {
        if (flags & WRDE_UNDEF) {
            return WRDE_BADVAL;
        }
        /* Leave variable as-is if not defined and WRDE_UNDEF not set */
        *out++ = '$';
        if (i > 0 && var_name[0] == '{') {
            *out++ = '{';
        }
        strcpy(out, var_name);
        out += strlen(var_name);
        if (i > 0 && var_name[0] == '{') {
            *out++ = '}';
        }
    } else {
        /* Copy expanded value */
        strcpy(out, var_value);
        out += strlen(var_value);
    }
    
    *input = in;
    *output = out;
    
    return 0;
}

/*
 * expand_tilde - expand tilde to home directory
 *
 * This function expands ~ to the home directory.
 */
static int expand_tilde(const char **input, char **output, int flags)
{
    const char *in = *input;
    char *out = *output;
    char *home_dir;
    
    /* Skip ~ */
    in++;
    
    /* Get home directory */
    home_dir = getenv("HOME");
    if (home_dir == NULL) {
        /* If no HOME, leave ~ as-is */
        *out++ = '~';
    } else {
        /* Copy home directory */
        strcpy(out, home_dir);
        out += strlen(home_dir);
    }
    
    *input = in;
    *output = out;
    
    return 0;
}

/*
 * expand_glob - expand glob patterns
 *
 * This function expands glob patterns like *, ?, [abc].
 * For simplicity, this implementation just copies the pattern as-is.
 */
static int expand_glob(const char **input, char **output, int flags)
{
    const char *in = *input;
    char *out = *output;
    
    /* For now, just copy the glob pattern as-is */
    /* A full implementation would need to match against filesystem */
    while (*in != '\0' && !isspace((unsigned char)*in)) {
        *out++ = *in++;
    }
    
    *input = in;
    *output = out;
    
    return 0;
}

/*
 * add_word - add a word to the wordexp result
 *
 * This function adds a word to the word expansion result.
 */
static int add_word(wordexp_t *pwordexp, const char *word)
{
    char **new_wordv;
    char *word_copy;
    size_t new_size;
    
    /* Allocate space for new word */
    word_copy = malloc(strlen(word) + 1);
    if (word_copy == NULL) {
        return WRDE_NOSPACE;
    }
    strcpy(word_copy, word);
    
    /* Reallocate word array */
    new_size = (pwordexp->we_wordc + 1) * sizeof(char *);
    new_wordv = realloc(pwordexp->we_wordv, new_size);
    if (new_wordv == NULL) {
        free(word_copy);
        return WRDE_NOSPACE;
    }
    
    pwordexp->we_wordv = new_wordv;
    pwordexp->we_wordv[pwordexp->we_wordc] = word_copy;
    pwordexp->we_wordc++;
    
    return 0;
}

/*
 * free_words - free all words in wordexp result
 *
 * This function frees all words in the word expansion result.
 */
static void free_words(wordexp_t *pwordexp)
{
    size_t i;
    
    if (pwordexp->we_wordv == NULL) {
        return;
    }
    
    for (i = 0; i < pwordexp->we_wordc; i++) {
        free(pwordexp->we_wordv[i]);
    }
    
    free(pwordexp->we_wordv);
    pwordexp->we_wordv = NULL;
}


