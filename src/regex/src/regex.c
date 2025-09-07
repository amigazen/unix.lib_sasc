/*
 * POSIX-compliant regex.library implementation
 * Based on Henry Spencer's regex engine with POSIX API
 */

#define __NOLIBBASE__

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <ctype.h>
#include <clib/compiler-specific.h>
#include <internal/regex_internal.h>
#include "startup.h"
#include "lvo_clib.h"

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

/* POSIX regcomp implementation */
int __ASM__ regcomp(__REG__(a6, struct RegexBase *rb), __REG__(a0, void *preg), __REG__(a1, const char *pattern), __REG__(d0, int cflags))
{
    regex_compile_state_t state;
    regex_program_t *prog;
    regex_t *preg_t = (regex_t *)preg;
    
    if (!preg_t || !pattern) {
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
    preg_t->re_nsub = state.nsub;
    preg_t->re_endp = NULL;
    preg_t->re_compiled = prog;
    preg_t->re_flags = cflags;
    preg_t->re_size = state.code_size;
    
    return 0;
}

/* POSIX regexec implementation */
int __ASM__ regexec(__REG__(a6, struct RegexBase *rb), __REG__(a0, const void *preg), __REG__(a1, const char *string), 
                   __REG__(d0, ULONG nmatch), __REG__(d1, void *pmatch), __REG__(d2, int eflags))
{
    regex_exec_state_t state;
    const regex_program_t *prog;
    const char *s;
    int i;
    const regex_t *preg_t = (const regex_t *)preg;
    regmatch_t *pmatch_t = (regmatch_t *)pmatch;
    
    if (!preg_t || !string) {
        return REG_BADPAT;
    }
    
    prog = (const regex_program_t *)preg_t->re_compiled;
    if (!prog || prog->magic != REGEX_MAGIC) {
        return REG_BADPAT;
    }
    
    /* Initialize execution state */
    memset(&state, 0, sizeof(state));
    state.string = string;
    state.input = string;
    state.bol = string;
    state.pmatch = pmatch_t;
    state.nmatch = nmatch;
    state.eflags = eflags;
    
    /* Clear pmatch array */
    if (pmatch_t && nmatch > 0) {
        for (i = 0; i < nmatch; i++) {
            pmatch_t[i].rm_so = -1;
            pmatch_t[i].rm_eo = -1;
        }
    }
    
    /* Handle REG_STARTEND flag */
    if (eflags & REG_STARTEND && pmatch_t && nmatch > 0) {
        state.input = string + pmatch_t[0].rm_so;
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
            if (pmatch_t && nmatch > 0) {
                pmatch_t[0].rm_so = s - string;
                pmatch_t[0].rm_eo = state.input - string;
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
ULONG __ASM__ regerror(__REG__(a6, struct RegexBase *rb), __REG__(d0, int errcode), __REG__(a0, const void *preg), 
                       __REG__(a1, char *errbuf), __REG__(d1, ULONG errbuf_size))
{
    const char *msg;
    ULONG len;
    
    msg = regex_error_string(rb, errcode);
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
void __ASM__ regfree(__REG__(a6, struct RegexBase *rb), __REG__(a0, void *preg))
{
    regex_t *preg_t = (regex_t *)preg;
    
    if (preg_t && preg_t->re_compiled) {
        FreeVec(preg_t->re_compiled);
        preg_t->re_compiled = NULL;
    }
}

/* ARexx interface implementation */
LONG __ASM__ rematch(__REG__(a6, struct RegexBase *rb), __REG__(a0, STRPTR regex), __REG__(a1, STRPTR string), 
                    __REG__(d0, LONG flags), __REG__(a2, void *pmatch))
{
    regex_t preg;
    int result;
    int cflags = 0;
    int eflags = 0;
    regmatch_t *pmatch_t = (regmatch_t *)pmatch;
    
    if (!regex || !string) {
        return REG_BADPAT;
    }
    
    /* Convert ARexx flags to POSIX flags */
    if (flags & REGEX_ICASE) cflags |= REG_ICASE;
    if (flags & REGEX_OLD) cflags |= REG_BASIC;
    if (flags & REGEX_NEWLINE) cflags |= REG_NEWLINE;
    if (flags & REGEX_STARTEND) eflags |= REG_STARTEND;
    
    /* Compile pattern */
    result = regcomp(rb, &preg, regex, cflags);
    if (result != 0) {
        return result;
    }
    
    /* Execute pattern */
    result = regexec(rb, &preg, string, 1, pmatch_t, eflags);
    
    /* Free compiled pattern */
    regfree(rb, &preg);
    
    return result;
}

/* Internal regex parsing functions */
static int regex_parse(regex_compile_state_t *state)
{
    return regex_parse_expr(state);
}

static int regex_parse_expr(regex_compile_state_t *state)
{
    int result;
    
    /* Parse first branch */
    result = regex_parse_branch(state);
    if (result != 0) return result;
    
    /* Parse alternations */
    while (*state->parse == '|') {
        state->parse++;
        result = regex_parse_branch(state);
        if (result != 0) return result;
    }
    
    return 0;
}

static int regex_parse_branch(regex_compile_state_t *state)
{
    int result;
    
    /* Parse first piece */
    result = regex_parse_piece(state);
    if (result != 0) return result;
    
    /* Parse remaining pieces */
    while (*state->parse != '\0' && *state->parse != '|' && *state->parse != ')') {
        result = regex_parse_piece(state);
        if (result != 0) return result;
    }
    
    return 0;
}

static int regex_parse_piece(regex_compile_state_t *state)
{
    int result;
    UBYTE op;
    
    /* Parse atom */
    result = regex_parse_atom(state);
    if (result != 0) return result;
    
    /* Parse quantifier */
    switch (*state->parse) {
    case '*':
        op = REG_STAR;
        state->parse++;
        break;
    case '+':
        op = REG_PLUS;
        state->parse++;
        break;
    case '?':
        op = REG_NOTHING; /* Will be handled specially */
        state->parse++;
        break;
    default:
        return 0; /* No quantifier */
    }
    
    /* Emit quantifier node */
    if (op == REG_NOTHING) {
        /* Handle ? as (atom|) */
        result = regex_emit_node(state, REG_BRANCH);
        if (result != 0) return result;
        result = regex_emit_node(state, REG_NOTHING);
        if (result != 0) return result;
    } else {
        result = regex_emit_node(state, op);
        if (result != 0) return result;
    }
    
    return 0;
}

static int regex_parse_atom(regex_compile_state_t *state)
{
    int result;
    UBYTE op;
    const char *start;
    size_t len;
    
    switch (*state->parse) {
    case '^':
        op = REG_BOL;
        state->parse++;
        break;
    case '$':
        op = REG_EOL;
        state->parse++;
        break;
    case '.':
        op = REG_ANY;
        state->parse++;
        break;
    case '[':
        /* Character class */
        start = state->parse;
        state->parse++;
        while (*state->parse != ']' && *state->parse != '\0') {
            state->parse++;
        }
        if (*state->parse != ']') {
            state->error = REG_EBRACK;
            return -1;
        }
        state->parse++;
        len = state->parse - start;
        result = regex_emit(state, REG_ANYOF, start, len);
        if (result != 0) return result;
        return 0;
    case '(':
        /* Group */
        state->parse++;
        if (state->nsub >= MAX_SUBEXPR) {
            state->error = REG_ESUBREG;
            return -1;
        }
        op = REG_OPEN + state->nsub++;
        result = regex_emit_node(state, op);
        if (result != 0) return result;
        result = regex_parse_expr(state);
        if (result != 0) return result;
        if (*state->parse != ')') {
            state->error = REG_EPAREN;
            return -1;
        }
        state->parse++;
        op = REG_CLOSE + state->nsub - 1;
        result = regex_emit_node(state, op);
        if (result != 0) return result;
        return 0;
    case '\\':
        /* Escape sequence */
        state->parse++;
        if (*state->parse == '\0') {
            state->error = REG_EESCAPE;
            return -1;
        }
        /* Fall through to literal character */
    default:
        /* Literal character */
        start = state->parse;
        state->parse++;
        len = 1;
        result = regex_emit(state, REG_EXACTLY, start, len);
        if (result != 0) return result;
        return 0;
    }
    
    result = regex_emit_node(state, op);
    if (result != 0) return result;
    return 0;
}

/* Internal regex matching functions */
static int regex_match_expr(regex_exec_state_t *state, const regex_node_t *node)
{
    return regex_match_branch(state, node);
}

static int regex_match_branch(regex_exec_state_t *state, const regex_node_t *node)
{
    const regex_node_t *next;
    const char *save_input;
    
    /* Try each alternative */
    while (node && node->op == REG_BRANCH) {
        save_input = state->input;
        if (regex_match_piece(state, node + 1)) {
            return 1;
        }
        state->input = save_input;
        next = (const regex_node_t *)((const UBYTE *)node + 
               ((node->next[0] << 8) | node->next[1]));
        node = next;
    }
    
    return 0;
}

static int regex_match_piece(regex_exec_state_t *state, const regex_node_t *node)
{
    int count;
    
    if (!node) return 1;
    
    switch (node->op) {
    case REG_STAR:
        /* Match zero or more */
        count = 0;
        while (regex_match_node(state, node + 1)) {
            count++;
        }
        return 1;
    case REG_PLUS:
        /* Match one or more */
        if (!regex_match_node(state, node + 1)) {
            return 0;
        }
        count = 1;
        while (regex_match_node(state, node + 1)) {
            count++;
        }
        return 1;
    case REG_NOTHING:
        /* Match zero or one */
        regex_match_node(state, node + 1);
        return 1;
    default:
        return regex_match_node(state, node);
    }
}

static int regex_match_node(regex_exec_state_t *state, const regex_node_t *node)
{
    const char *input;
    int len;
    
    if (!node) return 1;
    
    input = state->input;
    
    switch (node->op) {
    case REG_BOL:
        return (input == state->bol);
    case REG_EOL:
        return (*input == '\0');
    case REG_ANY:
        if (*input == '\0') return 0;
        if (node->data[0] & REG_NEWLINE && *input == '\n') return 0;
        state->input++;
        return 1;
    case REG_EXACTLY:
        len = strlen((const char *)node->data);
        if (strncmp(input, (const char *)node->data, len) != 0) return 0;
        state->input += len;
        return 1;
    case REG_ANYOF:
        if (*input == '\0') return 0;
        if (strchr((const char *)node->data, *input) == NULL) return 0;
        state->input++;
        return 1;
    case REG_ANYBUT:
        if (*input == '\0') return 0;
        if (strchr((const char *)node->data, *input) != NULL) return 0;
        state->input++;
        return 1;
    case REG_OPEN:
    case REG_CLOSE:
        /* Subexpression handling */
        return 1;
    default:
        return 0;
    }
}

/* Utility functions */
static int regex_emit(regex_compile_state_t *state, UBYTE op, const void *data, size_t len)
{
    if (state->code) {
        if (state->code + len + 3 > state->code_end) {
            state->error = REG_ESIZE;
            return -1;
        }
        *state->code++ = op;
        *state->code++ = 0; /* next[0] */
        *state->code++ = 0; /* next[1] */
        memcpy(state->code, data, len);
        state->code += len;
    } else {
        state->code_size += len + 3;
    }
    return 0;
}

static int regex_emit_node(regex_compile_state_t *state, UBYTE op)
{
    if (state->code) {
        if (state->code + 3 > state->code_end) {
            state->error = REG_ESIZE;
            return -1;
        }
        *state->code++ = op;
        *state->code++ = 0; /* next[0] */
        *state->code++ = 0; /* next[1] */
    } else {
        state->code_size += 3;
    }
    return 0;
}

static int regex_emit_data(regex_compile_state_t *state, const void *data, size_t len)
{
    if (state->code) {
        if (state->code + len > state->code_end) {
            state->error = REG_ESIZE;
            return -1;
        }
        memcpy(state->code, data, len);
        state->code += len;
    } else {
        state->code_size += len;
    }
    return 0;
}

static int regex_emit_offset(regex_compile_state_t *state, UBYTE *node, int offset)
{
    if (state->code) {
        if (node + 3 > state->code_end) {
            state->error = REG_ESIZE;
            return -1;
        }
        node[1] = (offset >> 8) & 0xFF;
        node[2] = offset & 0xFF;
    }
    return 0;
}

const char *__ASM__ regex_error_string(__REG__(a6, struct RegexBase *rb), __REG__(d0, int error))
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
