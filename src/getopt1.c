/*
 * getopt1.c - Extended getopt functions for long option support
 *
 * Original Author: Daniel J. Barrett
 * Copyright (C) 2025 by amigazen project
 */

#include "getopt.h"
#include <stdio.h>
#include <string.h>

/* Function prototypes for internal functions */
static int handle_long_option(int argc, char *const *argv, const char *shortopts,
                             const struct option *longopts, int *longind, int long_only);
static int handle_short_option(int argc, char *const *argv, const char *shortopts);

/* Internal variables for long option processing */
static int nextchar = 0;  /* Index of next character to process in argv[optind] */

/*
 * _getopt_internal - Internal function for handling both short and long options
 * 
 * This function provides the core logic for getopt_long and getopt_long_only.
 * It first tries to match long options, then falls back to short options
 * if long option matching fails.
 */
int _getopt_internal(int argc, char *const *argv, const char *shortopts,
                     const struct option *longopts, int *longind, int long_only)
{
    static int initialized = 0;
    
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
    if (argv[optind][0] != '-') {
        return EOF;
    }
    
    /* Handle single dash '-' - Berkeley compatibility */
    if (argv[optind][1] == '\0') {
        return EOF;
    }
    
    /* Handle "--" end of options marker */
    if (argv[optind][1] == '-' && argv[optind][2] == '\0') {
        optind++;
        return EOF;
    }
    
    /* Try to handle as long option if it starts with "--" */
    if (argv[optind][1] == '-') {
        return handle_long_option(argc, argv, shortopts, longopts, longind, long_only);
    }
    
    /* Handle as short option */
    return handle_short_option(argc, argv, shortopts);
}

/*
 * handle_long_option - Process long options (--option)
 */
static int handle_long_option(int argc, char *const *argv, const char *shortopts,
                             const struct option *longopts, int *longind, int long_only)
{
    char *option_name = &argv[optind][2]; /* Skip "--" */
    char *equals_sign = strchr(option_name, '=');
    int name_len;
    const struct option *opt;
    
    if (equals_sign) {
        name_len = equals_sign - option_name;
    } else {
        name_len = strlen(option_name);
    }
    
    /* Find matching long option */
    for (opt = longopts; opt->name != NULL; opt++) {
        if (strncmp(opt->name, option_name, name_len) == 0 && 
            opt->name[name_len] == '\0') {
            
            /* Found matching option */
            if (longind) {
                *longind = opt - longopts;
            }
            
            /* Handle argument */
            if (opt->has_arg == required_argument) {
                if (equals_sign) {
                    optarg = equals_sign + 1;
                } else if (optind + 1 < argc) {
                    optarg = argv[optind + 1];
                    optind++;
                } else {
                    /* Missing required argument */
                    if (opterr) {
                        fprintf(stderr, "%s: option '--%s' requires an argument\n", 
                                argv[0], opt->name);
                    }
                    optopt = 0;
                    return '?';
                }
            } else if (opt->has_arg == optional_argument) {
                if (equals_sign) {
                    optarg = equals_sign + 1;
                } else {
                    optarg = NULL;
                }
            } else {
                optarg = NULL;
            }
            
            /* Handle flag or return value */
            if (opt->flag) {
                *opt->flag = opt->val;
                optind++;
                return 0;
            } else {
                optind++;
                return opt->val;
            }
        }
    }
    
    /* No matching long option found */
    if (opterr) {
        fprintf(stderr, "%s: unrecognized option '--%s'\n", argv[0], option_name);
    }
    optopt = 0;
    optind++;
    return '?';
}

/*
 * handle_short_option - Process short options (-o)
 * Enhanced to match the improved getopt.c logic
 */
static int handle_short_option(int argc, char *const *argv, const char *shortopts)
{
    int c;
    char *str;
    char *p;
    
    /* Skip the initial dash for first character */
    if (nextchar == 0) {
        nextchar = 1;
    }
    
    c = argv[optind][nextchar];
    if (c == '\0') {
        optind++;
        nextchar = 0;
        return EOF;
    }
    
    /* Find option in option string */
    str = strchr(shortopts, c);
    if (!str) {
        if (opterr) {
            /* Berkeley-style error message */
            if (!(p = strrchr(argv[0], '/')))
                p = argv[0];
            else
                ++p;
            fprintf(stderr, "%s: illegal option -- %c\n", p, c);
        }
        optopt = c;
        nextchar++;
        if (argv[optind][nextchar] == '\0') {
            optind++;
            nextchar = 0;
        }
        return '?';
    }
    
    /* Check if option requires argument */
    if (str[1] == ':') {
        if (argv[optind][nextchar + 1] != '\0') {
            /* Argument is in same argv element */
            optarg = &argv[optind][nextchar + 1];
        } else if (optind + 1 < argc) {
            /* Argument is in next argv element */
            optarg = argv[optind + 1];
            optind++;
        } else {
            /* Missing required argument */
            if (opterr) {
                /* Berkeley-style error message */
                if (!(p = strrchr(argv[0], '/')))
                    p = argv[0];
                else
                    ++p;
                fprintf(stderr, "%s: option requires an argument -- %c\n", p, c);
            }
            optopt = c;
            return '?';
        }
        nextchar = 0;
        optind++;
    } else {
        /* No argument required */
        optarg = NULL;
        nextchar++;
        if (argv[optind][nextchar] == '\0') {
            optind++;
            nextchar = 0;
        }
    }
    
    optopt = c;
    return c;
}

/*
 * getopt_long - Parse long options in addition to short options
 * 
 * This function works like getopt() but also accepts long options
 * starting with "--". Long options can be abbreviated as long as
 * the abbreviation is unique.
 */
int getopt_long(int argc, char *const *argv, const char *options,
                const struct option *long_options, int *opt_index)
{
    return _getopt_internal(argc, argv, options, long_options, opt_index, 0);
}

/*
 * getopt_long_only - Parse long options only
 * 
 * This function is like getopt_long(), but '-' as well as '--' can
 * indicate a long option. If an option that starts with '-' (not '--')
 * doesn't match a long option, but does match a short option, it is
 * parsed as a short option instead.
 */
int getopt_long_only(int argc, char *const *argv, const char *options,
                     const struct option *long_options, int *opt_index)
{
    return _getopt_internal(argc, argv, options, long_options, opt_index, 1);
}

#ifdef TEST
/*
 * Test program for extended getopt functionality
 */
int main(int argc, char **argv)
{
    int c;
    int digit_optind = 0;
    
    static struct option long_options[] = {
        {"add", required_argument, 0, 0},
        {"append", no_argument, 0, 0},
        {"delete", required_argument, 0, 0},
        {"verbose", no_argument, 0, 0},
        {"create", no_argument, 0, 0},
        {"file", required_argument, 0, 0},
        {0, 0, 0, 0}
    };
    
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        
        c = getopt_long(argc, argv, "abc:d:0123456789",
                        long_options, &option_index);
        if (c == EOF) {
            break;
        }
        
        switch (c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg) {
                    printf(" with arg %s", optarg);
                }
                printf("\n");
                break;
                
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (digit_optind != 0 && digit_optind != this_option_optind) {
                    printf("digits occur in two different argv-elements.\n");
                }
                digit_optind = this_option_optind;
                printf("option %c\n", c);
                break;
                
            case 'a':
                printf("option a\n");
                break;
                
            case 'b':
                printf("option b\n");
                break;
                
            case 'c':
                printf("option c with value `%s'\n", optarg);
                break;
                
            case 'd':
                printf("option d with value `%s'\n", optarg);
                break;
                
            case '?':
                break;
                
            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }
    
    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }
    
    return 0;
}
#endif /* TEST */
