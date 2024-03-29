# ChangeLog
This is a overview of changes users may care about. Detailed changelog can be
displayed using git (does not work in tarballs, you need a development checkout
for this).


## 1,0

During the development of this release the project was migrated from bzr on
launchpad to git on GitHub.

Also as of 1,0 a new versioning scheme is used, see
[the README](README.md#version-number-scheme).

New features and improvements:

 * Added alternative IP list implementation, that stores pointers to the IPs
   instead. Useful for very large number (thousands) of IPs. This is now the
   default since the performance drop with a small number of threads is tiny
   compared to the increase for a large number of threads.
 * Cache env vars and other constant (per execution) data in y.
 * Changed stack stack code to not realloc once for each call of { and }.
 * Improved speed for non-cardinal warp.
 * Made cfunge work with the PathScale EKOPath compiler.
 * Add Zsh argument completion.
 * Automated running of tests with ctest, including mycology.

Changed features:

 * Dropped support for Boehm-GC. It was not well tested and manual memory
   management worked better.
 * Removed scaling of coordinates from TURT.
 * Dropped inline asm since intrinsics work well on modern GCC and ICC.
 * Improved fuzz testing script, and support for AFL fuzz testing.
 * Show more of the stack while tracing.

Major bug fixes:

 * Fixed bugs in run_sys_info in stack_get_index that together caused
   incorrect behaviour for y picking of top of stack and for last 0 below
   env vars.
 * Removed counts from hash tables if they reach zero. Needed for large model
   funge-space bounds shrinking logic to be correct.
 * Fixed bug in STRN G that resulted in an extra \0 being pushed.
 * Fixed STRN G, it incorrectly subtracted 1 from string length when
   calling stack_push_string().
 * Fixed several STRN functions truncating strings to unsigned chars.
 * Fixed bug in FILE causing a read of uninitialized memory when O (fopen) was
   called with an invalid mode.
 * Fix issue where using y for picking from stack would count from the wrong
   end of the stack.
 * Fix error handling of t (split).
 * Fix s wrapping and add test for it.
 * Fix (NCRS) for reentrant ncurses (thanks to Thomas Jollans @tjol)

Minor bug fixes:

 * Fixed possible problem in FRTH for very large stack sizes (outside range
   of funge cells, but inside range of size_t).
 * Various small fixes for clang and ICC.
 * Various build system fixes.
 * Handle zero bytes properly in stdin. Note that STRN still retains the old
   behaviour and arguably should do so.
 * Fixed various bugs (including memory leaks) when out of memory.
 * Fixed bug in text mode file output for lines with a single (non-space)
   char on them.
 * Add missing NULL pointer checks in code for i, o and = as well as in several
   fingerprints.
 * Fix several memory leaks on malloc failure (out of memory).
 * Fix pointer to out of scope buffer discovered with Coverity Scan.
 * Fix crash on integer division of -2^63 / -1
 * Fix some allocation size overflows in stack code.

Added fingerprints:

 * BOOL Logic Functions


## 0.9.0

This is a bug fix release. The version number changed to 0.9.0 to reflect that
cfunge is now reaching a stable version.

 * Bug fix: In a file with CR line endings and lines with leading spaces, the
   leading spaces were lost.
 * Bug fix: When j was used to jump backwads (negative argument) to a cell
   directly on the edge of the current bounding box of the Funge Space, it would
   incorrectly wrap. Fix this by peforming the normal moving forward of the IP
   before the jump in j instead of after as usual for most other instructions.
 * Minor bug fix: Hypens were incorrectly escaped when generating man pages in
   tools/gen_fprint_list.sh. Fix and update man page to reflect.


## 0.4.1

Major highlights:

 * cfunge can now track exact bounds for Funge Space (option at compile time).
 * Support for disabling some heavy parts of cfunge to reduce memory usage (see
   below for details).

Other important changes since last release:

 * For exact bounds: Implement sparse bounds minimising when difference is huge.
 * Reworked sysinfo code, now less overhead and more reused results (from time()
   and fungespace_get_bounds_rect()).
 * Bug fix: y pushed bounds wrong in certain cases.
 * Bug fix: Fixed OOM behaviour in cfunge on huge count for stack-stack begin.
 * DATE: Make sure constants are long double ones (L suffix). Do more of the
         conversion as integer math to reduce rounding errors. Make sure to
         always use int_least64_t internally for greatest precision. Add some
         missing function attributes.
 * TURT and FIXP: Use more exact value of pi than M_PI for long double
   calculations.
 * Added system for diagnostics output, collected in one place. This should make
   the error messages more uniform.
 * Added new command line switch (-E) to show non-fatal errors.
 * Added the command line switch (-v) to display build info and features in a
   compact format.
 * Reordered some code in file loading to make sure we don't push uninitialised
   size in i when trying to load an empty file.
 * Removed unused functions (cf_strndup() and cf_strnlen()).
 * Cleaned up includes (removing unused includes in header and source files).
 * Avoid malloc() calls in BASE. The max needed size is small and thus we can
   safely use a VLA.
 * Avoid some malloc() calls for fingerprint opcode stacks when they are empty.
   Now the actual stacks are not allocated until they are needed. Also "inline"
   the opcode stack metadata into the ip structs to reduce memory overhead.
 * Added the file doc/API_CHANGES documenting API changes. Please see that file
   for changes in the API.
 * Fixed build issue with IFFI.
 * Fixed handling of SSE intrinsics and inline asm for ICC in funge-space.c.
 * Build system:
   * Cleaned up warning flags in CMakeLists.txt.
   * Add options to disable some optional features even if needed dependencies
     found. Features that can be disabled are: clock_gettime(), ncurses,
     fingerprints using floating point, TURT.
   * Clean up CMakeLists.txt splitting some stuff out into separate files.


## 0.4.0

This release add support for some more fingerprints, speed up execution quite
a bit, and fixes several bugs. See details below.

Important changes in this release:

 * cfunge now depend on cmake 2.6 (or later).
 * cfunge is now quite a bit faster thanks to many parts rewritten in a faster
   way. This includes:
     * Stack-stack begin/end are faster.
     * Pushing strings on stack have been optimised a lot (important for y and
       various fingerprints).
     * Popping strings from stack pass string length info back to caller to
       avoid an extra strlen() call.
     * System information (y) faster by avoiding to allocate as much and as
       often.
     * Optimised N in STRN to avoid popping and pushing the string at all. It
       now operates on the stack directly.
     * Added a flag (-b) for fully buffered output, this halves the sys time for
       mycology and removes about 20% of the wall clock time.
     * Some code has been rewritten to allow GCC and ICC (and potentially other
       compilers) to auto-vectorise it.
     * Avoid reallocing the IP list in concurrent Funge as often.
     * Avoid ip_set_delta() in some often used macros for working on IPs.
     * Avoid strdup() of argv. We can point to them in the stack instead.
     * Avoid building vectors in many places, instead modify existing ones.
     * stringbuffer_finish() now returns string length, avoiding unneeded calls
       to strlen().
     * Annotate some branches as "probably not taken", this includes error paths
       for failed malloc() and such.
     * When looping over a block of the Funge-Space like:
         `for (x=0; x<maxx; x++)
            for (y=0; y<maxy; y++)`
       make sure the x-loop is the outermost loop in order to increase locality
       of reference in the static Funge-Space area (doesn't matter for the hash
       area outside that).
     * Use movntps if SSE is supported when space filling the static Funge-Space
       at startup in order to reduce cache trashing.
     * Optimise Funge-Space loading in various ways.
     * And more...
 * Various bug fixes in many places.
   * FILE: Fixed bug with how R works on EOF.
           The current Mycology version (2008–11–15) has a bug and will report
           the new behaviour as BAD. Next Mycology version will fix it.
   * FRTH: Fixed a crash bug when calling L or P with negative arguments.
   * PERL: Fixed a rare memory corruption bug.
   * PERL: Fix bug that caused data corruption on long resulting strings.
   * PERL: Remove a spurious stack_push() call.
   * STRN: Proper error checks for A (append) added.
   * Correctly handle form feed in i.
   * Handle CR line endings correctly (both initial file load and i).
   * Added casts to make the -Wconversion warning in GCC 4.3 useful. Fixed bugs
     found thanks to this.
 * Fingerprint format has been updated to 1.4 (1.2 still supported since the
   new features are a superset of the old ones). 1.3 added support for
   conditional compilation (enabling a fingerprint based on #if).
 * NCRS and TERM now use of the conditional compilation of fingerprints to make
   the dependency on ncurses optional.
 * Funge-108 renamed Funge-109 due to time constraints. Update all mentions of
   this in cfunge.
 * There has been some changes to make cfunge more portable:
   * Added some workarounds for OpenBSD (missing AI_ADDRCONFIG, ncurses
     redefining bool).
   * Made the fuzz testing script more portable.
   * If the XSI functions random and srandom aren't available, fall back on
     rand and srand.
   * For POSIX.1-2008 compatibility use clock_gettime() when available. Fall
     back on gettimeofday().
   * Added various checks at configuration time to check that needed functions
     exist.
 * Add the defines CFUNGE_VERSION and CFUNGE_API_VERSION to allow external
   fingerprints to easily find which cfunge version is used. Useful since the
   cfunge API isn't very stable.
 * Important API changes:
   * Renamed:
     * fungeCell to funge_cell
     * fungeUnsignedCell to funge_unsigned_cell
     * fungeVector to funge_vector
   * Removed:
     * ipDelta: Use funge_vector instead.
   * Changed:
     * stack_pop_string(): Additional out parameter, returns string length if
       non-NULL.
     * ip_forward(): Steps parameter removed. Will now only take one step
       forwards.
   * Added:
     * ip_backward(): Same as ip_forward() but takes one step backwards.
 * Build system cleanup:
   * Macros split into separate files in a subdirectory.
   * Check if some linker flags are supported.
 * Man page have been extended.
 * List of fingerprints in man page is now auto generated.

Added fingerprints:

 * DATE Date Functions
 * NCRS Ncurses Extension


## 0.3.3

Major highlights:

 * Massively faster Funge-Space for the commonly used area. cfunge now uses a
   static array for the Funge-Space that most common programs use (a chunk
   around 0,0 that is).
 * Funge-Space (outside the static area, that is the hash table area) now use
   memory pools for allocation. This reduces memory fragmentation and also
   malloc bookkeeping overhead. It is also slightly faster.
 * Reduced memory usage by fixing an over-allocation in the hash library code.
 * Fixed rare crash bug in the shared (between several fingerprints) string
   buffer code. Thanks to Heikki Kallasjoki and Alex Smith for reporting this.
 * Set FD_CLOEXEC on files and sockets in the fingerprints FILE and SOCK.
   This makes it close any sockets and files before invoking programs using
   either = or PERL.
 * Flush standard output less often, this increases speed in mycology a lot.
 * Changed function naming scheme, now using underscore instead of CamelCase.
 * Standard IO, i, o, FILE, SOCK and so on now use unsigned char*.
 * Minor bug fixes in TIME, STRN, REXP, the build system and various other
   places.

Important changes in this release:

 * Fixed off by one in TIME.
 * Made STRN use the global input buffer.
 * Fixed alignment bugs in several of the test programs.
 * Handle some cases of failed malloc() better.
 * Reduced memory usage by fixing an over-allocation in hash library code.
 * Removed some GNU LD specific linker flags to make the code link with other
   linkers.
 * Changed to use memcpy() to duplicate stack.
 * Added missing free() on some error code paths.
 * SCKE now use getaddrinfo() instead of the obsolete gethostbyname().
 * Changed some often accessed bitfields to full bools.
 * Funge-Space now use memory pools for allocation. This reduces memory
   fragmentation and also malloc bookkeeping overhead. It is also slightly
   faster.
 * Added valgrind annotations for the memory pools as a compile time option.
   If enabled this slows down the code by several orders of magnitude, but is
   useful for debugging.
 * Major changes to function names, they now use _ instead of CamelCase.
 * Fixed various build system bugs.
 * Major speed improvement in mycology, flushing output less often.
 * Strings in cfunge are now mostly unsigned.
 * Reading a file with i is now unsigned.
 * STDIO is now unsigned.
 * Removed kdevelop project files.
 * Set FD_CLOEXEC on files and sockets in the fingerprints FILE and SOCK. This
   makes it close any sockets and files before invoking programs using either =
   or PERL.
 * Fixed rare crash bug in the shared (between several fingerprints) string
   buffer code. Thanks to Heikki Kallasjoki and Alex Smith for reporting this.
 * STRN A is now a bit more efficient (still far from perfect).
 * Changed order of tests in bound testing code, slight speedup for most
   programs.
 * Fixed bug in REXP, REG_NOSUB used to cause E to push 128 empty strings, now
   it pushes a 0 instead.
 * Massively faster Funge-Space for the commonly used area. cfunge now uses a
   static array for the Funge-Space that most common programs use (a chunk
   around 0,0 that is).


## 0.3.2

Important changes in this release:

 * Fix small bug in K in SOCK causing the instruction to fail if the socket
   wasn't connected.
 * Add checks for some long double math.h functions that didn't exist on
   FreeBSD. Fall back on double versions.
 * Support cygwin a bit better.
 * Update FING to match updated specs.


## 0.3.1

Important changes in this release:

 * Code now use memcpy() to copy IP for t, then deep copy of the remaining
   elements.
 * Fixed crash with nested k instructions on t.
 * Fixed build system to work with other compilers than GCC (the C code already
   did work).
 * Fixed a lot of small issues detected with compiler warnings of the other
   compilers (like %zu when it should have been %zd in format string).
 * Fixed a single precision function call that should have used the double
   precision variant in FPDP.
 * Fixed off by one error in validation of handles in SOCK and FILE.
 * Made tools/fuzz-test.sh work on other systems.
 * Made build-man work for out of tree builds.
 * When using GC do not use ec (extensible cords). It caused segfault...

Added fingerprints:

 * SCKE TCP/IP async socket and dns resolving extension
 * SOCK TCP/IP socket extension


## 0.3.0

Important changes in this release:

 * Fixed a few bugs in loading code.
 * Added support for showing top 5 elements on the stack when tracing.
 * Fixed several bugs in StackPreallocSpace() that resulted in crashes later on.
 * Added a man page.
 * Imported two new libraries into the code:
   genx - An XML output library (used by TURT).
   stringbuffer - Some utilities to build strings in an easy way, code was taken
                  from crossfire.
 * Some work on Funge-108 features:
   * y now works as it should.
   * k handles ; as in Funge-108 (was undef in Funge-98).
 * Made & and ~ handle EOF correctly.
 * Made several fingerprints use stringbuffer, making their code much simpler.
 * New macro StackFreeString() to make handling Boehm-GC quirk easier.
 * Update of fingerprint spec format, some preliminary support for Funge-108
   style fingerprints.
 * New library dependency: due to TERM cfunge now needs ncurses.
 * Fixed Form Feed handling.
 * Made k on k work.
 * Fixed broken k (mycology was wrong about how to interpret it).
 * Fixed number of ticks spaces in strings took in concurrent Funge.
 * Fixed "jump to next instruction" in Funge-108 in 0k.
 * Several Funge-108 fixes in y command.
 * Various typo fixes.
 * Clean up the mess in loading code by using mmap() to read files.
 * Mycology was wrong about "y as pick" behaviour, fixed cfunge to conform to
   the standard.
 * Refactor Funge-Space loading code.
 * Fix bug with setting initial least point at loading.
 * Major renaming of types:
   * Renamed FUNGEDATATYPE to fungeCell.
   * Dropped FUNGEVECTORTYPE (use fungeCell instead).
   * Dropped fungePosition (use fungeVector instead).
   * FUNGEDATA*PRI for printf renamed to FUNGECELL*PRI.

Added fingerprints:

 * 3DSP 3D space manipulation extension
 * FING Operate on single fingerprint semantics
 * FRTH Some common forth commands
 * REXP Regular Expression Matching
 * STRN String functions
 * TERM Terminal control functions

Changes to fingerprints:

 * Fixed many bugs in TURT.
 * FILE got extended with the instruction D, added it.
 * Removed PNTR, it was a deprecated alias for INDV.
 * INDV instructions are now relative storage offset.
 * Certain JSTR and STRN instruction are now relative storage offset.

There was no 0.2.1, it got turned into 0.3.0.


## 0.2.1-pre2

Important changes in this release:

 * Speedup in pushing strings on stack.
 * Aliases for fingerprints supported (spec fileformat changed).
 * Added doxygen documentation for API exposed to fingerprints.
 * To help C-INTERCAL add a FungeSpaceLoadString()

Added fingerprints:

 * FILE File I/O functions
 * INDV Pointer functions
 * JSTR Read and write strings in Funge-Space
 * PNTR Alias for INDV
 * TIME Time and Date functions
 * TURT Simple Turtle Graphics Library


## 0.2.1-pre1

Important changes in this release:

 * Support for the (optional) i, o and = instructions.
 * Internal API was cleaned up to be more consistent with itself.
 * Fix some places were cfunge wasn't 100% standard conforming.
   (Note that mycology didn't detect these issues.)
 * Some fixes for bugs that caused crashes.
 * A lot of less serious bugs were fixed.
 * Some code were optimised (including the hash library).
 * And some added fingerprints.

Added fingerprints:

 * CPLI Complex Integer extension
 * FIXP Some useful math functions
 * FPDP Double precision floating point
 * FPSP Single precision floating point
 * HRTI High-Resolution Timer Interface
 * PERL Generic Interface to the Perl Language (not available in sandbox mode)
 * TOYS Funge-98 Standard Toys


## 0.2.0

This release adds fingerprint support (along with several fingerprints),
concurrency, a lot of bug fixes and implementing missing features.

Added fingerprints:

 * BASE I/O for numbers in other bases
 * DIRF Directory functions extension (not available in sandbox mode)
 * MODU Modulo Arithmetic Extension
 * NULL Funge-98 Null Fingerprint
 * ORTH Orthogonal Easement Library
 * REFC Referenced Cells Extension
 * ROMA Funge-98 Roman Numerals
 * SUBR Subroutine extension


## 0.1.0

Initial release. Had basic support for what is required in Befunge98 standard.
