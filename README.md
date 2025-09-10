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

**Version 2 (1996)**: A second version was further developed by Enrico Forestieri who expanded the library to support porting various x11 programs to Amiga. This version added many new functions as well as bsdsocket.library support.

**UniLib3 (2025)**: This current version by amigazen project presents a complete refactor and expansion, incorporating not only the lessons learned from previous versions but also significant contributions merged in from Irmen de Jong's Amiga Python 2 implementation and other BSD license-compatible projects. UniLib3 aims to be a comprehensive, POSIX and C99 standards-compliant solution pulling together all of the Unix-compatibility open source projects for Amiga from over the years into one unified library, excluding GPL/GNU projects for licensing reasons.

### Other unix.lib libraries

**Version ?** - A unix.lib of unknown provenance was also included with the Inet225 TCP/IP stack but the source code for this has not been made public. 

**clib2 version** - Olaf Barthel's clib2 generates several library targets including one called unix.lib which is a companion to the clib2 c.lib. This seems to simply contain a spillover of functions that did not fit in the main C library and is not meant to be used standalone. To avoid clashes with the original unix.lib that is part of UniLib3, the ToolKit SDK version of clib2 available at https://github.com/amigazen/clib2 renames this target "x.lib".

**AmigaPerl version** - Going even further back to 1990, David Grubbs created a simple unix.lib with functions necessary for his port of Perl 3. Some of those functions have been added to UniLib3's unix.lib where they offered something additional, including the Unix style file paths processing and wildcard handling.

### Key Features

- **POSIX compliance**: Implements core POSIX.1 functionality for file operations, process management, and system interfaces (except _fork()_)
- **C99 support**: Full C99 standard library implementation including stdio, stdlib, string, wchar, and (soon) math functions
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
- *New functions*: New Amiga native versions of POSIX functions including alloca(), ustat(), utime(), gettimeofday() and many more
- *Inet225 unix.lib*: Reimplements new versions of the POSIX functions provided by the version of unix.lib that came with Inet225
- *AmiTCP netlib*: Reimplements new versions of the POSIX functions provided by AmiTCP's netlib - rcmd(), herror() and the syslog API wrapping _bsdsocket.library_
- *librt APIs*: A brand new implementation of librt functions starting with POSIX mq_* message queues built on top of message ports, realtime.library and timer.device
- *libiconv APIs*: A brand new implementation of libiconv functions integrated with _locale.library_, with basic support for Latin1 and UTF8 codesets to start with
- *SysV IPC APIs*: SysV IPC implementation backported from the OS4 _sysvipc.library_ by Peter Bengtsson
- *Unit tests*: Unit tests for many of the new functions, aiming to reach full test coverage in time
- **curses.lib**: Updated version of Simon Raybould's Amiga port of _curses_ now BSD licensed
- **termcap and terminfo**: A brand new termcap and terminfo implementation built around con-handler, keymap.library and console.device supporting Amiga compatible ANSI escape codes
- **psockets.lib**: A brand new Amiga port of _psockets_ wrapping _bsdsocket.library_
- **pthread.lib**: A brand new Amiga native implementation of _pthread_ also including Diego Cassoran's _psem_ semaphores
- **regex.library**: Updated shared library implementation of Henry Spencer's regex functions made POSIX compatible
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

### Is **UniLib3** a complete standalone C library?

No, **UniLib3** and especially the _unix.lib_ core is designed to extend POSIX.1 and C99 C library functionality to existing C89 standard C libraries, in particular the _sc.lib_ and _scm.lib_ libraries that ship with SAS/C for Amiga. The SAS/C libraries are already extensive, covering all of C89 and many more utility functions, and also well optimised for Amiga computers, but in some cases have older function prototypes not compliant to the POSIX standard and deviating behaviours reflecting the needs and limits of 1990s Amiga development. Development of SAS/C for the Amiga ended in the early 1990s, before the standard set of C libraries settled and therefore it is missing many crucial functions, even if in many cases the underlying Amiga native libraries can support the functionality. **UniLib3** therefore exists to provide those missing functions as a superset of what _sc.lib_ provides, and eventually also the complex math extensions missing from _scm.lib_.

To use _unix.lib_, it is necessary to ensure that your project has the **UniLib3** include files ahead of your compiler's default C library headers, and to ensure _unix.lib_ is ahead of _sc.lib_ in the linker chain so that where functions that exist in both libraries, the **UniLib3** version is the one used.

While **UniLib3**'s roadmap is currently focussed on providing the functions missing from _sc.lib_ over time there's no reason the additional functions needed to complete the coverage of other Amiga C compilers such as DICE cannot be added.

With all that said, since the SAS/C library source code is not open source, nor maintained or readily available, a long term goal could be to also provide a complete C library with all-new implementations of the C89 standard library, but it is unlikely that such an implementation would be as well optimised, with many _sc.lib_ functions having been implemented in pure assembler.

### How can a C library help make Amiga a more robust and reliable platform?

Amiga has well known limitations in system reliability and stability that come as a result of its physical hardware pre-dating consumer personal computing hardware coming with features such as memory management units as standard. Furthermore the exec architecture turns the weakness of the lack of memory protection and resource tracking into a performance advantage, specifically employing a shared memory space to avoid the context switching overhead microkernels experience when implementing a hard division between supervisor and user space.

A partial solution to this problem, is for software to be built on top of a C library that provides a lot of protections automatically. Historically features such as automatic stack checking, Ctrl-C checking, resource tracking and cleanup, added a high memory and processing time overhead often left unused in the name of maximising application performance. But today's Amigas, even classic hardware, typically have upgraded memory and faster CPUs meaning that overhead goes away. 

### Will there be a shared library version of UniLib3?

Before answering that, a discussion of how best to employ shared libraries. Dynamically loaded shared libraries require the whole library to be loaded into memory once, no matter how many applications use it, its only loaded once, but the whole thing is loaded. So this works well for libraries where most of the functions will be needed often, and a large number of programs will all be using the library. Boot up any Amiga and not only will ROM resident libraries like exec, dos and utility be loaded (that run directly from ROM and thus do not take up RAM space themselves) be available all the time because they are so fundamental to the running of the system, but many libraries will be loaded from disk too. Libraries like locale and datatypes are almost always going to be present unless running a lean boot to maximise free memory.

Static libraries on the other hand have their functions embedded in the programs that use them, duplicating the same functions both on disk and in memory over and over - for a popular function used in almost every program, like a printf() or an alloc() this is inefficient and those functions should be as lean as possible wrapping the system native implementation as thinly as possible, but for most functions, and the C library has dozens if not hundreds of functions, few of those functions are as widely used so it makes sense to embed them in the applications that use them but not have them loaded in memory all of the time. If you were to make the whole C library a dynamically loaded, the whole library would have to be loaded even though 90% of it would rarely be needed. This is one of the disadvantages of ixemul.library, which was already large with its early 1990s GNU C library. Imagine that an equivalent C library now in 2025 would be even larger - consider the original unix.lib was only 48KB, the second version 77KB and the new UniLib3 version is over 100KB and still growing simply because of all the additional functions it contains.

Linux and some other Unix-like platforms make use of libdl - which allows dynamic linking of static libraries. In many ways this is the worst of all possible worlds, insofar as in its simplest incarnation it loads the whole static library into memory - even though most of it is not needed. More advanced implementations can load individual functions as and when needed (along with the functions those functions depend upon, and the functions those dependencies depend upon and so on), but this in turn contributes to memory fragmentation and adds unnecessary complexity. In a world of ample RAM, stopgap solutions invented to solve problems of the 1990s where software complexity was outgrowing the typical RAM available on most computers, solutions such as libdl and virtual memory, are now unnecessary and a return to a leaner, more _kanso_ approach is warranted.

In conclusion the right solution is to:
- Keep popular functions as lean as possible, deferring to native implementations provided by the operating system's dynamic libraries.
- Use a static library for the bulk of functions you need to offer but which are not used all the time by all applications.
- For a library used by most or all programs running on the system, it may make sense to have the most popular functions available in a core dynamic library while the remaining functions are static only.

### Why do we need both fifo and apipe when the operating system already provides the Queue-Handler and PIPE:?

While AmigaOS 3.2's built-in queue-handler and PIPE: device provide basic pipe functionality, they fall short of the comprehensive POSIX pipe requirements that modern Unix software expects. Here's why we need both FIFO and APipe:

#### **What Queue-Handler Provides:**
- **Basic named pipes** (`PIPE:name`) and anonymous pipes
- **Blocking I/O** with configurable buffer sizes
- **Simple message passing** between processes using a standard Amiga message port interface

#### **What's Missing for Full POSIX Compatibility:**

**1. Process Execution Pipes (`popen()`/`pclose()`)**
- Queue-handler cannot execute commands and connect their I/O streams
- APipe provides the `APIPE:` device that can spawn processes and create pipes to their stdin/stdout/stderr
- Essential for shell command execution, subprocess management, and system() calls

**2. Advanced FIFO Features**
- **Non-blocking I/O** (`O_NONBLOCK`) - Queue-Handler only supports blocking operations
- **Socket pairs** (`socketpair()`) - Not supported by Queue-Handler
- **Cooked mode** with line buffering and special character processing
- **`mkfifo()`** function for creating named pipes programmatically
- **Priority-based message handling** with multiple priority levels
- **Signal-based notifications** for async I/O events

**3. POSIX Compliance**
- **Standard file descriptors** - Queue-Handler uses Amiga message ports, not file descriptors
- **`select()`/`poll()` support** - Required for multiplexed I/O
- **Error handling** - POSIX-compliant errno values and error reporting
- **Resource management** - Proper cleanup and reference counting

**4. Performance and Reliability**
- **Optimized buffering** - FIFO system provides better memory management
- **Concurrent access** - Better handling of multiple readers/writers
- **Robust error recovery** - Better handling of edge cases and error conditions

#### **Why Both FIFO and APipe?**

**FIFO (Matt Dillon's fifo.library)** provides:
- Complete POSIX pipe implementation
- Non-blocking I/O and advanced features
- Socket pairs and named pipes
- High-performance message queuing

**APipe (Per Bojsen's APipe-Handler)** provides:
- Process execution pipes (`popen()`/`pclose()`)
- Command execution with I/O redirection
- Subprocess management
- Integration with Amiga's process system

#### **The Integration Strategy:**
Rather than trying to extend the basic Queue-Handler to support all POSIX features (which would be a massive undertaking), UniLib3 integrates the proven, battle-tested FIFO and APipe systems that already provide these capabilities. This approach:

- **Leverages existing, working code** that's been tested in real-world applications
- **Maintains compatibility** with existing Amiga software that uses these systems
- **Provides full POSIX compliance** without reinventing the wheel
- **Ensures reliability** by using mature, stable implementations

The result is a complete POSIX pipe implementation that works seamlessly with both classic Amiga software and modern Unix applications ported to Amiga.

### Which regex shared library is included here?

There have been at least three shared library interfaces defined on Amiga over the years including:

- **regex.library**: A shared library implementing the POSIX-defined regular expression API
- **regexp.library**: A shared library similar to but different from the POSIX regular expression function interface, based on the well known Henry Spencer public domain algorithms
- **pcre.library**: A shared library version of the Perl-compatible regular expression library, again similar to the above but defining a different interface and behaviours

Since UniLib3 is a project to bring POSIX interfaces to Amiga, the _regex.library_ is the one included here. However, of these three the source code has only ever been released for _regexp.library_ and _pcre.library_, so the version here is an all-new open source version designed to be API compatible with the previously released versions. The other two libraries can be found as part of the ToolKit project.

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
- **Irmen de Jong** - The developer behind the original Amiga Python 2, whose amiga compatibility code has been incorporated into UniLib3
- **Henry Spencer** - Author of the public domain string functions distributed as _stringlib_ and the regex functions in _regex.library_
- **Simon John Raybould** - Author of the _curses_ library implementation for Amiga
- **Greg Parker** - Author of poll.c used in the _psockets_ implementation
- **Matt Dillon** - Creator of the FIFO system included here, and countless other Unix-on-Amiga foundation stones, not least the DICE compiler itself
- **Per Bojsen** - Developer of APipe 
- **Diego Cassoran** - Developer of _libpsem_, an Amiga native POSIX semaphores implementation
- **Peter Bengtsson** - Developer of _sysvipc.library_ for OS4
- **David Grubb** - Ported Perl version 3 to Amiga in 1990 and at the same time created a set of Unix functions
- **The Regents of the University of California** - Original BSD code contributors