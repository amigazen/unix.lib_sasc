/*
 * Amiga-Specific Termcap Implementation
 * Keymap system integration
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "amiga_termcap_private.h"
#include <exec/exec.h>
#include <devices/keymap.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/console.h>
#include <proto/exec.h>
#include <proto/keymap.h>
#include <proto/dos.h>
#include <proto/console.h>

extern struct Library *ConsoleDevice;

/* Console device constants - defined manually for SAS/C compatibility */
#ifndef CMD_NONSTD
#define CMD_NONSTD 9
#endif

/* Input event qualifier constants - defined manually for SAS/C compatibility */
#ifndef IEQUALIFIER_LCTRL
#define IEQUALIFIER_LCTRL 0x0001
#endif
#ifndef IEQUALIFIER_RCTRL
#define IEQUALIFIER_RCTRL 0x0002
#endif
#ifndef CD_SETCURSOR
#define CD_SETCURSOR (CMD_NONSTD+7)
#endif
#ifndef CD_CLEAR
#define CD_CLEAR (CMD_NONSTD+11)
#endif
#ifndef CD_SETATTRS
#define CD_SETATTRS (CMD_NONSTD+9)
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

#endif /* _CONSOLE_STRUCTS_DEFINED */

/* External libraries */
extern struct ExecBase *SysBase;
extern struct Library *KeymapBase;

/*
 * amiga_keymap_init - initialize keymap system for termcap
 * 
 * This function initializes the keymap system for termcap operations.
 */
int
amiga_keymap_init(struct amiga_termcap *termcap)
{
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    
    /* Open keymap library if not already open */
    if (!KeymapBase) {
        KeymapBase = OpenLibrary("keymap.library", 0);
        if (!KeymapBase) {
            return AMIGA_TERMCAP_ERROR_DEVICE;
        }
    }
    
    /* Get default keymap */
    termcap->keymap = NULL;  /* Use default keymap via RawKeyConvert */
    if (!termcap->keymap) {
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_keymap_cleanup - cleanup keymap system
 * 
 * This function cleans up the keymap system resources.
 */
void
amiga_keymap_cleanup(struct amiga_termcap *termcap)
{
    if (!termcap) {
        return;
    }
    
    
    /* Note: We don't free the keymap as it's managed by the system */
    termcap->keymap = NULL;
    
    /* Close keymap library if we opened it */
    if (KeymapBase) {
        CloseLibrary(KeymapBase);
        KeymapBase = NULL;
    }
}

/*
 * amiga_keymap_convert - convert raw key input to string
 * 
 * This function converts raw key input from the input device to
 * a string representation using the keymap system.
 */
int
amiga_keymap_convert(struct amiga_termcap *termcap, UWORD qual, UWORD code, 
                     char *buffer, int buflen)
{
    struct InputEvent ie;
    char *result;
    int len;
    
    if (!termcap || !buffer || buflen <= 0) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    /* Set up input event */
    ie.ie_Class = IECLASS_RAWKEY;
    ie.ie_SubClass = 0;
    ie.ie_Code = code;
    ie.ie_Qualifier = qual;
    ie.ie_EventAddress = NULL;
    
    /* Convert using default keymap (NULL = use default console device keymap) */
    result = (char *)RawKeyConvert(&ie, buffer, buflen, (struct KeyMap *)NULL);
    if (!result) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    /* Calculate length of result */
    len = amiga_termcap_strlen(result);
    
    return len;
}

/*
 * amiga_keymap_get_default - get default keymap
 * 
 * This function gets the default system keymap.
 */
int
amiga_keymap_get_default(struct amiga_termcap *termcap)
{
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    termcap->keymap = NULL;  /* Use default keymap via RawKeyConvert */
    if (!termcap->keymap) {
        return AMIGA_TERMCAP_ERROR_DEVICE;
    }
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_keymap_set_current - set current keymap
 * 
 * This function sets the current keymap for termcap operations.
 */
int
amiga_keymap_set_current(struct amiga_termcap *termcap, struct KeyMap *keymap)
{
    if (!termcap || !keymap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    termcap->keymap = keymap;
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_keymap_qual_to_string - convert qualifier to string
 * 
 * This function converts key qualifiers to a human-readable string.
 */
int
amiga_keymap_qual_to_string(UWORD qual, char *buffer, int buflen)
{
    int len = 0;
    
    if (!buffer || buflen <= 0) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    buffer[0] = '\0';
    
    if (qual & IEQUALIFIER_LSHIFT) {
        if (len < buflen - 1) {
            buffer[len++] = 'L';
        }
    }
    if (qual & IEQUALIFIER_RSHIFT) {
        if (len < buflen - 1) {
            buffer[len++] = 'R';
        }
    }
    if (qual & IEQUALIFIER_LALT) {
        if (len < buflen - 1) {
            buffer[len++] = 'A';
        }
    }
    if (qual & IEQUALIFIER_RALT) {
        if (len < buflen - 1) {
            buffer[len++] = 'a';
        }
    }
    if (qual & IEQUALIFIER_LCOMMAND) {
        if (len < buflen - 1) {
            buffer[len++] = 'C';
        }
    }
    if (qual & IEQUALIFIER_RCOMMAND) {
        if (len < buflen - 1) {
            buffer[len++] = 'c';
        }
    }
    if (qual & IEQUALIFIER_LCTRL) {
        if (len < buflen - 1) {
            buffer[len++] = 'T';
        }
    }
    if (qual & IEQUALIFIER_RCTRL) {
        if (len < buflen - 1) {
            buffer[len++] = 't';
        }
    }
    
    buffer[len] = '\0';
    return len;
}

/*
 * amiga_termcap_get_key - get a key from the console
 * 
 * This function gets a key from the console, converting it through
 * the keymap system.
 */
int
amiga_termcap_get_key(struct amiga_termcap *termcap, char *key, int maxlen)
{
    struct InputEvent ie;
    int len;
    
    if (!termcap || !key || maxlen <= 0) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    /* Read raw key from input device */
    if (amiga_console_read(termcap, (char *)&ie, sizeof(struct InputEvent)) <= 0) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    /* Check if it's a raw key event */
    if (ie.ie_Class != IECLASS_RAWKEY) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    /* Convert through keymap */
    len = amiga_keymap_convert(termcap, ie.ie_Qualifier, ie.ie_Code, key, maxlen);
    if (len < 0) {
        return len;
    }
    
    return len;
}

/*
 * amiga_termcap_set_mode - set console mode
 * 
 * This function sets the console mode (raw, cooked, etc.).
 */
int
amiga_termcap_set_mode(struct amiga_termcap *termcap, int mode)
{
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
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

/*
 * amiga_termcap_get_size - get terminal size
 * 
 * This function gets the current terminal size.
 */
int
amiga_termcap_get_size(struct amiga_termcap *termcap, int *width, int *height)
{
    if (!termcap || !width || !height) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    *width = termcap->width;
    *height = termcap->height;
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_termcap_set_cursor - set cursor position
 * 
 * This function sets the cursor position on the console.
 */
int
amiga_termcap_set_cursor(struct amiga_termcap *termcap, int x, int y)
{
    struct ConsoleCursor cursor;
    
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    cursor.x = x;
    cursor.y = y;
    
    if (amiga_console_ioctl(termcap, CD_SETCURSOR, &cursor) != AMIGA_TERMCAP_ERROR_NONE) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    termcap->cursor_x = x;
    termcap->cursor_y = y;
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_termcap_clear_screen - clear the screen
 * 
 * This function clears the console screen.
 */
int
amiga_termcap_clear_screen(struct amiga_termcap *termcap)
{
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    if (amiga_console_ioctl(termcap, CD_CLEAR, NULL) != AMIGA_TERMCAP_ERROR_NONE) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    /* Reset cursor position */
    termcap->cursor_x = 0;
    termcap->cursor_y = 0;
    
    return AMIGA_TERMCAP_ERROR_NONE;
}

/*
 * amiga_termcap_set_attrs - set text attributes
 * 
 * This function sets the text attributes on the console.
 */
int
amiga_termcap_set_attrs(struct amiga_termcap *termcap, UWORD attrs)
{
    struct ConsoleAttribute console_attrs;
    
    if (!termcap) {
        return AMIGA_TERMCAP_ERROR_INVALID;
    }
    
    console_attrs.attr = attrs;
    console_attrs.fg = termcap->text_fg;
    console_attrs.bg = termcap->text_bg;
    
    if (amiga_console_ioctl(termcap, CD_SETATTRS, &console_attrs) != AMIGA_TERMCAP_ERROR_NONE) {
        return AMIGA_TERMCAP_ERROR_IO;
    }
    
    termcap->text_attr = attrs;
    
    return AMIGA_TERMCAP_ERROR_NONE;
}
