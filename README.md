# UnixLib3

This is UnixLib3, a POSIX and C99 compatible standard C library for SAS/C and DICE compilers on Amiga.

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** uses modern software development tools and methods to update and rerelease classic Amiga open source software. Our releases include a new AWeb, this new Amiga Python 2, and the ToolKit project - a universal SDK for Amiga.

Key to our approach is ensuring every project can be built with the same common set of development tools and configurations, so we created the ToolKit project to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

The original authors of the *unix.lib* software are not affiliated with the amigazen project. This software is redistributed on terms described in the documentation, particularly the file LICENSE.md

Our philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all of our projects are gratefully received at [GitHub](https://github.com/amigazen/). While our focus now is on classic 68k software, we do intend that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## About UnixLib3

UnixLib3 is a comprehensive POSIX and C99 compatible standard C library implementation for Amiga, designed specifically for SAS/C and DICE compilers. This project aims to provide Amiga developers with a modern, standards-compliant C library that bridges the gap between classic Amiga development and contemporary POSIX standards.

### Project History

UnixLib3 represents the third major evolution of Unix compatibility libraries for Amiga:

**Version 1 (1995)**: The original unix.lib was created and placed into the public domain by David Gay specifically to support his port of GNU Emacs to Amiga. This first version provided essential Unix compatibility functions needed for Emacs to run on Amiga, focusing on the specific requirements of that port.

**Version ?** - A unix.lib of unknown provenance was also included with the Inet225 TCP/IP stack but the source code for this has not been made public. 

**Version 2 (1996)**: A second version was further developed by Enrico Forestieri who expanded the library to support porting various GNU utilities to Amiga. This version added many new functions as well as bsdsocket.library support.

**UnixLib3 (2025)**: This current version by amigazen project represents a complete refactor and expansion, incorporating not only the lessons learned from previous versions but also significant contributions merged in from Irmen de Jong's Amiga Python 2 implementation. UnixLib3 aims to be a comprehensive, POSIX and C99 standards-compliant solution rather than a limited compatibility layer.

### Key Features

- **POSIX Compliance**: Implements core POSIX.1 functionality for file operations, process management, and system interfaces
- **C99 Support**: Full C99 standard library implementation including stdio, stdlib, string, and math functions
- **Amiga Integration**: Native AmigaOS integration while maintaining POSIX compatibility
- **Compiler Support**: Primary support for SAS/C with planned DICE compiler compatibility
- **Memory Management**: Efficient memory allocation and management optimized for Amiga hardware constraints

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

## Building UnixLib3

UnixLib3 is designed to build against the ToolKit standard. The build process uses SAS/C with smake for Amiga, ensuring compatibility with classic Amiga development workflows.

### Prerequisites

- SAS/C compiler (primary target)
- DICE compiler (planned support)
- ToolKit development environment
- AmigaOS 3.2 or compatible

### Build Instructions

Detailed build instructions will be available in the [BUILD.md](BUILD.md) file.


## Contact 

- At GitHub https://github.com/amigazen/unsui/ 
- on the web at http://www.amigazen.com/ (Amiga browser compatible)
- or email unsui@amigazen.com

## Acknowledgements

*Amiga* is a trademark of **Amiga Inc**. 

Unix is probably a trademark of someone somewhere.

UnixLib3 is part of amigazen project's effort to modernize Amiga development tools and libraries.

**Historical Acknowledgements:**
- **David Gay** - Original creator of unix.lib for his GNU Emacs port to Amiga
- **Enrico Forestieri** - Developer of the second version of unix.lib, expanding Unix compatibility for Amiga
- **Irmen de Jong** - Creator of Amiga Python 2, whose C library implementation code has been incorporated into UnixLib3
- **The Regents of the University of California** - Original BSD code contributors (1982, 1986, 1991)

**Current Project:**
UnixLib3 represents a collaborative effort by amigazen project to create a modern, comprehensive Unix compatibility solution for Amiga, building upon the work of these pioneering developers while adding contemporary standards compliance and expanded functionality.

This software is redistributed on terms described in the documentation, particularly the file LICENSE.md