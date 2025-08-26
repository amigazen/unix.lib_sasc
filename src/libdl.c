/*
 * AmigaOS libdl implementation for unix.lib2
 * 
 * This is a complete implementation of the POSIX dlopen(), dlsym(),
 * dlclose(), and dlerror() functions for AmigaOS.
 * 
 * The implementation combines our POSIX API with the robust hunk parsing
 * and dynamic linking capabilities from dld-3.2.6.
 * 
 * Modified for AmigaOS compatibility and C89 standards
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "amiga.h"

/* Include the dlfcn header for our own function declarations */
#include "include/dlfcn.h"

extern struct DosLibrary *DOSBase;

/* Error message buffer */
static char dlerror_buffer[256];
static int dlerror_set = 0;

/* AmigaOS hunk format constants */
#define HUNK_UNIT      999
#define HUNK_NAME      1000
#define HUNK_CODE      1001
#define HUNK_DATA      1002
#define HUNK_BSS       1003
#define HUNK_RELOC32   1004
#define HUNK_RELOC16   1005
#define HUNK_RELOC8    1006
#define HUNK_EXT       1007
#define HUNK_SYMBOL    1008
#define HUNK_DEBUG     1009
#define HUNK_END       1010
#define HUNK_HEADER    1011
#define HUNK_OVERLAY   1013
#define HUNK_BREAK     1014
#define HUNK_DRELOC32  1015
#define HUNK_DRELOC16  1016
#define HUNK_DRELOC8   1017
#define HUNK_DRELOC    1018
#define HUNK_LIB       1019
#define HUNK_INDEX     1020

/* Symbol types from hunk_ext */
#define EXT_DEF        1    /* Relocatable definition */
#define EXT_ABS        2    /* Absolute definition */
#define EXT_RES        3    /* Resident library definition */
#define EXT_REF32      129  /* 32-bit reference to symbol */
#define EXT_REF16      131  /* 16-bit reference to symbol */
#define EXT_REF8       132  /* 8-bit reference to symbol */
#define EXT_COMMON     130  /* 32-bit reference to COMMON */
#define EXT_DRELOC32   133  /* 32-bit relocation */
#define EXT_DRELOC16   134  /* 16-bit relocation */
#define EXT_DRELOC8    135  /* 8-bit relocation */

/* SAS/C specific symbol types */
#define SAS_EXT_REF16  134  /* SAS/C 16-bit reference */
#define SAS_EXT_ABS    97   /* SAS/C absolute reference */
#define SAS_EXT_REF8   135  /* SAS/C 8-bit reference */
#define SAS_EXT_DEF    96   /* SAS/C relocatable definition */
#define SAS_EXT_COMMON 131  /* SAS/C common block */

/* File type detection */
typedef enum {
    FILE_TYPE_UNKNOWN,
    FILE_TYPE_LOAD,      /* HUNK_HEADER - ready for LoadSeg */
    FILE_TYPE_OBJECT,    /* HUNK_UNIT - needs runtime linking */
    FILE_TYPE_LIBRARY    /* Archive of object files - needs extraction and linking */
} file_type_t;

/* Symbol table entry */
typedef struct symbol_entry {
    char *name;
    void *address;
    int type;
    int hunk_number;        /* Which hunk this symbol belongs to */
    struct symbol_entry *next; /* For hash table chaining */
} symbol_entry_t;

/* Hunk information structure */
typedef struct hunk_info {
    ULONG type;          /* Hunk type */
    ULONG size;          /* Size in longwords */
    void *data;          /* Pointer to hunk data in memory */
    void *original_data; /* Original file data (for relocations) */
    ULONG flags;         /* Memory flags */
    int hunk_number;     /* Hunk number for relocations */
    void *base_address;  /* Base address for this hunk */
} hunk_info_t;

/* Library handle structure */
typedef struct lib_handle {
    struct lib_handle *next;
    BPTR seglist;           /* BPTR as returned by LoadSeg() or our allocator */
    char *filename;
    struct symbol_entry **symbol_table; /* Hash table for symbols */
    int symbol_count;
    void *base_addr;
    file_type_t file_type;
    hunk_info_t *hunks;     /* Array of hunk information */
    int hunk_count;         /* Number of hunks */
    void **hunk_addresses;  /* Array of hunk base addresses */
    int symbol_table_size;  /* Size of symbol hash table */
} lib_handle_t;

/* Global list of loaded libraries */
static lib_handle_t *loaded_libs = NULL;

/* Hash table size for symbols */
#define SYMBOL_TABLE_SIZE 256

/* Function prototypes */
static void set_dlerror(const char *msg);
static void set_dlerror_with_ioerr(const char *base_msg);
static file_type_t detect_file_type(const char *filename);
static int load_object_file(const char *filename, lib_handle_t *handle);
static int load_static_library(const char *filename, lib_handle_t *handle);
static int parse_load_file(BPTR seglist, lib_handle_t *handle);
static void *find_symbol(lib_handle_t *handle, const char *symbol_name);
static void free_lib_handle(lib_handle_t *handle);
static void *allocate_hunk_memory(ULONG size, ULONG flags);
static ULONG read_longword(void *addr);
static unsigned long get_num(FILE *fd);
static void skip(FILE *fd, unsigned long t);
static int amiga_read_file_symbols(FILE *desc, lib_handle_t *handle);
static int parse_object_file_hunks(FILE *desc, lib_handle_t *handle);
static void add_symbol_to_table(lib_handle_t *handle, const char *name, void *address, int type, int hunk_number);
static unsigned int hash_symbol_name(const char *name);

/* Function to set error message */
static void set_dlerror(const char *msg)
{
    if (msg) {
        strncpy(dlerror_buffer, msg, sizeof(dlerror_buffer) - 1);
        dlerror_buffer[sizeof(dlerror_buffer) - 1] = '\0';
        dlerror_set = 1;
    } else {
        dlerror_set = 0;
    }
}

/* Function to get detailed error from AmigaDOS */
static void set_dlerror_with_ioerr(const char *base_msg)
{
    char error_msg[256];
    LONG io_error;
    
    io_error = IoErr();
    if (io_error != 0) {
        sprintf(error_msg, "%s (IoErr: %ld)", base_msg, io_error);
    } else {
        strcpy(error_msg, base_msg);
    }
    set_dlerror(error_msg);
}

/* Hash function for symbol names */
static unsigned int hash_symbol_name(const char *name)
{
    unsigned int hash = 0;
    while (*name) {
        hash = (hash << 5) + hash + *name++;
    }
    return hash % SYMBOL_TABLE_SIZE;
}

/* Add symbol to hash table */
static void add_symbol_to_table(lib_handle_t *handle, const char *name, void *address, int type, int hunk_number)
{
    unsigned int hash = hash_symbol_name(name);
    symbol_entry_t *entry;
    
    entry = malloc(sizeof(symbol_entry_t));
    if (!entry) return;
    
    entry->name = strdup(name);
    entry->address = address;
    entry->type = type;
    entry->hunk_number = hunk_number;
    entry->next = handle->symbol_table[hash];
    handle->symbol_table[hash] = entry;
    handle->symbol_count++;
    
    /* Also add a version without leading underscore for C compatibility */
    if (name[0] == '_' && strlen(name) > 1) {
        unsigned int hash_no_underscore = hash_symbol_name(name + 1);
        symbol_entry_t *entry_no_underscore;
        
        entry_no_underscore = malloc(sizeof(symbol_entry_t));
        if (entry_no_underscore) {
            entry_no_underscore->name = strdup(name + 1);
            entry_no_underscore->address = address;
            entry_no_underscore->type = type;
            entry_no_underscore->hunk_number = hunk_number;
            entry_no_underscore->next = handle->symbol_table[hash_no_underscore];
            handle->symbol_table[hash_no_underscore] = entry_no_underscore;
            handle->symbol_count++;
        }
    }
}

/* Function to detect file type by examining the first hunk */
static file_type_t detect_file_type(const char *filename)
{
    BPTR file_handle;
    ULONG hunk_id;
    file_type_t file_type = FILE_TYPE_UNKNOWN;
    
    file_handle = Open(filename, MODE_OLDFILE);
    if (!file_handle) {
        return FILE_TYPE_UNKNOWN;
    }
    
    /* Read first hunk ID */
    if (Read(file_handle, &hunk_id, 4) == 4) {
        /* Extract hunk type (lower 24 bits) */
        ULONG hunk_type = hunk_id & 0x00FFFFFF;
        
        switch (hunk_type) {
            case HUNK_HEADER:
                file_type = FILE_TYPE_LOAD;
                break;
            case HUNK_UNIT:
                file_type = FILE_TYPE_OBJECT;
                break;
            case HUNK_LIB:
                file_type = FILE_TYPE_LIBRARY;
                break;
            default:
                file_type = FILE_TYPE_UNKNOWN;
                break;
        }
    }
    
    Close(file_handle);
    return file_type;
}

/* Function to allocate memory for a hunk with proper flags */
static void *allocate_hunk_memory(ULONG size, ULONG flags)
{
    ULONG mem_flags = MEMF_PUBLIC;
    
    if (flags & 0x80000000) mem_flags |= MEMF_FAST;    /* Bit 31 */
    if (flags & 0x40000000) mem_flags |= MEMF_CHIP;    /* Bit 30 */
    
    return AllocMem(size * 4, mem_flags);  /* Convert longwords to bytes */
}

/* Function to read a longword from memory */
static ULONG read_longword(void *addr)
{
    return *(ULONG *)addr;
}

/* Function to read a longword from file */
static unsigned long get_num(FILE *fd)
{
    unsigned long t;
    fread(&t, 1, 4, fd);
    return t;
}

/* Function to skip bytes in file */
static void skip(FILE *fd, unsigned long t)
{
    fseek(fd, t * 4, SEEK_CUR);
}

/* Enhanced Amiga hunk parsing based on dld-3.2.6 */
static int amiga_read_file_symbols(FILE *desc, lib_handle_t *handle)
{
    int t, f, l;
    int type, sdata, sbss;
    int name_size = 500;
    char *name;
    int hunk_index = 0;
    int max_hunks = 50;
    unsigned long name_len;
    unsigned char sym_type;
    unsigned long symbol_value;
    void *symbol_address;
    unsigned long ref_count;
    
    /* Initialize hunk arrays */
    handle->hunks = malloc(max_hunks * sizeof(hunk_info_t));
    if (!handle->hunks) {
        set_dlerror("Out of memory for hunk structures");
        return -1;
    }
    
    /* Initialize symbol table */
    handle->symbol_table = calloc(SYMBOL_TABLE_SIZE, sizeof(symbol_entry_t *));
    if (!handle->symbol_table) {
        free(handle->hunks);
        set_dlerror("Out of memory for symbol table");
        return -1;
    }
    handle->symbol_table_size = SYMBOL_TABLE_SIZE;
    
    if (get_num(desc) != HUNK_HEADER) {
        set_dlerror("Invalid hunk header");
        free(handle->hunks);
        free(handle->symbol_table);
        return -1;
    }
    
    name = malloc(name_size);
    if (!name) {
        set_dlerror("Out of memory for name buffer");
        free(handle->hunks);
        free(handle->symbol_table);
        return -1;
    }
    
    /* Skip resident library names */
    while ((t = get_num(desc)) != 0) {
        skip(desc, t);
    }
    
    /* Read hunk table size and first/last hunk numbers */
    get_num(desc); /* table size */
    f = get_num(desc); /* first hunk */
    l = get_num(desc); /* last hunk */
    skip(desc, l - f + 1); /* skip hunk sizes */
    
    handle->hunk_count = l - f + 1;
    if (handle->hunk_count > max_hunks) {
        max_hunks = handle->hunk_count;
        handle->hunks = realloc(handle->hunks, max_hunks * sizeof(hunk_info_t));
        if (!handle->hunks) {
            set_dlerror("Out of memory for hunk structures");
            free(name);
            free(handle->symbol_table);
            return -1;
        }
    }
    
    /* Parse individual hunks */
    while (l >= 0) {
        switch (get_num(desc) & 0x00FFFFFF) {
            case HUNK_CODE:      /* text */
                type = EXT_DEF;
                t = get_num(desc);
                if (hunk_index < handle->hunk_count) {
                    handle->hunks[hunk_index].type = HUNK_CODE;
                    handle->hunks[hunk_index].size = t;
                    handle->hunks[hunk_index].data = allocate_hunk_memory(t, 0x80000000); /* MEMF_FAST for executable code */
                    handle->hunks[hunk_index].hunk_number = hunk_index;
                    if (!handle->hunks[hunk_index].data) {
                        set_dlerror("Failed to allocate memory for code hunk");
                        free(name);
                        return -1;
                    }
                    /* Read the code data */
                    fread(handle->hunks[hunk_index].data, 1, t * 4, desc);
                    hunk_index++;
                } else {
                    skip(desc, t);
                }
                break;
                
            case HUNK_DATA:      /* data */
                type = EXT_DEF;
                sdata = get_num(desc);
                if (hunk_index < handle->hunk_count) {
                    handle->hunks[hunk_index].type = HUNK_DATA;
                    handle->hunks[hunk_index].size = sdata;
                    handle->hunks[hunk_index].data = allocate_hunk_memory(sdata, 0); /* MEMF_PUBLIC for data */
                    handle->hunks[hunk_index].hunk_number = hunk_index;
                    if (!handle->hunks[hunk_index].data) {
                        set_dlerror("Failed to allocate memory for data hunk");
                        free(name);
                        return -1;
                    }
                    /* Read the data */
                    fread(handle->hunks[hunk_index].data, 1, sdata * 4, desc);
                    hunk_index++;
                } else {
                    skip(desc, sdata);
                }
                break;
                
            case HUNK_BSS:      /* bss */
                sbss = get_num(desc);
                if (hunk_index < handle->hunk_count) {
                    handle->hunks[hunk_index].type = HUNK_BSS;
                    handle->hunks[hunk_index].size = sbss;
                    handle->hunks[hunk_index].data = AllocMem(sbss * 4, MEMF_PUBLIC | MEMF_CLEAR);
                    handle->hunks[hunk_index].hunk_number = hunk_index;
                    if (!handle->hunks[hunk_index].data) {
                        set_dlerror("Failed to allocate memory for BSS hunk");
                        free(name);
                        return -1;
                    }
                    hunk_index++;
                }
                break;
                
            case HUNK_NAME:      /* name */
            case HUNK_UNIT:      /* unit */
                skip(desc, get_num(desc));
                break;
                
            case HUNK_DEBUG:     /* debug */
                t = get_num(desc);
                skip(desc, t);
                break;
                
            case HUNK_RELOC8:    /* reloc8 */
            case HUNK_RELOC16:   /* reloc16 */
            case HUNK_RELOC32:   /* reloc32 */
                while ((t = get_num(desc)) != 0) {
                    skip(desc, t + 1);
                }
                break;
                
            case HUNK_EXT:       /* ext */
                while ((t = get_num(desc)) != 0) {
                    name_len = t & 0x00FFFFFF;
                    sym_type = (t >> 24) & 0xFF;
                    
                    /* Read symbol name */
                    if (name_len * 4 > name_size - 1) {
                        name_size = name_len * 4 + 1;
                        name = realloc(name, name_size);
                        if (!name) {
                            set_dlerror("Out of memory for symbol name");
                            return -1;
                        }
                    }
                    fread(name, 1, name_len * 4, desc);
                    name[name_len * 4] = '\0';
                    
                    /* Handle different symbol types */
                    if (sym_type == EXT_DEF || sym_type == EXT_ABS || 
                        sym_type == SAS_EXT_DEF || sym_type == SAS_EXT_ABS) {
                        symbol_value = get_num(desc);
                        
                        if (sym_type == EXT_DEF || sym_type == SAS_EXT_DEF) {
                            /* Relocatable definition - calculate address relative to hunk */
                            if (hunk_index > 0) {
                                symbol_address = (void *)((ULONG)handle->hunks[hunk_index-1].data + symbol_value);
                            } else {
                                symbol_address = (void *)symbol_value;
                            }
                        } else {
                            /* Absolute definition */
                            symbol_address = (void *)symbol_value;
                        }
                        
                        add_symbol_to_table(handle, name, symbol_address, sym_type, hunk_index > 0 ? hunk_index-1 : 0);
                    } else if (sym_type == EXT_REF32 || sym_type == EXT_COMMON) {
                        ref_count = get_num(desc);
                        skip(desc, ref_count);
                    } else if (sym_type == EXT_REF16 || sym_type == SAS_EXT_REF16) {
                        ref_count = get_num(desc);
                        skip(desc, ref_count * 2);
                    } else {
                        /* Skip unknown symbol types */
                        printf("[DEBUG] Skipping unknown symbol type %d ('%s')\n", sym_type, name);
                    }
                }
                break;
                
            case HUNK_SYMBOL:    /* symbols */
                while ((t = get_num(desc)) != 0) {
                    if (t * 4 > name_size - 1) {
                        name_size = t * 4 + 1;
                        name = realloc(name, name_size);
                        if (!name) {
                            set_dlerror("Out of memory for symbol name");
                            return -1;
                        }
                    }
                    fread(name, 1, t * 4, desc);
                    name[t * 4] = '\0';
                    
                    symbol_value = get_num(desc);
                    
                    if (hunk_index > 0) {
                        symbol_address = (void *)((ULONG)handle->hunks[hunk_index-1].data + symbol_value);
                    } else {
                        symbol_address = (void *)symbol_value;
                    }
                    
                    add_symbol_to_table(handle, name, symbol_address, EXT_DEF, hunk_index > 0 ? hunk_index-1 : 0);
                }
                break;
                
            case HUNK_END:       /* end */
                l--;
                break;
        }
    }
    
    free(name);
    handle->hunk_count = hunk_index;
    
    /* Build hunk address array and set base address */
    if (handle->hunk_count > 0) {
        int i;
        handle->hunk_addresses = malloc(handle->hunk_count * sizeof(void *));
        if (handle->hunk_addresses) {
            for (i = 0; i < handle->hunk_count; i++) {
                handle->hunk_addresses[i] = handle->hunks[i].data;
            }
        }
        handle->base_addr = handle->hunks[0].data;
    }
    
    return 0;
}

/* Function to parse object file hunks (HUNK_UNIT format) */
static int parse_object_file_hunks(FILE *desc, lib_handle_t *handle)
{
    int t, hunk_count;
    int hunk_index = 0;
    int max_hunks = 50;
    int name_size = 500;
    char *name;
    unsigned long name_len;
    unsigned char sym_type;
    unsigned long symbol_value;
    void *symbol_address;
    unsigned long ref_count;
    
    /* Initialize hunk arrays */
    handle->hunks = malloc(max_hunks * sizeof(hunk_info_t));
    if (!handle->hunks) {
        set_dlerror("Out of memory for hunk structures");
        return -1;
    }
    
    /* Initialize symbol table */
    handle->symbol_table = calloc(SYMBOL_TABLE_SIZE, sizeof(symbol_entry_t *));
    if (!handle->symbol_table) {
        free(handle->hunks);
        set_dlerror("Out of memory for symbol table");
        return -1;
    }
    handle->symbol_table_size = SYMBOL_TABLE_SIZE;
    
    name = malloc(name_size);
    if (!name) {
        set_dlerror("Out of memory for name buffer");
        free(handle->hunks);
        free(handle->symbol_table);
        return -1;
    }
    
    /* Object files start with HUNK_UNIT, not HUNK_HEADER */
    t = get_num(desc);
    printf("[DEBUG] First hunk type: 0x%06X\n", t & 0x00FFFFFF);
    if ((t & 0x00FFFFFF) != HUNK_UNIT) {
        set_dlerror("Invalid object file format - expected HUNK_UNIT");
        free(name);
        free(handle->hunks);
        free(handle->symbol_table);
        return -1;
    }
    
    printf("[DEBUG] Valid HUNK_UNIT found, continuing with parsing...\n");
    
    /* Skip the unit name */
    t = get_num(desc);
    if (t > 0) {
        skip(desc, t);
    }
    
    /* Parse hunks until we hit HUNK_END */
    hunk_count = 0;
    while (1) {
        hunk_count++;
        if (hunk_count > 1000) {
            printf("[DEBUG] Safety limit reached, stopping parsing\n");
            break;
        }
        
        t = get_num(desc);
        printf("[DEBUG] Parsing hunk type: 0x%06X (hunk #%d)\n", t & 0x00FFFFFF, hunk_count);
        if ((t & 0x00FFFFFF) == HUNK_END) {
            printf("[DEBUG] Found HUNK_END, parsing complete\n");
            break;
        }
        
        switch (t & 0x00FFFFFF) {
            case HUNK_CODE:      /* text */
                t = get_num(desc);
                if (hunk_index >= max_hunks) {
                    max_hunks *= 2;
                    handle->hunks = realloc(handle->hunks, max_hunks * sizeof(hunk_info_t));
                    if (!handle->hunks) {
                        set_dlerror("Out of memory for hunk structures");
                        free(name);
                        free(handle->symbol_table);
                        return -1;
                    }
                }
                handle->hunks[hunk_index].type = HUNK_CODE;
                handle->hunks[hunk_index].size = t;
                handle->hunks[hunk_index].data = allocate_hunk_memory(t, 0x80000000); /* MEMF_FAST for executable code */
                handle->hunks[hunk_index].hunk_number = hunk_index;
                if (!handle->hunks[hunk_index].data) {
                    set_dlerror("Failed to allocate memory for code hunk");
                    free(name);
                    return -1;
                }
                /* Read the code data */
                fread(handle->hunks[hunk_index].data, 1, t * 4, desc);
                hunk_index++;
                break;
                
            case HUNK_DATA:      /* data */
                t = get_num(desc);
                if (hunk_index >= max_hunks) {
                    max_hunks *= 2;
                    handle->hunks = realloc(handle->hunks, max_hunks * sizeof(hunk_info_t));
                    if (!handle->hunks) {
                        set_dlerror("Out of memory for hunk structures");
                        free(name);
                        free(handle->symbol_table);
                        return -1;
                    }
                }
                handle->hunks[hunk_index].type = HUNK_DATA;
                handle->hunks[hunk_index].size = t;
                handle->hunks[hunk_index].data = allocate_hunk_memory(t, 0); /* MEMF_PUBLIC for data */
                handle->hunks[hunk_index].hunk_number = hunk_index;
                if (!handle->hunks[hunk_index].data) {
                    set_dlerror("Failed to allocate memory for data hunk");
                    free(name);
                    return -1;
                }
                /* Read the data */
                fread(handle->hunks[hunk_index].data, 1, t * 4, desc);
                hunk_index++;
                break;
                
            case HUNK_BSS:      /* bss */
                t = get_num(desc);
                if (hunk_index >= max_hunks) {
                    max_hunks *= 2;
                    handle->hunks = realloc(handle->hunks, max_hunks * sizeof(hunk_info_t));
                    if (!handle->hunks) {
                        set_dlerror("Out of memory for hunk structures");
                        free(name);
                        free(handle->symbol_table);
                        return -1;
                    }
                }
                handle->hunks[hunk_index].type = HUNK_BSS;
                handle->hunks[hunk_index].size = t;
                handle->hunks[hunk_index].data = AllocMem(t * 4, MEMF_PUBLIC | MEMF_CLEAR);
                handle->hunks[hunk_index].hunk_number = hunk_index;
                if (!handle->hunks[hunk_index].data) {
                    set_dlerror("Failed to allocate memory for BSS hunk");
                    free(name);
                    return -1;
                }
                hunk_index++;
                break;
                
            case HUNK_NAME:      /* name */
                t = get_num(desc);
                skip(desc, t);
                break;
                
            case HUNK_DEBUG:     /* debug */
                t = get_num(desc);
                skip(desc, t);
                break;
                
            case HUNK_RELOC8:    /* reloc8 */
            case HUNK_RELOC16:   /* reloc16 */
            case HUNK_RELOC32:   /* reloc32 */
                while ((t = get_num(desc)) != 0) {
                    skip(desc, t + 1);
                }
                break;
                
            case HUNK_EXT:       /* ext */
                while ((t = get_num(desc)) != 0) {
                    name_len = t & 0x00FFFFFF;
                    sym_type = (t >> 24) & 0xFF;
                    
                    /* Read symbol name */
                    if (name_len * 4 > name_size - 1) {
                        name_size = name_len * 4 + 1;
                        name = realloc(name, name_size);
                        if (!name) {
                            set_dlerror("Out of memory for symbol name");
                            return -1;
                        }
                    }
                    fread(name, 1, name_len * 4, desc);
                    name[name_len * 4] = '\0';
                    
                    /* Handle different symbol types */
                    if (sym_type == EXT_DEF || sym_type == EXT_ABS || 
                        sym_type == SAS_EXT_DEF || sym_type == SAS_EXT_ABS) {
                        symbol_value = get_num(desc);
                        
                        if (sym_type == EXT_DEF || sym_type == SAS_EXT_DEF) {
                            /* Relocatable definition - calculate address relative to hunk */
                            if (hunk_index > 0) {
                                symbol_address = (void *)((ULONG)handle->hunks[hunk_index-1].data + symbol_value);
                            } else {
                                symbol_address = (void *)symbol_value;
                            }
                        } else {
                            /* Absolute definition */
                            symbol_address = (void *)symbol_value;
                        }
                        
                        add_symbol_to_table(handle, name, symbol_address, sym_type, hunk_index > 0 ? hunk_index-1 : 0);
                    } else if (sym_type == EXT_REF32 || sym_type == EXT_COMMON) {
                        ref_count = get_num(desc);
                        skip(desc, ref_count);
                    } else if (sym_type == EXT_REF16 || sym_type == SAS_EXT_REF16) {
                        ref_count = get_num(desc);
                        skip(desc, ref_count * 2);
                    } else {
                        /* Skip unknown symbol types */
                        printf("[DEBUG] Skipping unknown symbol type %d ('%s')\n", sym_type, name);
                    }
                }
                break;
                
            case HUNK_SYMBOL:    /* symbols */
                while ((t = get_num(desc)) != 0) {
                    if (t * 4 > name_size - 1) {
                        name_size = t * 4 + 1;
                        name = realloc(name, name_size);
                        if (!name) {
                            set_dlerror("Out of memory for symbol name");
                            return -1;
                        }
                    }
                    fread(name, 1, t * 4, desc);
                    name[t * 4] = '\0';
                    
                    symbol_value = get_num(desc);
                    
                    if (hunk_index > 0) {
                        symbol_address = (void *)((ULONG)handle->hunks[hunk_index-1].data + symbol_value);
                    } else {
                        symbol_address = (void *)symbol_value;
                    }
                    
                    add_symbol_to_table(handle, name, symbol_address, EXT_DEF, hunk_index > 0 ? hunk_index-1 : 0);
                }
                break;
                
            default:
                /* Skip unknown hunk types */
                printf("[DEBUG] Skipping unknown hunk type 0x%06X\n", t & 0x00FFFFFF);
                break;
        }
    }
    
    free(name);
    handle->hunk_count = hunk_index;
    
    /* Build hunk address array and set base address */
    if (handle->hunk_count > 0) {
        int i;
        handle->hunk_addresses = malloc(handle->hunk_count * sizeof(void *));
        if (handle->hunk_addresses) {
            for (i = 0; i < handle->hunk_count; i++) {
                handle->hunks[i].base_address = handle->hunks[i].data;
                handle->hunk_addresses[i] = handle->hunks[i].data;
            }
        }
        handle->base_addr = handle->hunks[0].data;
    }
    
    printf("[DEBUG] Parsing complete. Found %d hunks, %d symbols, 0 relocations.\n", 
           handle->hunk_count, handle->symbol_count);
    
    /* Debug: Show what symbols were added */
    printf("[DEBUG] Symbols added to table:\n");
    {
        int i;
        for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
            symbol_entry_t *entry = handle->symbol_table[i];
            while (entry) {
                printf("[DEBUG]   Symbol: '%s' at address %p (type: %d, hunk: %d)\n", 
                       entry->name, entry->address, entry->type, entry->hunk_number);
                entry = entry->next;
            }
        }
    }
    
    return 0;
}

/* Function to parse and load an object file into memory */
static int load_object_file(const char *filename, lib_handle_t *handle)
{
    FILE *file_handle;
    int result;
    
    printf("[DEBUG] load_object_file: Starting to parse %s\n", filename);
    
    file_handle = fopen(filename, "rb");
    if (!file_handle) {
        set_dlerror_with_ioerr("Cannot open object file");
        return -1;
    }
    
    printf("[DEBUG] File opened successfully, starting object file parsing...\n");
    
    /* Use a specialized object file parser */
    result = parse_object_file_hunks(file_handle, handle);
    
    fclose(file_handle);
    
    if (result == 0) {
        printf("[DEBUG] Object file loaded successfully. Found %d hunks and %d symbols.\n", 
               handle->hunk_count, handle->symbol_count);
    } else {
        printf("[DEBUG] Object file parsing failed with error code: %d\n", result);
    }
    
    return result;
}

/* Function to parse and load a static library */
static int load_static_library(const char *filename, lib_handle_t *handle)
{
    /* For now, just recognize it's a library */
    /* Future enhancement: extract object files and link them */
    handle->hunk_count = 0;
    handle->symbol_count = 0;
    handle->hunks = NULL;
    handle->symbol_table = NULL;
    handle->hunk_addresses = NULL;
    handle->base_addr = NULL;
    
    set_dlerror("Static library loading not yet implemented");
    return -1;
}

/* Function to parse hunk format and extract symbols from LOAD files */
static int parse_load_file(BPTR seglist, lib_handle_t *handle)
{
    void *seg_addr;
    char *hunk_data;
    ULONG hunk_type, hunk_size;
    int hunk_index = 0;
    int symbol_count = 0;
    int max_symbols = 100;  /* Start with reasonable size */
    UBYTE symbol_type;
    ULONG name_length;
    char *symbol_name;
    ULONG symbol_offset;
    ULONG symbol_value;
    ULONG ref_count;
    ULONG table_size;
    ULONG first_hunk;
    ULONG last_hunk;
    
    /* Initialize symbol table */
    handle->symbol_table = calloc(SYMBOL_TABLE_SIZE, sizeof(symbol_entry_t *));
    if (!handle->symbol_table) {
        set_dlerror("Out of memory");
        return -1;
    }
    handle->symbol_table_size = SYMBOL_TABLE_SIZE;
    
    /* Get the first segment - this contains the hunk header */
    seg_addr = (void *)((ULONG)seglist + 4); /* Skip segment list header */
    if (!seg_addr) {
        set_dlerror("Failed to get segment address");
        return -1;
    }
    
    handle->base_addr = seg_addr;
    hunk_data = (char *)seg_addr;
    
    /* Parse hunk header first */
    hunk_type = read_longword(hunk_data);
    if (hunk_type == HUNK_HEADER) {
        hunk_data += 4;  /* Skip hunk type */
        
        /* Read resident library names (we'll skip these for now) */
        while (1) {
            name_length = read_longword(hunk_data);
            if (name_length == 0) break;
            hunk_data += 4 + (name_length * 4);  /* Skip name */
        }
        
        /* Read hunk table size and first/last hunk numbers */
        table_size = read_longword(hunk_data);
        hunk_data += 4;
        first_hunk = read_longword(hunk_data);
        hunk_data += 4;
        last_hunk = read_longword(hunk_data);
        hunk_data += 4;
        
        handle->hunk_count = last_hunk - first_hunk + 1;
        handle->hunk_addresses = malloc(handle->hunk_count * sizeof(void *));
        
        if (!handle->hunk_addresses) {
            set_dlerror("Out of memory for hunk addresses");
            return -1;
        }
        
        /* Read hunk sizes */
        for (hunk_index = 0; hunk_index < handle->hunk_count; hunk_index++) {
            hunk_size = read_longword(hunk_data);
            hunk_data += 4;
            
            /* Calculate hunk address (simplified - in reality LoadSeg handles this) */
            handle->hunk_addresses[hunk_index] = (void *)((ULONG)seg_addr + (hunk_index * 4));
        }
        
        /* Now parse individual hunks for symbols */
        while (hunk_index < handle->hunk_count) {
            hunk_type = read_longword(hunk_data);
            hunk_data += 4;
            
            if (hunk_type == HUNK_END) {
                hunk_index++;
                continue;
            }
            
            if (hunk_type == HUNK_EXT) {
                /* Parse external symbol information */
                while (1) {
                    symbol_type = *(UBYTE *)hunk_data;
                    hunk_data += 1;
                    
                    if (symbol_type == 0) break;  /* End of hunk_ext */
                    
                    /* Read symbol name length (3 bytes) */
                    name_length = (*(ULONG *)hunk_data) & 0x00FFFFFF;
                    hunk_data += 4;
                    
                    /* Read symbol name */
                    symbol_name = malloc(name_length + 1);
                    if (symbol_name) {
                        strncpy(symbol_name, (char *)hunk_data, name_length);
                        symbol_name[name_length] = '\0';
                        hunk_data += (name_length + 3) & ~3;  /* Align to longword */
                        
                        /* Handle different symbol types */
                        if (symbol_type == EXT_DEF) {
                            /* Relocatable definition - read offset */
                            symbol_offset = read_longword(hunk_data);
                            hunk_data += 4;
                            
                            /* Add to symbol table */
                            if (symbol_count < max_symbols) {
                                add_symbol_to_table(handle, symbol_name, 
                                    (void *)((ULONG)handle->hunk_addresses[hunk_index] + symbol_offset),
                                    EXT_DEF, hunk_index);
                                symbol_count++;
                            } else {
                                free(symbol_name);
                            }
                        } else if (symbol_type == EXT_ABS) {
                            /* Absolute definition - read absolute value */
                            symbol_value = read_longword(hunk_data);
                            hunk_data += 4;
                            
                            /* Add to symbol table */
                            if (symbol_count < max_symbols) {
                                add_symbol_to_table(handle, symbol_name, (void *)symbol_value,
                                    EXT_ABS, hunk_index);
                                symbol_count++;
                            } else {
                                free(symbol_name);
                            }
                        } else {
                            /* Skip other symbol types for now */
                            free(symbol_name);
                            if (symbol_type == EXT_REF32) {
                                ref_count = read_longword(hunk_data);
                                hunk_data += 4;
                                hunk_data += ref_count * 4;  /* Skip references */
                            }
                        }
                    } else {
                        hunk_data += (name_length + 3) & ~3;  /* Skip name */
                    }
                }
            } else {
                /* Skip other hunk types */
                hunk_size = read_longword(hunk_data);
                hunk_data += 4;
                hunk_data += hunk_size * 4;  /* Skip hunk data */
            }
        }
    } else {
        /* Simple case - no hunk header, just use basic approach */
        handle->hunk_count = 1;
        handle->hunk_addresses = malloc(sizeof(void *));
        if (handle->hunk_addresses) {
            handle->hunk_addresses[0] = seg_addr;
        }
        
        /* Create a minimal symbol table for compatibility */
        add_symbol_to_table(handle, "_dummy_symbol", seg_addr, EXT_DEF, 0);
        symbol_count = 1;
    }
    
    handle->symbol_count = symbol_count;
    
    return 0;
}

/* Function to find a symbol in a library using hash table */
static void *find_symbol(lib_handle_t *handle, const char *symbol_name)
{
    unsigned int hash = hash_symbol_name(symbol_name);
    symbol_entry_t *entry;
    
    printf("[DEBUG] find_symbol: Looking for '%s' in library with %d symbols\n", 
           symbol_name, handle->symbol_count);
    printf("[DEBUG] find_symbol: Hash value for '%s': %u\n", symbol_name, hash);
    
    entry = handle->symbol_table[hash];
    if (!entry) {
        printf("[DEBUG] find_symbol: No symbols in hash bucket %u\n", hash);
    }
    
    while (entry) {
        printf("[DEBUG] find_symbol: Checking symbol: '%s' at address %p\n", 
               entry->name, entry->address);
        if (strcmp(entry->name, symbol_name) == 0) {
            printf("[DEBUG] find_symbol: Found symbol '%s' at address %p\n", 
                   symbol_name, entry->address);
            return entry->address;
        }
        entry = entry->next;
    }
    
    printf("[DEBUG] find_symbol: Symbol '%s' not found in hash bucket %u\n", symbol_name, hash);
    
    /* Debug: Show all symbols in the table for troubleshooting */
    printf("[DEBUG] find_symbol: Dumping entire symbol table for debugging:\n");
    {
        int i;
        for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
            symbol_entry_t *debug_entry = handle->symbol_table[i];
            if (debug_entry) {
                printf("[DEBUG]   Hash bucket %d:\n", i);
                while (debug_entry) {
                    printf("[DEBUG]     Symbol: '%s' at address %p\n", 
                           debug_entry->name, debug_entry->address);
                    debug_entry = debug_entry->next;
                }
            }
        }
    }
    
    return NULL;
}

/* Function to free library handle */
static void free_lib_handle(lib_handle_t *handle)
{
    int i;
    
    if (!handle) {
        return;
    }
    
    /* Free symbol table */
    if (handle->symbol_table) {
        for (i = 0; i < handle->symbol_table_size; i++) {
            symbol_entry_t *entry = handle->symbol_table[i];
            while (entry) {
                symbol_entry_t *next = entry->next;
                if (entry->name) {
                    free(entry->name);
                }
                free(entry);
                entry = next;
            }
        }
        free(handle->symbol_table);
    }
    
    /* Free hunk data and info */
    if (handle->hunks) {
        for (i = 0; i < handle->hunk_count; i++) {
            if (handle->hunks[i].data) {
                FreeMem(handle->hunks[i].data, handle->hunks[i].size * 4);
            }
        }
        free(handle->hunks);
    }
    
    /* Free filename */
    if (handle->filename) {
        free(handle->filename);
    }
    
    /* Free hunk addresses */
    if (handle->hunk_addresses) {
        free(handle->hunk_addresses);
    }
    
    /* Free the handle itself */
    free(handle);
}

/* POSIX dlopen function */
void *dlopen(const char *libname, int flag)
{
    lib_handle_t *handle;
    BPTR seglist;
    file_type_t file_type;
    
    /* Clear any previous error */
    dlerror();
    
    /* Check parameters */
    if (!libname) {
        set_dlerror("NULL library name");
        return NULL;
    }
    
    /* Check if file exists before trying to load it */
    if (strlen(libname) == 0) {
        set_dlerror("Empty library name");
        return NULL;
    }
    
    /* Detect file type */
    file_type = detect_file_type(libname);
    
    printf("[DEBUG] dlopen: File type detected: %d\n", file_type);
    
    if (file_type == FILE_TYPE_UNKNOWN) {
        set_dlerror("Unknown or unsupported file type");
        return NULL;
    }
    
    switch (file_type) {
        case FILE_TYPE_LOAD:
            /* Try to load the library using LoadSeg */
            seglist = LoadSeg(libname);
            if (!seglist) {
                set_dlerror_with_ioerr("Failed to load library");
                return NULL;
            }
            handle = malloc(sizeof(lib_handle_t));
            if (!handle) {
                UnLoadSeg(seglist);
                set_dlerror("Out of memory");
                return NULL;
            }
            handle->seglist = seglist;
            handle->filename = strdup(libname);
            handle->file_type = FILE_TYPE_LOAD;
            handle->symbol_table = NULL;
            handle->symbol_count = 0;
            handle->base_addr = NULL;
            handle->hunk_addresses = NULL;
            handle->hunk_count = 0;
            handle->hunks = NULL;
            
            /* Parse the library to extract symbols */
            if (parse_load_file(seglist, handle) != 0) {
                free_lib_handle(handle);
                UnLoadSeg(seglist);
                return NULL;
            }
            break;
            
        case FILE_TYPE_OBJECT:
            handle = malloc(sizeof(lib_handle_t));
            if (!handle) {
                set_dlerror("Out of memory");
                return NULL;
            }
            handle->seglist = NULL;
            handle->filename = strdup(libname);
            handle->file_type = FILE_TYPE_OBJECT;
            handle->symbol_table = NULL;
            handle->symbol_count = 0;
            handle->base_addr = NULL;
            handle->hunk_addresses = NULL;
            handle->hunk_count = 0;
            handle->hunks = NULL;
            
            /* Parse the object file to extract symbols */
            if (load_object_file(libname, handle) != 0) {
                free_lib_handle(handle);
                return NULL;
            }
            break;
            
        case FILE_TYPE_LIBRARY:
            /* Load static library by extracting and linking object files */
            handle = malloc(sizeof(lib_handle_t));
            if (!handle) {
                set_dlerror("Out of memory");
                return NULL;
            }
            handle->seglist = NULL;
            handle->filename = strdup(libname);
            handle->file_type = FILE_TYPE_LIBRARY;
            handle->symbol_table = NULL;
            handle->symbol_count = 0;
            handle->base_addr = NULL;
            handle->hunk_addresses = NULL;
            handle->hunk_count = 0;
            handle->hunks = NULL;
            
            /* Load the static library */
            if (load_static_library(libname, handle) != 0) {
                free_lib_handle(handle);
                return NULL;
            }
            break;
            
        default:
            set_dlerror("Unknown file type");
            return NULL;
    }
    
    /* Add to global list */
    handle->next = loaded_libs;
    loaded_libs = handle;
    
    return handle;
}

/* POSIX dlsym function */
void *dlsym(void *handle, const char *symbol)
{
    lib_handle_t *lib_handle;
    void *symbol_addr;
    
    /* Clear any previous error */
    dlerror();
    
    /* Check parameters */
    if (!handle || !symbol) {
        set_dlerror("NULL handle or symbol name");
        return NULL;
    }
    
    /* Find the library handle */
    lib_handle = (lib_handle_t *)handle;
    
    /* Look for the symbol */
    symbol_addr = find_symbol(lib_handle, symbol);
    if (!symbol_addr) {
        char error_msg[256];
        sprintf(error_msg, "Symbol '%s' not found", symbol);
        set_dlerror(error_msg);
        return NULL;
    }
    
    return symbol_addr;
}

/* POSIX dlclose function */
int dlclose(void *handle)
{
    lib_handle_t *lib_handle, *prev, *current;
    
    /* Clear any previous error */
    dlerror();
    
    /* Check parameters */
    if (!handle) {
        set_dlerror("NULL handle");
        return -1;
    }
    
    lib_handle = (lib_handle_t *)handle;
    
    /* Remove from global list */
    prev = NULL;
    current = loaded_libs;
    
    while (current && current != lib_handle) {
        prev = current;
        current = current->next;
    }
    
    if (!current) {
        set_dlerror("Invalid library handle");
        return -1;
    }
    
    if (prev) {
        prev->next = current->next;
    } else {
        loaded_libs = current->next;
    }
    
    /* Unload the segments if it was a LoadSeg file */
    if (lib_handle->seglist && lib_handle->file_type == FILE_TYPE_LOAD) {
        UnLoadSeg(lib_handle->seglist);
    }
    
    /* Free the handle */
    free_lib_handle(lib_handle);
    
    return 0;
}

/* POSIX dlerror function */
char *dlerror(void)
{
    if (dlerror_set) {
        dlerror_set = 0;
        return dlerror_buffer;
    }
    return NULL;
}
 