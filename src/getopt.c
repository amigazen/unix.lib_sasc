/*
 * getopt() - Return the next user option on each iteration
 *
 * Original Author: Daniel J. Barrett
 * Copyright (C) 2025 by amigazen project
 */

#include <stdio.h>
#include <string.h>

/* Global variables for getopt() */
char *optarg = NULL;    /* Pointer to argument of current option */
int optind = 1;         /* Index of next element to be processed in argv */
int optopt = 0;         /* Option character that was returned */
int opterr = 1;         /* If error messages should be printed */

/* Internal variables */
static int nextchar = 0;        /* Index of next character to process in argv[optind] */
static int initialized = 0;      /* Flag to indicate if getopt() has been initialized */

/* Constants */
#define DASH            '-'     /* This precedes an option */
#define ARG_COMING     ':'     /* In the option string, this indicates that
                                  the option requires an argument */
#define UNKNOWN_OPT    '?'     /* The char returned for unknown option */

/* Internal error codes */
#define ERROR_BAD_OPTION        1
#define ERROR_MISSING_ARGUMENT  2

/* Function prototypes */
static int NextOption(char *argv[], char *optString, int argc);
static int RealOption(char *argv[], char *str, int *skip, int *ind, int opt, int argc);
static int HandleArgument(char *argv[], int *optind, int *skip);
static void Error(int err, int c, char *argv[]);

/*
 * getopt() - Main function to parse command line options
 * Returns: option character, '?' for unknown option, EOF when done
 */
int getopt(int argc, char *argv[], char *optString)
{
    int c;
    int skip;
    
    /* Initialize on first call */
    if (!initialized) {
        optind = 1;
        nextchar = 0;
        initialized = 1;
    }
    
    /* Check if we've processed all arguments */
    if (optind >= argc) {
        return EOF;
    }
    
    /* Check if current argument starts with a dash */
    if (argv[optind][0] != DASH) {
        return EOF;
    }
    
    /* Handle single dash '-' - Berkeley compatibility */
    if (argv[optind][1] == '\0') {
        return EOF;
    }
    
    /* Handle "--" end of options marker */
    if (argv[optind][1] == DASH && argv[optind][2] == '\0') {
        optind++;
        return EOF;
    }
    
    /* Get next option */
    c = NextOption(argv, optString, argc);
    
    /* Handle argument if needed */
    if (c != EOF && c != UNKNOWN_OPT) {
        skip = 0;
        if (HandleArgument(argv, &optind, &skip) != 0) {
            c = UNKNOWN_OPT;
        }
    }
    
    optopt = c;
    return c;
}

/*
 * NextOption() - Find the next option in the argument list
 */
static int NextOption(char *argv[], char *optString, int argc)
{
    int c;
    int skip;
    char *str;
    
    /* Get current option character */
    c = argv[optind][nextchar];
    if (c == '\0') {
        return EOF;
    }
    
    /* Skip the initial dash for first character */
    if (nextchar == 0) {
        nextchar = 1;
        c = argv[optind][nextchar];
        if (c == '\0') {
            return EOF;
        }
    }
    
    /* Find option in option string */
    str = strchr(optString, c);
    if (!str) {
        Error(ERROR_BAD_OPTION, c, argv);
        nextchar++;
        if (argv[optind][nextchar] == '\0') {
            optind++;
            nextchar = 0;
        }
        return UNKNOWN_OPT;
    }
    
    /* Check if option requires argument */
    if (str[1] == ARG_COMING) {
        skip = 1;
        if (RealOption(argv, str, &skip, &optind, c, argc) != 0) {
            return UNKNOWN_OPT;
        }
    } else {
        skip = 0;
        if (RealOption(argv, str, &skip, &optind, c, argc) != 0) {
            return UNKNOWN_OPT;
        }
    }
    
    return c;
}

/*
 * RealOption() - Process a real option (not a dash)
 */
static int RealOption(char *argv[], char *str, int *skip, int *ind, int opt, int argc)
{
    /* Check if option requires argument */
    if (str[1] == ARG_COMING) {
        if (*skip) {
            /* Argument is in next argv element */
            optarg = argv[*ind + 1];
            if (!optarg) {
                Error(ERROR_MISSING_ARGUMENT, opt, argv);
                return -1;
            }
            (*ind)++;
        } else {
            /* Argument is in same argv element */
            optarg = &argv[*ind][nextchar + 1];
            if (*optarg == '\0') {
                /* No argument in same element, try next element */
                if (*ind + 1 >= argc) {
                    Error(ERROR_MISSING_ARGUMENT, opt, argv);
                    return -1;
                }
                optarg = argv[*ind + 1];
                (*ind)++;
            }
        }
        nextchar = 0;
        (*ind)++;
    } else {
        /* No argument required */
        optarg = NULL;
        nextchar++;
        if (argv[*ind][nextchar] == '\0') {
            (*ind)++;
            nextchar = 0;
        }
    }
    
    return 0;
}

/*
 * HandleArgument() - Handle argument processing
 */
static int HandleArgument(char *argv[], int *optind, int *skip)
{
    /* This function handles the argument processing logic */
    /* For now, we'll keep it simple */
    return 0;
}

/*
 * Error() - Print error messages if opterr is set
 */
static void Error(int err, int c, char *argv[])
{
    char *p;
    
    if (opterr) {
        switch (err) {
            case ERROR_BAD_OPTION:
                /* Berkeley-style error message */
                if (!(p = strrchr(argv[0], '/')))
                    p = argv[0];
                else
                    ++p;
                fprintf(stderr, "%s: illegal option -- %c\n", p, c);
                break;
            case ERROR_MISSING_ARGUMENT:
                /* Berkeley-style error message */
                if (!(p = strrchr(argv[0], '/')))
                    p = argv[0];
                else
                    ++p;
                fprintf(stderr, "%s: option requires an argument -- %c\n", p, c);
                break;
            default:
                fprintf(stderr, "%c: Unknown error.\n", c);
                break;
        }
    }
}
