/*
 * Amiga-Specific Termcap Implementation
 * Console device integration
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include <exec/exec.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <devices/console.h>
#include <devices/input.h>
#include <intuition/intuition.h>
#include <proto/dos.h>
#include <proto/console.h>
#include <proto/exec.h>
#include <proto/intuition.h>

/* Console device constants - defined manually for SAS/C compatibility */
#ifndef CMD_NONSTD
#define CMD_NONSTD 9
#endif
#ifndef CD_SETCONSOLE
#define CD_SETCONSOLE (CMD_NONSTD+6)
#endif
#ifndef CD_SETCURSOR
#define CD_SETCURSOR (CMD_NONSTD+7)
#endif
#ifndef CD_GETCURSOR
#define CD_GETCURSOR (CMD_NONSTD+8)
#endif
#ifndef CD_SETATTRS
#define CD_SETATTRS (CMD_NONSTD+9)
#endif
#ifndef CD_GETATTRS
#define CD_GETATTRS (CMD_NONSTD+10)
#endif
#ifndef CD_CLEAR
#define CD_CLEAR (CMD_NONSTD+11)
#endif
#ifndef CD_CLEARBLOCK
#define CD_CLEARBLOCK (CMD_NONSTD+12)
#endif

/* Console structures - defined manually for SAS/C compatibility */
#ifndef _CONSOLE_STRUCTS_DEFINED
#define _CONSOLE_STRUCTS_DEFINED

struct ConsoleCursor {
    short x;
    short y;
};

struct ConsoleAttribute {
    short attr;
    short fg;
    short bg;
};

struct ConsoleBlock {
    short x1, y1;
    short x2, y2;
};

#endif /* _CONSOLE_STRUCTS_DEFINED */
/* External libraries */
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern struct IntuitionBase *IntuitionBase;

/*
 * amiga_console_open - open console device for termcap operations
 * 
 * This function opens the console device and associates it with
 * the provided intuition window for termcap operations.
 */
int
amiga_console_open(struct amiga_termcap *termcap, struct Window *window)
{
    struct IOStdReq *con_io, *input_io;
    
    if (!termcap || !window) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    
    /* Allocate I/O requests */
    con_io = (struct IOStdReq *)CreateIORequest(NULL, sizeof(struct IOStdReq));
    if (!con_io) {
        return AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    input_io = (struct IOStdReq *)CreateIORequest(NULL, sizeof(struct IOStdReq));
    if (!input_io) {
        DeleteIORequest((struct IORequest *)con_io);
        return AMIGA_TERMCAP_ERROR_MEMORY;
    }
    
    /* Open console device */
    if (OpenDevice("console.device", 0, (struct IORequest *)con_io, 0) != 0) {
        DeleteIORequest((struct IORequest *)con_io);
        DeleteIORequest((struct IORequest *)input_io);
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    /* Open input device */
    if (OpenDevice("input.device", 0, (struct IORequest *)input_io, 0) != 0) {
        CloseDevice((struct IORequest *)con_io);
        DeleteIORequest((struct IORequest *)con_io);
        DeleteIORequest((struct IORequest *)input_io);
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    /* Set up console for the window */
    con_io->io_Data = (APTR)window;
    con_io->io_Length = sizeof(struct Window);
    con_io->io_Command = CD_SETCONSOLE;
    
    if (DoIO((struct IORequest *)con_io) != 0) {
        CloseDevice((struct IORequest *)con_io);
        CloseDevice((struct IORequest *)input_io);
        DeleteIORequest((struct IORequest *)con_io);
        DeleteIORequest((struct IORequest *)input_io);
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    /* Store references */
    termcap->con_io = con_io;
    termcap->input_io = input_io;
    termcap->window = window;
    
    /* Initialize console attributes */
    termcap->text_attr = 0;
    termcap->text_fg = 1;  /* Default foreground color */
    termcap->text_bg = 0;  /* Default background color */
    termcap->cursor_x = 0;
    termcap->cursor_y = 0;
    termcap->raw_mode = 0;      /* Start in cooked mode (CON-Handler mode 0) */
    termcap->echo_mode = 1;     /* Echo enabled by default */
    termcap->buffer_mode = 0;   /* CON-Handler buffer mode 0 (cooked) */
    
    /* Get window dimensions */
    if (window) {
        termcap->width = window->Width / 8;   /* Approximate character width */
        termcap->height = window->Height / 8; /* Approximate character height */
    } else {
        termcap->width = 80;   /* Default width */
        termcap->height = 25;  /* Default height */
    }
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_console_close - close console device
 * 
 * This function closes the console device and frees associated resources.
 */
void
amiga_console_close(struct amiga_termcap *termcap)
{
    if (!termcap) {
        return;
    }
    
    
    /* Close devices */
    if (termcap->con_io) {
        CloseDevice((struct IORequest *)termcap->con_io);
        DeleteIORequest((struct IORequest *)termcap->con_io);
        termcap->con_io = NULL;
    }
    
    if (termcap->input_io) {
        CloseDevice((struct IORequest *)termcap->input_io);
        DeleteIORequest((struct IORequest *)termcap->input_io);
        termcap->input_io = NULL;
    }
    
    termcap->window = NULL;
}

/*
 * amiga_console_write - write data to console
 * 
 * This function writes data to the console device.
 */
int
amiga_console_write(struct amiga_termcap *termcap, const char *data, int len)
{
    struct IOStdReq *con_io;
    
    if (!termcap || !data || len <= 0) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    con_io = termcap->con_io;
    if (!con_io) {
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    /* Set up I/O request */
    con_io->io_Data = (APTR)data;
    con_io->io_Length = len;
    con_io->io_Command = CMD_WRITE;
    con_io->io_Flags = IOF_QUICK;
    
    /* Execute I/O */
    if (DoIO((struct IORequest *)con_io) != 0) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    return (int)con_io->io_Actual;
}

/*
 * amiga_console_read - read data from console
 * 
 * This function reads data from the console device.
 */
int
amiga_console_read(struct amiga_termcap *termcap, char *data, int len)
{
    struct IOStdReq *con_io;
    
    if (!termcap || !data || len <= 0) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    con_io = termcap->con_io;
    if (!con_io) {
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    /* Set up I/O request */
    con_io->io_Data = (APTR)data;
    con_io->io_Length = len;
    con_io->io_Command = CMD_READ;
    con_io->io_Flags = IOF_QUICK;
    
    /* Execute I/O */
    if (DoIO((struct IORequest *)con_io) != 0) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    return (int)con_io->io_Actual;
}

/*
 * amiga_console_ioctl - perform console I/O control operations
 * 
 * This function performs various console control operations.
 */
int
amiga_console_ioctl(struct amiga_termcap *termcap, int cmd, void *arg)
{
    struct IOStdReq *con_io;
    
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    con_io = termcap->con_io;
    if (!con_io) {
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    switch (cmd) {
    case CD_SETCURSOR:
        /* Set cursor position */
        if (arg) {
            struct ConsoleCursor *cursor = (struct ConsoleCursor *)arg;
            con_io->io_Data = (APTR)cursor;
            con_io->io_Length = sizeof(struct ConsoleCursor);
            con_io->io_Command = CD_SETCURSOR;
            
            if (DoIO((struct IORequest *)con_io) != 0) {
                return AMIGA_TERMCAP_ERROR_IO;
            }
            
            termcap->cursor_x = cursor->x;
            termcap->cursor_y = cursor->y;
        }
        break;
        
    case CD_GETCURSOR:
        /* Get cursor position */
        if (arg) {
            struct ConsoleCursor *cursor = (struct ConsoleCursor *)arg;
            con_io->io_Data = (APTR)cursor;
            con_io->io_Length = sizeof(struct ConsoleCursor);
            con_io->io_Command = CD_GETCURSOR;
            
            if (DoIO((struct IORequest *)con_io) != 0) {
                return AMIGA_TERMCAP_ERROR_IO;
            }
        }
        break;
        
    case CD_SETATTRS:
        /* Set text attributes */
        if (arg) {
            struct ConsoleAttribute *new_attrs = (struct ConsoleAttribute *)arg;
            con_io->io_Data = (APTR)new_attrs;
            con_io->io_Length = sizeof(struct ConsoleAttribute);
            con_io->io_Command = CD_SETATTRS;
            
            if (DoIO((struct IORequest *)con_io) != 0) {
                return AMIGA_TERMCAP_ERROR_IO;
            }
            
            termcap->text_attr = new_attrs->attr;
            termcap->text_fg = new_attrs->fg;
            termcap->text_bg = new_attrs->bg;
        }
        break;
        
    case CD_GETATTRS:
        /* Get text attributes */
        if (arg) {
            struct ConsoleAttribute *attrs = (struct ConsoleAttribute *)arg;
            con_io->io_Data = (APTR)attrs;
            con_io->io_Length = sizeof(struct ConsoleAttribute);
            con_io->io_Command = CD_GETATTRS;
            
            if (DoIO((struct IORequest *)con_io) != 0) {
                return AMIGA_TERMCAP_ERROR_IO;
            }
        }
        break;
        
    case CD_CLEAR:
        /* Clear screen */
        con_io->io_Command = CD_CLEAR;
        if (DoIO((struct IORequest *)con_io) != 0) {
            return AMIGA_TERMCAP_ERROR_IO;
        }
        break;
        
    case CD_CLEARBLOCK:
        /* Clear block */
        if (arg) {
            struct ConsoleBlock *block = (struct ConsoleBlock *)arg;
            con_io->io_Data = (APTR)block;
            con_io->io_Length = sizeof(struct ConsoleBlock);
            con_io->io_Command = CD_CLEARBLOCK;
            
            if (DoIO((struct IORequest *)con_io) != 0) {
                return AMIGA_TERMCAP_ERROR_IO;
            }
        }
        break;
        
    default:
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_termcap_init - initialize Amiga termcap structure
 * 
 * This function initializes an Amiga termcap structure and opens
 * the associated console device.
 */
struct amiga_termcap *
amiga_termcap_init(struct Window *window)
{
    struct amiga_termcap *termcap;
    int result;
    
    
    /* Allocate termcap structure */
    termcap = amiga_termcap_malloc(sizeof(struct amiga_termcap));
    if (!termcap) {
        return NULL;
    }
    
    /* Initialize structure */
    memset(termcap, 0, sizeof(struct amiga_termcap));
    
    /* Allocate working buffer */
    termcap->buffer = amiga_termcap_malloc(AMIGA_TERMCAP_BUFFER_SIZE);
    if (!termcap->buffer) {
        amiga_termcap_free(termcap);
        return NULL;
    }
    termcap->buffer_size = AMIGA_TERMCAP_BUFFER_SIZE;
    
    /* Open console device */
    result = amiga_console_open(termcap, window);
    if (result != AMIGA_TERMCAP_ERROR_NONE) {
        amiga_termcap_free(termcap->buffer);
        amiga_termcap_free(termcap);
        return NULL;
    }
    
    /* Initialize keymap */
    result = amiga_keymap_init(termcap);
    if (result != AMIGA_TERMCAP_ERROR_NONE) {
        amiga_console_close(termcap);
        amiga_termcap_free(termcap->buffer);
        amiga_termcap_free(termcap);
        return NULL;
    }
    
    return termcap;
}

/*
 * amiga_termcap_cleanup - cleanup Amiga termcap structure
 * 
 * This function cleans up an Amiga termcap structure and closes
 * associated devices.
 */
void
amiga_termcap_cleanup(struct amiga_termcap *termcap)
{
    if (!termcap) {
        return;
    }
    
    
    /* Cleanup keymap */
    amiga_keymap_cleanup(termcap);
    
    /* Close console device */
    amiga_console_close(termcap);
    
    /* Free working buffer */
    if (termcap->buffer) {
        amiga_termcap_free(termcap->buffer);
    }
    
    /* Free termcap structure */
    amiga_termcap_free(termcap);
}

/*
 * amiga_console_set_buffer_mode - set CON-Handler buffer mode
 * 
 * Mode 0: Cooked mode (line buffering, echo, line editing)
 * Mode 1: Raw mode (character-by-character input)
 * Mode 2: Medium mode (for shells with history/TAB expansion)
 */
int
amiga_console_set_buffer_mode(struct amiga_termcap *termcap, int mode)
{
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    if (mode < 0 || mode > 2) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    /* Update buffer mode */
    termcap->buffer_mode = mode;
    
    /* Set corresponding raw/echo modes */
    switch (mode) {
    case 0:  /* Cooked mode */
        termcap->raw_mode = 0;
        termcap->echo_mode = 1;
        break;
    case 1:  /* Raw mode */
        termcap->raw_mode = 1;
        termcap->echo_mode = 0;
        break;
    case 2:  /* Medium mode */
        termcap->raw_mode = 0;
        termcap->echo_mode = 1;
        break;
    default:
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    return AMIGA_TERMCAP_ERROR_NONE;
}
