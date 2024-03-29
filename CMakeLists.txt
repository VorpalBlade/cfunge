# cfunge - A standard-conforming Befunge93/98/109 interpreter in C.
# Copyright (C) 2008 Arvid Norlander <VorpalBlade AT users.noreply.github.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at the proxy's option) any later version. Arvid Norlander is a
# proxy who can decide which future versions of the GNU General Public
# License can be used.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.12)

project(CFUNGE C)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Only out of tree builds are supported. Please execute something like: cd .. && mkdir cfunge-build && cd cfunge-build && cmake ${CMAKE_SOURCE_DIR}")
endif ()

enable_testing()

mark_as_advanced(
	CMAKE_BACKWARDS_COMPATIBILITY
	EXECUTABLE_OUTPUT_PATH
	LIBRARY_OUTPUT_PATH
	LIBRT_LOCATION
	CURSES_CURSES_H_PATH
	CURSES_FORM_LIBRARY
	CURSES_HAVE_CURSES_H
	CURSES_HAVE_NCURSES_CURSES_H
	CURSES_HAVE_NCURSES_H
	CURSES_HAVE_NCURSES_NCURSES_H
	USE_CLOCK_GETTIME
	ENABLE_FLOATS
	ENABLE_TURT
	USE_MUDFLAP
	USE_WERROR
	USE_VALGRIND
)

# Where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH
	${CMAKE_SOURCE_DIR}/cmake/modules
	${CMAKE_MODULE_PATH}
)

# Set default build type and debug cflags on first run, unless they have been
# set on cmake/ccmake command line.
if (NOT CONFIGURED)
	if (NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release CACHE STRING
		    "Choose the type of build, options are: None(CMAKE_C_FLAGS only used) Debug Release RelWithDebInfo MinSizeRel."
		    FORCE)
	endif ()
	if (CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3"
		    CACHE STRING "Flags used by the compiler during debug builds."
		    FORCE)
        set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG"
            CACHE STRING "Flags used by the compiler during release builds."
            FORCE)
	endif ()
endif ()

# http://www.cmake.org/pipermail/cmake/2007-October/016786.html
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

# Common cmake modules
include(CheckCCompilerFlag)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(FindPkgConfig)
include(GNUInstallDirs)

# Cfunge specific modules.
include(CfungeCheckCflag)
include(CfungeCheckFunction)
include(CfungeCheckLibraryFunction)
include(CfungeCheckLinkerFlag)
include(CfungeCheckWarningFlags)
include(CfungeRequireFunction)
include(CfungeRequireInclude)
include(CfungeRequireMultipleIncludes)
include(CfungeSetBuildInfoFlags)
include(MacroAddLinkFlags)


################################################################################
# Options to enable optional dependencies.

if (CMAKE_COMPILER_IS_CYGWIN)
	option(USE_NCURSES "Should we (try to) enable fingerprints needing ncurses?"  OFF)
	option(USE_CLOCK_GETTIME "Try to use clock_gettime() instead of gettimeofday(). (Recommended.)"  OFF)
else (CMAKE_COMPILER_IS_CYGWIN)
	option(USE_NCURSES "Should we (try to) enable fingerprints needing ncurses?"  ON)
	option(USE_CLOCK_GETTIME "Try to use clock_gettime() instead of gettimeofday(). (Recommended.)"  ON)
endif ()

option(ENABLE_FLOATS "Enable all fingerprints using floating point. (Recommended.)"  ON)
if (NOT ENABLE_FLOATS)
	add_definitions(-DCFUN_NO_FLOATS)
endif ()

option(ENABLE_TURT "Enable TURT. (Recommended.)" ON)
if (NOT ENABLE_TURT)
	add_definitions(-DCFUN_NO_TURT)
endif ()


################################################################################
# Other options

option(EXACT_BOUNDS "Handle shrinking bounds (slower and most programs don't need it, but follows the standard more closely)." ON)
if (EXACT_BOUNDS)
	add_definitions(-DCFUN_EXACT_BOUNDS)
endif ()

option(USE_64BIT "Use 64-bit funge space cells (if off: use 32-bit)." ON)
if (USE_64BIT)
	add_definitions(-DUSE64)
else (USE_64BIT)
	add_definitions(-DUSE32)
endif ()

option(CONCURRENT_FUNGE "Enable support for concurrent funge." ON)
if (CONCURRENT_FUNGE)
	add_definitions(-DCONCURRENT_FUNGE)
endif ()

option(LARGE_IPLIST "Use alternative IP list that works better with a lot of IPs but is slower with few IPs. No effect without CONCURRENT_FUNGE." ON)
if (CONCURRENT_FUNGE AND LARGE_IPLIST)
	add_definitions(-DLARGE_IPLIST)
endif ()

option(ENABLE_TRACE "Enable support for tracing the execution (recommended)." ON)
if (NOT ENABLE_TRACE)
	add_definitions(-DDISABLE_TRACE)
endif ()

option(HARDENED "If this is enabled, and GCC is used, enable stack smash protection (slows down though) and some other features." OFF)
if (HARDENED)
	add_definitions(-D_FORTIFY_SOURCE=2)
	# Slows down though.
	CFUNGE_CHECK_CFLAG(fstack-protector -fstack-protector)
endif ()


################################################################################
# Options for cfunge developers debugging cfunge.

option(USE_MUDFLAP "Enable mudflap pointer debugging library, requires GCC. Not recommended for general use." OFF)
if (USE_MUDFLAP)
	if (CMAKE_COMPILER_IS_GNUCC)
		CFUNGE_CHECK_CFLAG(fmudflap -fmudflap)
	endif ()
endif ()

option(USE_VALGRIND "Add annotations for the memory debugger valgrind. Not recommended for general use." OFF)
if (USE_VALGRIND)
	add_definitions(-DENABLE_VALGRIND)
endif ()

option(USE_WERROR "Should we build with -Werror (GCC only)." OFF)
if (USE_WERROR)
	if (CMAKE_COMPILER_IS_GNUCC)
		CFUNGE_CHECK_CFLAG(Werror -Werror)
	endif ()
endif ()


################################################################################
# Compiler defines. Cygwin has broken headers that hide some function prototypes
# based on STRICT_ANSI instead of checking for the relevant _POSIX_C_SOURCE,
# thus we use -std=gnu99 for it. Furthermore it thinks strdup is _GNU_SOURCE.
if (CMAKE_COMPILER_IS_CYGWIN)
	CFUNGE_CHECK_CFLAG(std_gnu99 -std=gnu99)
	add_definitions(-D_GNU_SOURCE)
else (CMAKE_COMPILER_IS_CYGWIN)
	CFUNGE_CHECK_CFLAG(std_c99 -std=c99)
endif ()

# Feature test macros for exposing relevant definitions in headers.
add_definitions(-D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600)


################################################################################
# Check that certain system headers exist.

# These are C99 required ones
CFUNGE_REQUIRE_INCLUDE(stddef.h)
CFUNGE_REQUIRE_INCLUDE(stdbool.h)
CFUNGE_REQUIRE_INCLUDE(stdint.h)
CFUNGE_REQUIRE_INCLUDE(limits.h)
CFUNGE_REQUIRE_INCLUDE(inttypes.h)

# POSIX.1-2001 requires these:
CFUNGE_REQUIRE_INCLUDE(arpa/inet.h)
CFUNGE_REQUIRE_INCLUDE(unistd.h)
CFUNGE_REQUIRE_INCLUDE(fcntl.h)
CFUNGE_REQUIRE_INCLUDE(sys/types.h)
CFUNGE_REQUIRE_INCLUDE(sys/socket.h)
CFUNGE_REQUIRE_INCLUDE(netinet/in.h)
# Some includes must be checked together.
CFUNGE_REQUIRE_MULTIPLE_INCLUDES("sys/types.h;netinet/tcp.h" NETINET_TCP_H)
CFUNGE_REQUIRE_INCLUDE(netdb.h)
CFUNGE_REQUIRE_INCLUDE(regex.h)

# Optional in POSIX, we need it
CFUNGE_REQUIRE_INCLUDE(sys/mman.h)


################################################################################
# Existence of various functions
# Some are sanity checks (required in standards), some are required by cfunge
# but optional in standards, some are optionally used by cfunge.
#
# All those in the second category are optional in POSIX.1-2001 but required in
# POSIX.1-2008
#
# We don't check functions that exist in C89.

# Sanity checks: Required by POSIX.1-2001.
CFUNGE_REQUIRE_FUNCTION(dup2)
CFUNGE_REQUIRE_FUNCTION(fcntl)
CFUNGE_REQUIRE_FUNCTION(fork)
CFUNGE_REQUIRE_FUNCTION(gettimeofday)
CFUNGE_REQUIRE_FUNCTION(mkdir)
CFUNGE_REQUIRE_FUNCTION(regcomp)
CFUNGE_REQUIRE_FUNCTION(rmdir)
# Sanity checks: Some networking functions (required by POSIX.1-2001)
CFUNGE_REQUIRE_FUNCTION(getaddrinfo)
CFUNGE_REQUIRE_FUNCTION(freeaddrinfo)
CFUNGE_REQUIRE_FUNCTION(poll)
CFUNGE_REQUIRE_FUNCTION(socket)

# Required: These are optional in POSIX.1-2001 but we need them.
CFUNGE_REQUIRE_FUNCTION(mmap)
CFUNGE_REQUIRE_FUNCTION(munmap)
# Required: We require the XSI extension strdup():
CFUNGE_REQUIRE_FUNCTION(strdup)

# Optional: Some optional ones (in POSIX.1-2001) that we use if found.
#  random() and srandom() are XSI extensions, we fall back on rand/srand if they
#  aren't available.
CFUNGE_CHECK_FUNCTION(random)
CFUNGE_CHECK_FUNCTION(srandom)

if (ENABLE_FLOATS)
	# Optional: C99 requires these but we fall back on double versions since many
	# systems still lack the long double versions.
	set(CMAKE_REQUIRED_LIBRARIES "-lm")
	CFUNGE_CHECK_FUNCTION(acosl)
	CFUNGE_CHECK_FUNCTION(asinl)
	CFUNGE_CHECK_FUNCTION(atanl)
	CFUNGE_CHECK_FUNCTION(cosl)
	CFUNGE_CHECK_FUNCTION(floorl)
	CFUNGE_CHECK_FUNCTION(powl)
	CFUNGE_CHECK_FUNCTION(roundl)
	CFUNGE_CHECK_FUNCTION(sinl)
	CFUNGE_CHECK_FUNCTION(sqrtl)
	CFUNGE_CHECK_FUNCTION(tanl)
endif ()


################################################################################
# Warnings
CFUNGE_CHECK_WARNING_FLAGS()


################################################################################
# Stuff for debug/non-debug builds
# No case insensitive string copmpare, lets convert to upper-case.
# NOTE: CMAKE_BUILD_TYPE_UPPERCASE is used in CFUNGE_SET_BUILD_INFO_FLAGS() too.
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPERCASE)
if (CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "DEBUG")
	add_definitions(-DDEBUG)
else (CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "DEBUG")
	remove_definitions(-DDEBUG)
	if (NOT CMAKE_COMPILER_IS_CYGWIN)
		CFUNGE_CHECK_CFLAG(fvisibility_hidden -fvisibility=hidden)
	endif ()
endif ()


################################################################################
# Some CFLAGS that won't hurt debug builds but will help release builds.
if (CMAKE_COMPILER_IS_GNUCC)
	CFUNGE_CHECK_CFLAG(fno_math_errno -fno-math-errno)
	CFUNGE_CHECK_CFLAG(fno_trapping_math -fno-trapping-math)
endif ()


################################################################################
# Includes
include_directories(${CFUNGE_SOURCE_DIR}/src/)


################################################################################
# Targets
FILE(GLOB CFUNGE_SOURCES RELATIVE ${CFUNGE_SOURCE_DIR}
	lib/mempool/*.c
	lib/libghthash/*.c
	lib/genx/*.c
	lib/stringbuffer/*.c
	lib/fungestring/*.c
	src/*.c
	src/funge-space/*.c
	src/instructions/*.c
	src/fingerprints/*.c
	src/fingerprints/*/*.c
)

add_executable(cfunge ${CFUNGE_SOURCES})

# Try various nice linker flags.
CFUNGE_CHECK_LINKER_FLAG(cfunge Wl_O1          -Wl,-O1)
CFUNGE_CHECK_LINKER_FLAG(cfunge Wl_as_needed   -Wl,--as-needed)
CFUNGE_CHECK_LINKER_FLAG(cfunge Wl_warn_common -Wl,--warn-common)
if (HARDENED)
	CFUNGE_CHECK_LINKER_FLAG(cfunge Wl_z_relro -Wl,-z,relro)
	CFUNGE_CHECK_LINKER_FLAG(cfunge Wl_z_now   -Wl,-z,now)
endif ()


################################################################################
# Check for ncurses - needs to be done late due to libraries.
# Quite a mess yes.
if (USE_NCURSES)
	set(CURSES_NEED_NCURSES TRUE)
	find_package(Curses)
	CHECK_INCLUDE_FILE(term.h HAVE_TERM_H)
	CHECK_INCLUDE_FILE(ncurses/term.h HAVE_NCURSES_TERM_H)

	if (CURSES_FOUND AND (HAVE_TERM_H OR HAVE_NCURSES_TERM_H) AND (CURSES_HAVE_CURSES_H OR CURSES_HAVE_NCURSES_CURSES_H))
		# HACK: We should use ${CURSES_LIBRARIES} here, but that breaks static
		# builds as it contains the full path to ncurses.so
		target_link_libraries(cfunge ncurses)
		add_definitions(-DHAVE_NCURSES)
		if (HAVE_NCURSES_TERM_H)
			add_definitions(-DTERM_H_IN_NCURSES)
		endif ()
		if (CURSES_HAVE_NCURSES_CURSES_H)
			add_definitions(-DCURSES_H_IN_NCURSES)
		endif ()
		if (CURSES_HAVE_NCURSES_NCURSES_H)
			add_definitions(-DNCURSES_H_IN_NCURSES)
		endif ()
	else()
		message(STATUS "ncurses and/or term.h not found: disabling the TERM fingerprint.")
	endif ()
endif ()


################################################################################
# Check for clock_gettime() - needs to be done late due to possible libraries.
#
# clock_gettime is complex to check for since it can be in libc (FreeBSD) or in
# librt (Linux).
if (USE_CLOCK_GETTIME)
	CFUNGE_CHECK_FUNCTION(clock_gettime)
	if (NOT CFUNGE_HAVE_clock_gettime)
		find_library(LIBRT_LOCATION
		             NAMES rt)
		if (NOT ${LIBRT_LOCATION} STREQUAL "LIBRT_LOCATION-NOTFOUND")
			get_filename_component(LIBRT_PATH ${LIBRT_LOCATION} PATH)
			CFUNGE_CHECK_LIBRARY_FUNCTION(cfunge rt clock_gettime ${LIBRT_PATH})
		endif ()
	endif ()
endif ()


################################################################################
# Check for arc4random and strlcpy
#
# These may be in libc (Various BSDs) or libbsd (Linux),
# or not be found at all.
CFUNGE_CHECK_FUNCTION(strlcpy)
CFUNGE_CHECK_FUNCTION(arc4random_buf)
CFUNGE_CHECK_FUNCTION(arc4random_stir)

# Try again in libbsd if that exists.
if (NOT CFUNGE_HAVE_strlcpy
    OR NOT CFUNGE_HAVE_arc4random_buf
    OR NOT CFUNGE_HAVE_arc4random_stir)

	find_library(LIBBSD_LOCATION
	             NAMES bsd)
	if (NOT ${LIBBSD_LOCATION} STREQUAL "LIBBSD_LOCATION-NOTFOUND")
		get_filename_component(LIBBSD_PATH ${LIBBSD_LOCATION} PATH)
		CFUNGE_CHECK_LIBRARY_FUNCTION(cfunge bsd strlcpy ${LIBBSD_PATH})
		if (CFUNGE_HAVE_bsd_strlcpy)
			# In this case we need <bsd/string.h>
			# FIXME: Should fail gracefully if missing.
			CFUNGE_REQUIRE_INCLUDE(bsd/string.h)
			add_definitions(-DSTRLCPY_IN_BSD)
		endif ()

		CFUNGE_CHECK_LIBRARY_FUNCTION(cfunge bsd arc4random_buf ${LIBBSD_PATH})
		CFUNGE_CHECK_LIBRARY_FUNCTION(cfunge bsd arc4random_stir ${LIBBSD_PATH})
		if (CFUNGE_HAVE_bsd_arc4random_buf AND CFUNGE_HAVE_bsd_arc4random_stir)
			# In this case we need <bsd/stdlib.h>
			# FIXME: Should fail gracefully if missing.
			CFUNGE_REQUIRE_INCLUDE(bsd/stdlib.h)
			add_definitions(-DARC4RANDOM_IN_BSD)
		endif ()
	endif ()
endif ()


################################################################################
# Linking libraries

# Need -lm for some fingerprints.
if (ENABLE_FLOATS)
	target_link_libraries(cfunge m)
endif ()

if (USE_MUDFLAP)
	MACRO_ADD_LINK_FLAGS(cfunge "-fmudflap")
	target_link_libraries(cfunge mudflap)
endif ()


################################################################################
# Build info
CFUNGE_SET_BUILD_INFO_FLAGS()


################################################################################
# Generate man page
find_program(HELP2MAN help2man
             DOC "Path to help2man (for re-generating man page).")
if (NOT ${HELP2MAN} STREQUAL "HELP2MAN-NOTFOUND")
	add_custom_target(build-man
		${HELP2MAN} -N "--name=A fast Befunge93/98/109 interpreter in C" --help-option=-h --version-option=-V --include doc/cfunge-man.in --include doc/cfunge-man-fingerprints.in ${CFUNGE_BINARY_DIR}/cfunge > doc/cfunge.1
		DEPENDS cfunge
		WORKING_DIRECTORY ${CFUNGE_SOURCE_DIR}
		COMMENT "Generating man page..."
		VERBATIM
	)
endif ()


################################################################################
# Tests
add_subdirectory(tests)

option(CFUNGE_ENABLE_COVERAGE "Enable coverage build target" OFF)
if (CFUNGE_ENABLE_COVERAGE)
	include(CodeCoverage)
	setup_target_for_coverage(cfunge_coverage ctest coverage)
	add_dependencies(cfunge_coverage cfunge)
	add_definitions(${CMAKE_C_FLAGS_COVERAGE} -DCFUNGE_COVERAGE)
	target_link_libraries(cfunge gcov)
endif()


################################################################################
# Installation
install(TARGETS cfunge
	RUNTIME DESTINATION bin
)
install(FILES doc/cfunge.1
	DESTINATION ${CMAKE_INSTALL_FULL_MANDIR}/man1
)
install(FILES etc/_cfunge
	DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/zsh/site-functions
)


################################################################################
# Flag to keep track of if this is the first configure run or not.
set(CONFIGURED ON CACHE INTERNAL "")
