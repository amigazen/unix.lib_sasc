/*
 * getopt.c - POSIX getopt() function for command line option parsing
 *
 * This function parses command line options in the standard Unix format.
 * It supports both short options (-a, -b) and long options (--long).
 *
 * POSIX.1-2001, POSIX.1-2008, SVr4, 4.3BSD
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/* Global variables for getopt */
int opterr = 1;        /* Error reporting flag */
int optind = 1;        /* Index of next argument to process */
int optopt;            /* Last option character */
char *optarg;          /* Argument for option */

/* Error reporting macro */
#define ERR(s, c) \
    if (opterr) { \
        fprintf(stderr, "%s%s%c\n", argv[0], s, c); \
    }

int getopt(int argc, char * const argv[], const char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;
    
    /* Check if we need to move to next argument */
    if (sp == 1) {
        if (optind >= argc || 
            argv[optind][0] != '-' || 
            argv[optind][1] == '\0') {
            return EOF;
        }
        
        /* Handle -- to end options */
        if (strcmp(argv[optind], "--") == 0) {
            optind++;
            return EOF;
        }
    }
    
    /* Get current option character */
    optopt = c = argv[optind][sp];
    
    /* Check if option is valid */
    if (c == ':' || (cp = strchr(opts, c)) == NULL) {
        ERR(": illegal option -- ", c);
        if (argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return '?';
    }
    
    /* Check if option requires an argument */
    if (*++cp == ':') {
        if (argv[optind][sp + 1] != '\0') {
            optarg = &argv[optind++][sp + 1];
        } else if (++optind >= argc) {
            ERR(": option requires an argument -- ", c);
            sp = 1;
            return '?';
        } else {
            optarg = argv[optind++];
        }
        sp = 1;
    } else {
        if (argv[optind][++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    
    return c;
}
