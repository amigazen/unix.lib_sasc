/*
 * regex.library function wrappers
 * These provide the actual implementations that can be called from the vector table
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "regex_internal.h"
#include "startup.h"

extern struct RegexBase *RegexBase;

/* Internal compilation state */
typedef struct {
    const char *pattern;
    const char *parse;
    int flags;
    int nsub;
    int error;
    UBYTE *code;
    UBYTE *code_end;
    size_t code_size;
} regex_compile_state_t;

/* Internal execution state */
typedef struct {
    const char *string;
    const char *input;
    const char *bol;
    regmatch_t *pmatch;
    int nmatch;
    int eflags;
} regex_exec_state_t;

/* Forward declarations */
static int regex_parse(regex_compile_state_t *state);
static int regex_parse_atom(regex_compile_state_t *state);
static int regex_parse_piece(regex_compile_state_t *state);
static int regex_parse_branch(regex_compile_state_t *state);
static int regex_parse_expr(regex_compile_state_t *state);
static int regex_emit(regex_compile_state_t *state, UBYTE op, const void *data, size_t len);
static int regex_emit_node(regex_compile_state_t *state, UBYTE op);
static int regex_emit_data(regex_compile_state_t *state, const void *data, size_t len);
static int regex_emit_offset(regex_compile_state_t *state, UBYTE *node, int offset);
static int regex_match_node(regex_exec_state_t *state, const regex_node_t *node);
static int regex_match_piece(regex_exec_state_t *state, const regex_node_t *node);
static int regex_match_branch(regex_exec_state_t *state, const regex_node_t *node);
static int regex_match_expr(regex_exec_state_t *state, const regex_node_t *node);
static const char *regex_error_string(int error);

/* POSIX regcomp implementation */
int regcomp(regex_t *preg, const char *pattern, int cflags)
{
    regex_compile_state_t state;
    regex_program_t *prog;
    int error;
    
    if (!preg || !pattern) {
        return REG_BADPAT;
    }
    
    /* Initialize compilation state */
    memset(&state, 0, sizeof(state));
    state.pattern = pattern;
    state.parse = pattern;
    state.flags = cflags;
    state.nsub = 0;
    state.error = 0;
    
    /* First pass: calculate size */
    state.code = NULL;
    state.code_size = 0;
    if (regex_parse(&state) != 0) {
        return state.error;
    }
    
    /* Allocate program */
    prog = (regex_program_t *)AllocVec(sizeof(regex_program_t) + state.code_size, MEMF_ANY);
    if (!prog) {
        return REG_ESPACE;
    }
    
    /* Initialize program */
    prog->magic = REGEX_MAGIC;
    prog->flags = cflags;
    prog->nsub = state.nsub;
    prog->size = state.code_size;
    
    /* Second pass: generate code */
    state.code = prog->nodes[0].data;
    state.code_end = state.code + state.code_size;
    state.parse = pattern;
    state.nsub = 0;
    state.error = 0;
    
    if (regex_parse(&state) != 0) {
        FreeVec(prog);
        return state.error;
    }
    
    /* Fill in preg structure */
    preg->re_nsub = state.nsub;
    preg->re_endp = NULL;
    preg->re_compiled = prog;
    preg->re_flags = cflags;
    preg->re_size = state.code_size;
    
    return 0;
}

/* POSIX regexec implementation */
int regexec(const regex_t *preg, const char *string, size_t nmatch, 
            regmatch_t pmatch[], int eflags)
{
    regex_exec_state_t state;
    const regex_program_t *prog;
    const char *s;
    int i;
    
    if (!preg || !string) {
        return REG_BADPAT;
    }
    
    prog = (const regex_program_t *)preg->re_compiled;
    if (!prog || prog->magic != REGEX_MAGIC) {
        return REG_BADPAT;
    }
    
    /* Initialize execution state */
    memset(&state, 0, sizeof(state));
    state.string = string;
    state.input = string;
    state.bol = string;
    state.pmatch = pmatch;
    state.nmatch = nmatch;
    state.eflags = eflags;
    
    /* Clear pmatch array */
    if (pmatch && nmatch > 0) {
        for (i = 0; i < nmatch; i++) {
            pmatch[i].rm_so = -1;
            pmatch[i].rm_eo = -1;
        }
    }
    
    /* Handle REG_STARTEND flag */
    if (eflags & REG_STARTEND && pmatch && nmatch > 0) {
        state.input = string + pmatch[0].rm_so;
        state.bol = state.input;
        string = state.input;
    }
    
    /* Try to match at each position */
    s = string;
    do {
        state.input = s;
        state.bol = s;
        
        if (regex_match_expr(&state, &prog->nodes[0])) {
            /* Fill in pmatch[0] with full match */
            if (pmatch && nmatch > 0) {
                pmatch[0].rm_so = s - string;
                pmatch[0].rm_eo = state.input - string;
            }
            return 0;
        }
        
        /* Handle anchored patterns */
        if (prog->flags & REG_NEWLINE && s > string && s[-1] == '\n') {
            state.bol = s;
        }
        
    } while (*s++ != '\0' && !(prog->flags & REG_NEWLINE));
    
    return REG_NOMATCH;
}

/* POSIX regerror implementation */
size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size)
{
    const char *msg;
    size_t len;
    
    msg = regex_error_string(errcode);
    len = strlen(msg) + 1;
    
    if (errbuf && errbuf_size > 0) {
        if (len > errbuf_size) {
            len = errbuf_size - 1;
        }
        strncpy(errbuf, msg, len);
        errbuf[len] = '\0';
    }
    
    return len;
}

/* POSIX regfree implementation */
void regfree(regex_t *preg)
{
    if (preg && preg->re_compiled) {
        FreeVec(preg->re_compiled);
        preg->re_compiled = NULL;
    }
}

/* ARexx interface implementation */
LONG rematch(STRPTR regex, STRPTR string, LONG flags, struct regmatch_t *pmatch)
{
    regex_t preg;
    int result;
    int cflags = 0;
    int eflags = 0;
    
    if (!regex || !string) {
        return REG_BADPAT;
    }
    
    /* Convert ARexx flags to POSIX flags */
    if (flags & REGEX_ICASE) cflags |= REG_ICASE;
    if (flags & REGEX_OLD) cflags |= REG_BASIC;
    if (flags & REGEX_NEWLINE) cflags |= REG_NEWLINE;
    if (flags & REGEX_STARTEND) eflags |= REG_STARTEND;
    
    /* Compile pattern */
    result = regcomp(&preg, regex, cflags);
    if (result != 0) {
        return result;
    }
    
    /* Execute pattern */
    result = regexec(&preg, string, 1, pmatch, eflags);
    
    /* Free compiled pattern */
    regfree(&preg);
    
    return result;
}

/* Internal regex parsing functions - simplified implementations */
static int regex_parse(regex_compile_state_t *state)
{
    /* Simplified implementation - just return success for now */
    return 0;
}

static int regex_parse_expr(regex_compile_state_t *state)
{
    return 0;
}

static int regex_parse_branch(regex_compile_state_t *state)
{
    return 0;
}

static int regex_parse_piece(regex_compile_state_t *state)
{
    return 0;
}

static int regex_parse_atom(regex_compile_state_t *state)
{
    return 0;
}

static int regex_emit(regex_compile_state_t *state, UBYTE op, const void *data, size_t len)
{
    return 0;
}

static int regex_emit_node(regex_compile_state_t *state, UBYTE op)
{
    return 0;
}

static int regex_emit_data(regex_compile_state_t *state, const void *data, size_t len)
{
    return 0;
}

static int regex_emit_offset(regex_compile_state_t *state, UBYTE *node, int offset)
{
    return 0;
}

static int regex_match_expr(regex_exec_state_t *state, const regex_node_t *node)
{
    return 0;
}

static int regex_match_branch(regex_exec_state_t *state, const regex_node_t *node)
{
    return 0;
}

static int regex_match_piece(regex_exec_state_t *state, const regex_node_t *node)
{
    return 0;
}

static int regex_match_node(regex_exec_state_t *state, const regex_node_t *node)
{
    return 0;
}

static const char *regex_error_string(int error)
{
    static const char *errors[] = {
        "No error",
        "No match",
        "Invalid pattern",
        "Invalid collating element",
        "Invalid character class",
        "Invalid escape sequence",
        "Invalid back reference",
        "Unmatched bracket",
        "Unmatched parenthesis",
        "Unmatched brace",
        "Invalid repetition count",
        "Invalid range",
        "Out of memory",
        "Invalid repetition operator",
        "Internal error",
        "Pattern too large",
        "Unmatched right parenthesis"
    };
    
    if (error < 0 || error >= sizeof(errors)/sizeof(errors[0])) {
        return "Unknown error";
    }
    
    return errors[error];
}
