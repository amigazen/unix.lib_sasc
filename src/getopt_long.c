/*
 * getopt_long.c - parse long command line options (POSIX compliant)
 *
 * This function parses command line options, including long options
 * that start with two dashes. It's an extension of getopt().
 *
 * POSIX.1-2001, POSIX.1-2008, 4.3BSD
 */

#include "amiga.h"
#include <getopt.h>
#include <string.h>
#include <stdio.h>

int getopt_long(int argc, char * const argv[], const char *optstring,
                const struct option *longopts, int *longindex)
{
    static int longopt_index = 0;
    static int longopt_offset = 0;
    int i, match;
    
    chkabort();
    
    if (optind >= argc) {
        return -1;
    }
    
    /* Check if this is a long option (starts with --) */
    if (argv[optind][0] == '-' && argv[optind][1] == '-' && argv[optind][2] != '\0') {
        char *arg = argv[optind] + 2;
        char *equal_sign = strchr(arg, '=');
        int arg_len;
        
        if (equal_sign) {
            arg_len = equal_sign - arg;
        } else {
            arg_len = strlen(arg);
        }
        
        /* Find matching long option */
        for (i = 0; longopts[i].name != NULL; i++) {
            if (strncmp(longopts[i].name, arg, arg_len) == 0) {
                match = i;
                
                /* Check for exact match */
                if (strlen(longopts[i].name) == arg_len) {
                    if (longindex) {
                        *longindex = match;
                    }
                    
                    /* Handle option with argument */
                    if (longopts[match].has_arg == required_argument) {
                        if (equal_sign) {
                            optarg = equal_sign + 1;
                        } else if (optind + 1 < argc) {
                            optarg = argv[++optind];
                        } else {
                            if (opterr) {
                                fprintf(stderr, "%s: option '--%s' requires an argument\n",
                                        argv[0], longopts[match].name);
                            }
                            return '?';
                        }
                    } else if (longopts[match].has_arg == optional_argument) {
                        if (equal_sign) {
                            optarg = equal_sign + 1;
                        } else {
                            optarg = NULL;
                        }
                    }
                    
                    optind++;
                    return longopts[match].val;
                }
            }
        }
        
        /* No exact match found */
        if (opterr) {
            fprintf(stderr, "%s: unrecognized option '--%s'\n", argv[0], arg);
        }
        optind++;
        return '?';
    }
    
    /* Fall back to regular getopt for short options */
    return getopt(argc, argv, optstring);
}
