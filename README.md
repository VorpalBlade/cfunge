# cfunge

[![Build Status](https://app.travis-ci.com/VorpalBlade/cfunge.svg?branch=master)](https://app.travis-ci.com/VorpalBlade/cfunge)
[![codecov](https://codecov.io/gh/VorpalBlade/cfunge/branch/master/graph/badge.svg)](https://codecov.io/gh/VorpalBlade/cfunge)

This is cfunge - a fast Befunge93/98/109 interpreter in C.

cfunge offers some features that many other standard conforming interpreters
don't. For example:

 * Sandbox mode, prevents programs from harming system (more details below).
 * Advance tracing support, debugging protocol support system under development.
 * Passes mycology (of course other conforming interpreters does this, but most
   interpreters are, sadly, not conforming).
 * Tested using Valgrind and similar tools.
 * Tested with fuzz testing to ensure cfunge does not segfault on random stuff.
 * Coded for maximum performance AND correctness.
 * Support for selecting either 64-bit or 32-bit integers as datatype in
   funge-space at compile time.


## Dependencies

### Required

 * cmake (http://www.cmake.org/) to generate a Makefile for cfunge. At least
   version 3.12 is required.

 * A C99 compiler, or one that supports a large subset of C99, such as GCC or
   clang. Any relatively modern versions should work. Other compilers may or
   may not work.

 * A POSIX.1-2001 system with the memory mapped file option. It also needs the
   function strdup() which is part of the XSI extension. (For POSIX.1-2008 both
   strdup() and memory mapped files are mandatory and part of the base.)
   Operating systems known to work:
   + Linux 2.6.28 (with glibc 2.9) and later (earlier versions unknown but will
     probably work unless very old).
   + FreeBSD 6.4 or later (earlier versions unknown).
   + Mac OS X (unknown version) has been reported to work but I don't have it
     myself and thus can't test it.
   + NetBSD 5.0.1/amd64 is known to work.
   + OpenBSD 4.4 (sparc64): I have not managed to get cmake 2.6 to compile on
     OpenBSD, but I did manage to build cfunge by hand, required some strange
     workarounds and defines.

   Windows most likely won't work. Nor do I plan to support it. Cygwin may work,
   but you are on your own if it doesn't.

### Highly recommended

 * Ncurses (http://www.gnu.org/software/ncurses/), needed for the TERM
   fingerprint. This is most likely already installed, though headers may need
   to be installed separately via some form of -dev package. Ncurses will be
   automatically used if found.
 * IEC 60559 floating-point arithmetic. Please see Annex F in ISO/IEC 9899 for
   more details.
 * LibBSD (or have a BSD libc). This allows using arc4random which provides
   better randomness than the standard random() function.


## Configuring

**Warning**: Out of tree builds are highly recommended. Building in the source
tree will not work. Building in a sub-directory of the source tree may work
but is _completely_ untested.

**Warning**: Only the make and ninja backends for cmake have been tested. Other
backends may work, but are untested.

To build using cmake, you can run these commands in the top source directory:

```console
$ mkdir ../build && cd ../build
$ cmake ../cfunge  # Adjust path as needed.
```

If you are on a 32-bit system you may want to use 32-bit integers (instead of
64-bit integers) for speed. It is also slightly faster on some 64-bit systems
to use 32-bit integers. This may vary between architectures. To use 32-bit
integers you could use these commands instead of the above ones:

```console
$ mkdir ../build && cd ../build
$ cmake -DUSE_64BIT=OFF ../cfunge  # Adjust path as needed.
```

If you want to see a list of available options use ccmake. It will allow you to
select options in a ncurses based user interface. Help is always shown at the
bottom of the screen.

```console
$ mkdir ../build && cd ../build
$ ccmake ../cfunge  # Adjust path as needed.
  (press c)
  (change options - use t to show advanced options)
  (press c again)
  (press g to generate make file)
```

For more information see:

```console
$ cmake --help
```

and/or

```console
$ ccmake --help
```


## Compiling

After having run cmake as described in the above section, just run:

```console
$ make
```


Installing
----------
Not needed, cfunge can be run from build directory, but if you want to (after
having compiled cfunge):

```console
$ make install
```


Fingerprints
------------
It is planned to implement most or all of the existing fingerprints,
with some exceptions:

 * FNGR - Contradicts with 98 standard.
 * IMAP - Too intrusive.
 * MODE - Intrusive into IP handling.
 * TRDS - Exceedingly complex and intrusive.
 * WIND - Too complex to implement and not portable.

Short descriptions of implemented fingerprints:

Finger print | Description
------------ | -----------
3DSP         | 3D space manipulation extension
BASE         | I/O for numbers in other bases
BOOL         | Logic Functions
CPLI         | Complex Integer extension
DATE         | Date Functions
DIRF         | Directory functions extension
FILE         | File I/O functions
FING         | Operate on single fingerprint semantics
FIXP         | Some useful math functions
FPDP         | Double precision floating point
FPSP         | Single precision floating point
FRTH         | Some common forth commands
HRTI         | High-Resolution Timer Interface
INDV         | Pointer functions
JSTR         | Read and write strings in Funge-Space
MODU         | Modulo Arithmetic Extension
NCRS         | Ncurses Extension
NULL         | Funge-98 Null Fingerprint
ORTH         | Orthogonal Easement Library
PERL         | Generic Interface to the Perl Language
REFC         | Referenced Cells Extension
REXP         | Regular Expression Matching
ROMA         | Funge-98 Roman Numerals
SCKE         | TCP/IP async socket and dns resolving extension
SOCK         | TCP/IP socket extension
STRN         | String functions
SUBR         | Subroutine extension
TERM         | Terminal control functions
TIME         | Time and Date functions
TOYS         | Funge-98 Standard Toys
TURT         | Simple Turtle Graphics Library

For more details please see the specs for each fingerprint.
In cases of undefined behaviour in fingerprints, cfunge mostly tries to do the
same thing as CCBI.


## Undefined behaviour

The Befunge98 standard leaves some things undefined, here is what we do for
some of those cases:

 * `y` pushes time in UTC not local time.
 * `k` with a negative argument reflects.
 * `#` across edge of funge-space may or may not skip first char after wrapping
   depending on exact situation.
 * `(` and `)` with a negative count reflects and doesn't pop any fingerprint.
 * Loaded fingerprints are inherited to child IPs at split (`t`).
 * STDOUT is only flushed at:
   * Newline (line feed, ASCII 10) printed using `,` instruction.
   * Any input instructions.
   * End of program.
 * Standard input is read one line at a time and buffered internally. Those
   instructions reading chars fetch one char from this buffer, leaving the rest
   (if any) including any ending newline. Instructions reading an integer will
   leave anything after the integer in the buffer with one exception: if the
   next char is a newline it will be discarded.


## Notes on different standards

The option `-s 93` does not prevent the program from accessing outside the first
80x25 cells. Nor does it disallow instructions that didn't exist in 93. It does
however change space behaviour to match 93 style.

If a program depends on a instruction that is undefined in 93 to reflect, it
should be easy to replace such instructions with a r for reflect or any in the
range `A`-`Z` (and not load any fingerprint).

Further division by zero always returns 0 in all modes, though the Befunge93
specs says the interpreter should ask the user what result he/she wants in that
situation.


## Sandbox mode

Sandbox mode prevents Funge programs from doing "harmful" things, this includes,
but is not limited to:

 * Any file or filesystem IO is forbidden.
 * The list of environment variables the program can see in y are restricted.
 * Non-safe fingerprints can not be loaded (this includes network and file
   system access as well as other things).
