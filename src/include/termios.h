/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * termios.h - Terminal I/O
 * 
 * This header provides terminal I/O functions for Amiga
 * 
 * Copyright (C) 2025 by amigazen project
 */

#ifndef _TERMIOS_H
#define _TERMIOS_H 1

#include <sys/types.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Terminal I/O control structure */
struct termios {
    tcflag_t c_iflag;    /* Input modes */
    tcflag_t c_oflag;    /* Output modes */
    tcflag_t c_cflag;    /* Control modes */
    tcflag_t c_lflag;    /* Local modes */
    cc_t c_cc[NCCS];     /* Control characters */
    speed_t c_ispeed;    /* Input speed */
    speed_t c_ospeed;    /* Output speed */
};

/* Terminal control characters */
#define NCCS 32

/* Input modes */
#define IGNBRK 0x00000001    /* Ignore BREAK condition */
#define BRKINT 0x00000002    /* BREAK generates SIGINT */
#define IGNPAR 0x00000004    /* Ignore characters with parity errors */
#define PARMRK 0x00000008    /* Mark parity and framing errors */
#define INPCK  0x00000010    /* Enable input parity check */
#define ISTRIP 0x00000020    /* Strip 8th bit off characters */
#define INLCR  0x00000040    /* Map NL to CR on input */
#define IGNCR  0x00000080    /* Ignore CR */
#define ICRNL  0x00000100    /* Map CR to NL on input */
#define IUCLC  0x00000200    /* Map uppercase to lowercase on input */
#define IXON   0x00000400    /* Enable XON/XOFF flow control on output */
#define IXANY  0x00000800    /* Any character will restart stopped output */
#define IXOFF  0x00001000    /* Enable XON/XOFF flow control on input */
#define IMAXBEL 0x00002000   /* Ring bell when input queue is full */

/* Output modes */
#define OPOST  0x00000001    /* Post-process output */
#define OLCUC  0x00000002    /* Map lowercase to uppercase on output */
#define ONLCR  0x00000004    /* Map NL to CR-NL on output */
#define OCRNL  0x00000008    /* Map CR to NL on output */
#define ONOCR  0x00000010    /* No CR output at column 0 */
#define ONLRET 0x00000020    /* NL performs CR function */
#define OFILL  0x00000040    /* Use fill characters for delay */
#define OFDEL  0x00000080    /* Fill is DEL, else NUL */
#define NLDLY  0x00000300    /* Select newline delays */
#define NL0    0x00000000    /* Newline character type 0 */
#define NL1    0x00000100    /* Newline character type 1 */
#define NL2    0x00000200    /* Newline character type 2 */
#define NL3    0x00000300    /* Newline character type 3 */
#define CRDLY  0x00003000    /* Select carriage-return delays */
#define CR0    0x00000000    /* Carriage-return delay type 0 */
#define CR1    0x00001000    /* Carriage-return delay type 1 */
#define CR2    0x00002000    /* Carriage-return delay type 2 */
#define CR3    0x00003000    /* Carriage-return delay type 3 */
#define TABDLY 0x0000c000    /* Select horizontal-tab delays */
#define TAB0   0x00000000    /* Horizontal-tab delay type 0 */
#define TAB1   0x00004000    /* Horizontal-tab delay type 1 */
#define TAB2   0x00008000    /* Horizontal-tab delay type 2 */
#define TAB3   0x0000c000    /* Horizontal-tab delay type 3 */
#define BSDLY  0x00010000    /* Select backspace delays */
#define BS0    0x00000000    /* Backspace delay type 0 */
#define BS1    0x00010000    /* Backspace delay type 1 */
#define VTDLY  0x00020000    /* Select vertical-tab delays */
#define VT0    0x00000000    /* Vertical-tab delay type 0 */
#define VT1    0x00020000    /* Vertical-tab delay type 1 */
#define FFDLY  0x00040000    /* Select form-feed delays */
#define FF0    0x00000000    /* Form-feed delay type 0 */
#define FF1    0x00040000    /* Form-feed delay type 1 */

/* Control modes */
#define CSIZE  0x00000300    /* Character size mask */
#define CS5    0x00000000    /* 5 bits */
#define CS6    0x00000100    /* 6 bits */
#define CS7    0x00000200    /* 7 bits */
#define CS8    0x00000300    /* 8 bits */
#define CSTOPB 0x00000400    /* Send 2 stop bits, else 1 */
#define CREAD  0x00000800    /* Enable receiver */
#define PARENB 0x00001000    /* Parity enable */
#define PARODD 0x00002000    /* Odd parity, else even */
#define HUPCL  0x00004000    /* Hang up on last close */
#define CLOCAL 0x00008000    /* Ignore modem status lines */
#define CBAUD  0x000f0000    /* Baud rate mask */
#define B0     0x00000000    /* Hang up */
#define B50    0x00010000    /* 50 baud */
#define B75    0x00020000    /* 75 baud */
#define B110   0x00030000    /* 110 baud */
#define B134   0x00040000    /* 134.5 baud */
#define B150   0x00050000    /* 150 baud */
#define B200   0x00060000    /* 200 baud */
#define B300   0x00070000    /* 300 baud */
#define B600   0x00080000    /* 600 baud */
#define B1200  0x00090000    /* 1200 baud */
#define B1800  0x000a0000    /* 1800 baud */
#define B2400  0x000b0000    /* 2400 baud */
#define B4800  0x000c0000    /* 4800 baud */
#define B9600  0x000d0000    /* 9600 baud */
#define B19200 0x000e0000    /* 19200 baud */
#define B38400 0x000f0000    /* 38400 baud */
#define EXTA   B19200
#define EXTB   B38400

/* Local modes */
#define ISIG   0x00000001    /* Enable signals */
#define ICANON 0x00000002    /* Canonical input (erase and kill processing) */
#define XCASE  0x00000004    /* Canonical upper/lower presentation */
#define ECHO   0x00000008    /* Enable echo */
#define ECHOE  0x00000010    /* Echo ERASE as backspace */
#define ECHOK  0x00000020    /* Echo KILL by erasing line */
#define ECHONL 0x00000040    /* Echo NL */
#define ECHOCTL 0x00000080   /* Echo control characters as ^X */
#define ECHOPRT 0x00000100   /* Echo erased character as \ */
#define ECHOKE 0x00000200    /* Visual erase for KILL */
#define FLUSHO 0x00000400    /* Output being flushed */
#define PENDIN 0x00000800    /* Retype pending input */
#define IEXTEN 0x00001000    /* Extended input processing */
#define TOSTOP 0x00002000    /* Send SIGTTOU for background output */
#define NOFLSH 0x00004000    /* Don't flush after interrupt or quit */

/* Control characters */
#define VEOF    0    /* EOF character */
#define VEOL    1    /* EOL character */
#define VEOL2   2    /* EOL2 character */
#define VERASE  3    /* ERASE character */
#define VWERASE 4    /* WERASE character */
#define VKILL   5    /* KILL character */
#define VREPRINT 6   /* REPRINT character */
#define VINTR   7    /* INTR character */
#define VQUIT   8    /* QUIT character */
#define VSUSP   9    /* SUSP character */
#define VDSUSP  10   /* DSUSP character */
#define VSTART  11   /* START character */
#define VSTOP   12   /* STOP character */
#define VLNEXT  13   /* LNEXT character */
#define VDISCARD 14  /* DISCARD character */
#define VMIN    15   /* MIN value */
#define VTIME   16   /* TIME value */
#define VSTATUS 17   /* STATUS character */

/* Special characters */
#define _POSIX_VDISABLE 0

/* Function prototypes */
speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);
int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);
int tcdrain(int fildes);
int tcflow(int fildes, int action);
int tcflush(int fildes, int queue_selector);
int tcgetattr(int fildes, struct termios *termios_p);
pid_t tcgetsid(int fildes);
int tcsendbreak(int fildes, int duration);
int tcsetattr(int fildes, int optional_actions, const struct termios *termios_p);

/* Queue selectors */
#define TCOOFF 0    /* Suspend output */
#define TCOON  1    /* Restart suspended output */
#define TCIOFF 2    /* Transmit a STOP character */
#define TCION  3    /* Transmit a START character */

/* Queue selectors for tcflush */
#define TCIFLUSH  0 /* Flush pending input */
#define TCOFLUSH  1 /* Flush pending output */
#define TCIOFLUSH 2 /* Flush both pending input and output */

/* Optional actions for tcsetattr */
#define TCSANOW   0 /* Change attributes immediately */
#define TCSADRAIN 1 /* Change attributes when output has drained */
#define TCSAFLUSH 2 /* Change attributes when output has drained and flush pending input */

/* Type definitions */
typedef unsigned int tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int speed_t;

#ifdef __cplusplus
}
#endif

#endif /* _TERMIOS_H */
