# UniLib3

This is UniLib3, a POSIX and C99 compatible standard C library for SAS/C and DICE compilers on Amiga.

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** uses modern software development tools and methods to update and rerelease classic Amiga open source software. Releases include a new AWeb, this new Amiga Python 2, and the ToolKit project - a universal SDK for Amiga.

Key to the amigazen project approach is ensuring every project can be built with the same common set of development tools and configurations, so the ToolKit project was created to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

The amigazen project philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all of amigazen projects are gratefully received at [GitHub](https://github.com/amigazen/). While the focus now is on classic 68k software, it is intended that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

The original authors of the *unix.lib* software are not affiliated with the amigazen project. This software is redistributed on terms described in the documentation, particularly the file LICENSE.md

## About UniLib3

UniLib3 is a comprehensive POSIX and C99 compatible standard C library implementation for Amiga, designed specifically for SAS/C and DICE compilers. This project aims to provide Amiga developers with a modern, standards-compliant C library that bridges the gap between classic Amiga development and contemporary POSIX, and thus code portability, standards.

The name UniLib3 reflects the fact this is a universal set of libraries incorporating not just an updated unix.lib but also psockets.lib, curses.lib and more to come, whilst the version '3' indicates that this is the third version of unix.lib after David and Enrico's versions (see unix.lib History below) but also makes it clear this library is intended for version 3 of the Amiga operating system.

### unix.lib History

UniLib3 represents the third major evolution of Unix-compatibility libraries for Amiga:

**Version 1 (Sometime prior to 1995)**: The original unix.lib was created and placed into the public domain by David Gay specifically to support his port of GNU Emacs to Amiga. This first version provided essential Unix-compatibility functions needed for Emacs to run on Amiga, focusing on the specific requirements of that port.

**Version ?** - A unix.lib of unknown provenance was also included with the Inet225 TCP/IP stack but the source code for this has not been made public. 

**Version 2 (1996)**: A second version was further developed by Enrico Forestieri who expanded the library to support porting various x11 programs to Amiga. This version added many new functions as well as bsdsocket.library support.

**UniLib3 (2025)**: This current version by amigazen project presents a complete refactor and expansion, incorporating not only the lessons learned from previous versions but also significant contributions merged in from Irmen de Jong's Amiga Python 2 implementation and other BSD license-compatible projects. UniLib3 aims to be a comprehensive, POSIX and C99 standards-compliant solution pulling together all of the Unix-compatibility open source projects for Amiga from over the years into one unified library, excluding GPL/GNU projects for licensing reasons.

### Key Features

- **POSIX compliance**: Implements core POSIX.1 functionality for file operations, process management, and system interfaces (except _fork()_)
- **C99 support**: Full C99 standard library implementation including stdio, stdlib, string, and (soon) math functions
- **Amiga Integration**: Uses native Amiga library functions wherever suitable as the underlying implementation including memory pools, _utility.library_ and _locale.library_, falling back to builtin versions for edge cases not handled by the native implementation e.g. _snprintf()_ supports all format string tokens, using _SNPrintf()_ for most calls but its own fallback implementation for tokens not supported by _RawDoFmt()_
- **NOT backwards compatible**: Deliberately does NOT support version 1.x and 2.x operating systems
- **Latest NDK support**: Built against the latest official NDK
- **Builds out of the box**: Builds cleanly for anyone using the ToolKit configuration for Amiga development
- **Compiler support**: Primary support for SAS/C with planned DICE compiler compatibility. Probably also compatible with VBCC (but you can use PosixLib there)
- **Memory Management**: Efficient memory allocation and management optimized for Amiga hardware constraints utilizing native memory management features such as memory pools
- **unix.lib**: Newly expanded and updated POSIX and C99 compliant standard C library originally by David Gay and Enrico Forestieri featuring dozens of new and updated functions including: -
- *longlong_t*: A complete 32-bit compatible implementation of the 64-bit longlong_t type
- *String functions*: New string functions including memory safe implementations of the snprintf family
- *Filesystem permissions*: file permission and ownership functions talk to the FileSystem directly in case it DOES support multiuser features
- *getopts functions*: Complete BSD compatible implementations of getopts 
- *POSIX compliant*: 
- *New functions*: Amiga native versions of many new POSIX functions including alloca(), ustat(), utime(), gettimeofday() and many more
- *Inet225 unix.lib*: Reimplements new versions of the POSIX functions provided by the version of unix.lib that came with Inet225
- *AmiTCP netlib*: Reimplements new versions of the POSIX functions provided by AmiTCP's netlib - rcmd(), herror() and the syslog API wrapping _bsdsocket.library_
- *librt APIs*: A brand new implementation of librt functions starting with POSIX mq_* message queues built on top of message ports, realtime.library and timer.device
- *libiconv APIs*: A brand new implementation of libiconv functions integrated with _locale.library_, with basic support for Latin1 and UTF8 codesets to start with
- *Unit tests*: Unit tests for many of the new functions, aiming to reach full test coverage in time
- **curses.lib**: Updated version of Simon Raybould's Amiga port of _curses_ now BSD licensed
- **psockets.lib**: A brand new Amiga port of _psockets_ wrapping _bsdsocket.library_
- **pthread.lib**: A brand new Amiga native implementation of _pthread_
- **Full set of POSIX libraries**: For full POSIX compatibility regex, libdl and libiconv are also needed... watch this space!
- **Pipes support**: unix.lib dependencies Matt Dillon's _fifo_ and Per Bojsen's _APipe_ are now included directly in the project
- **Designed for use with _unsui_**: Used as the standard C library for amigazen project's _unsui_ POSIX runtime for Amiga

### Development Goals

- Provide a complete, standards-compliant C library for Amiga development
- Enable easier porting of Unix/Linux software to Amiga
- Support modern C development practices on classic Amiga hardware
- Integrate seamlessly with the ToolKit development standard

## About ToolKit

**ToolKit** exists to solve the problem that most Amiga software was written in the 1980s and 90s, by individuals working alone, each with their own preferred setup for where their dev tools are run from, where their include files, static libs and other toolchain artifacts could be found, which versions they used and which custom modifications they made. Open source collaboration did not exist as we know it in 2025. 

**ToolKit** from amigazen project is a work in progress to make a standardised installation of not just the Native Developer Kit, but the compilers, build tools and third party components needed to be able to consistently build projects in collaboration with others, without each contributor having to change build files to work with their particular toolchain configuration. 

All *amigazen project* releases will release in a ready to build configuration according to the ToolKit standard.

Each component of **ToolKit** is open source and will have it's own github repo, while ToolKit itself will eventually be released as an easy to install package containing the redistributable components, as well as scripts to easily install the parts that are not freely redistributable from archive.

## Building UniLib3

UniLib3 is designed to build against the ToolKit standard. The build process uses SAS/C with smake for Amiga, ensuring compatibility with classic Amiga development workflows.

Detailed build instructions will be available in the [BUILD.md](BUILD.md) file.

### Prerequisites

- SAS/C compiler (primary target)
- DICE compiler (planned support)
- ToolKit development environment
- Amiga operating system 3.1 or higher

## Frequently Asked Questions

### Will there be a shared library version of UniLib3?

Before answering that, a discussion of how best to employ shared libraries. Dynamically loaded shared libraries require the whole library to be loaded into memory once, no matter how many applications use it, its only loaded once, but the whole thing is loaded. So this works well for libraries where most of the functions will be needed often, and a large number of programs will all be using the library. Boot up any Amiga and not only will ROM resident libraries like exec, dos and utility be loaded (that run directly from ROM and thus do not take up RAM space themselves) be available all the time because they are so fundamental to the running of the system, but many libraries will be loaded from disk too. Libraries like locale and datatypes are almost always going to be present unless running a lean boot to maximise free memory.

Static libraries on the other hand have their functions embedded in the programs that use them, duplicating the same functions both on disk and in memory over and over - for a popular function used in almost every program, like a printf() or an alloc() this is inefficient and those functions should be as lean as possible wrapping the system native implementation as thinly as possible, but for most functions, and the C library has dozens if not hundreds of functions, few of those functions are as widely used so it makes sense to embed them in the applications that use them but not have them loaded in memory all of the time. If you were to make the whole C library a dynamically loaded, the whole library would have to be loaded even though 90% of it would rarely be needed. This is one of the disadvantages of ixemul.library, which was already large with its early 1990s GNU C library. Imagine that an equivalent C library now in 2025 would be even larger - consider the original unix.lib was only 48KB, the second version 77KB and the new UniLib3 version is over 100KB and still growing simply because of all the additional functions it contains.

Linux and some other Unix-like platforms make use of libdl - which allows dynamic linking of static libraries. In many ways this is the worst of all possible worlds, insofar as in its simplest incarnation it loads the whole static library into memory - even though most of it is not needed. More advanced implementations can load individual functions as and when needed (along with the functions those functions depend upon, and the functions those dependencies depend upon and so on), but this in turn contributes to memory fragmentation and adds unnecessary complexity. In a world of ample RAM, stopgap solutions invented to solve problems of the 1990s where software complexity was outgrowing the typical RAM available on most computers, solutions such as libdl and virtual memory, are now unnecessary and a return to a leaner, more _kanso_ approach is warranted.

In conclusion the right solution is to:
- Keep popular functions as lean as possible, deferring to native implementations provided by the operating system's dynamic libraries.
- Use a static library for the bulk of functions you need to offer but which are not used all the time by all applications.
- For a library used by most or all programs running on the system, it may make sense to have the most popular functions available in a core dynamic library while the remaining functions are static only.

## Contact 

- At GitHub https://github.com/amigazen/unsui/ 
- on the web at http://www.amigazen.com/ (Amiga browser compatible)
- or email unsui@amigazen.com

## Acknowledgements

*Amiga* is a trademark of **Amiga Inc**. 

Unix is probably a trademark of someone somewhere.

UniLib3 is part of amigazen project's effort to modernize Amiga development tools and libraries. It incorporates works by:

- **David Gay** - Original creator of unix.lib who used it for his Emacs port to Amiga
- **Enrico Forestieri** - Developer of the second version of unix.lib, expanding it with more functions and adding sockets support
- **Irmen de Jong** - The developer behind the original Amiga Python 2, whose C library implementation code has been incorporated into UniLib3
- **Henry Spencer** - Author of public domain string functions distributed as 'stringlib'
- **Simon John Raybould** - Author of the curses library implementation for Amiga
- **Greg Parker** - Author of poll.c used in the _psockets_ implementation
- **Matt Dillon** - Creator of the FIFO system included here, and countless other Unix-on-Amiga foundation stones, not least the DICE compiler itself
- **Per Bojsen** - Developer of APipe 
- **The Regents of the University of California** - Original BSD code contributors