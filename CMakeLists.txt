# cfunge - a conformant Befunge93/98/08 interpreter in C.
# Copyright (C) 2008 Arvid Norlander <anmaster AT tele2 DOT se>
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

project(CFUNGE C)

cmake_minimum_required(VERSION 2.6)

mark_as_advanced(
	CMAKE_BACKWARDS_COMPATIBILITY
	EXECUTABLE_OUTPUT_PATH
	LIBRARY_OUTPUT_PATH
	LIBRT_LOCATION
	BOEHM_INCLUDE_DIR
	BOEHM_LIBRARY_GC
	USE_MUDFLAP
	USE_WERROR
	USE_VALGRIND
)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
#set(CMAKE_MODULE_PATH
#	${CMAKE_SOURCE_DIR}/cmake/Modules
#)


# if you want the full compiler output, uncomment the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)
# http://www.cmake.org/pipermail/cmake/2007-October/016786.html
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

include(FindPkgConfig)
include(CheckCCompilerFlag)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFile)

set(MAN_INSTALL_PREFIX
	"${CMAKE_INSTALL_PREFIX}/share/man"
	CACHE PATH "Where to install man page."
)



################################################################################
# Macros

# Macro to check if a CFLAG is supported, and then add it
macro(CFUNGE_CHECK_CFLAG name flag)
	CHECK_C_COMPILER_FLAG(${flag} _CHECKFLAG_${name})
	if (_CHECKFLAG_${name})
		add_definitions(${flag})
	endif (_CHECKFLAG_${name})
endmacro(CFUNGE_CHECK_CFLAG)

# Macro to check if a library exists and have a function and add it to
# libraries to link to target if it does.
macro(CFUNGE_CHECK_LIBRARY_FUNC target library function location)
	CHECK_LIBRARY_EXISTS(${library} ${function} ${location} _CHECKLIBFUNC_${library}_${function})
	if (_CHECKLIBFUNC_${library}_${function})
		set(HAVE_${library}_${function} true PARENT_SCOPE)
		add_definitions(-DHAVE_${function})
		target_link_libraries(${target} ${library})
	endif (_CHECKLIBFUNC_${library}_${function})
endmacro(CFUNGE_CHECK_LIBRARY_FUNC)

# Macro to check if a function is supported and add a define if yes.
macro(CFUNGE_CHECK_FUNC name)
	CHECK_FUNCTION_EXISTS(${name} _CHECKFUNC_${name})
	if (_CHECKFUNC_${name})
		set(CFUNGE_HAVE_${name} true PARENT_SCOPE)
		add_definitions(-DHAVE_${name})
	endif (_CHECKFUNC_${name})
endmacro(CFUNGE_CHECK_FUNC)

# Macro to check if a function is supported and error out if not.
macro(CFUNGE_REQUIRE_FUNC name)
	CHECK_FUNCTION_EXISTS(${name} _CHECKFUNC_${name})
	if (NOT _CHECKFUNC_${name})
		message(FATAL_ERROR
		        "Your system seems to be missing the function \"${name}\" which is required by cfunge.")
	endif (NOT _CHECKFUNC_${name})
endmacro(CFUNGE_REQUIRE_FUNC)

# Macro to ensure that a specific system header exists; will error out if not.
macro(CFUNGE_REQUIRE_INCLUDE name)
	CHECK_INCLUDE_FILE(${name} _CHECKINCLUDE_${name})
	if (NOT _CHECKINCLUDE_${name})
		message(FATAL_ERROR
		        "Your system seems to be missing the header \"${name}\" which is required by cfunge.")
	endif (NOT _CHECKINCLUDE_${name})
endmacro(CFUNGE_REQUIRE_INCLUDE)



################################################################################
# Boehm GC
option(USE_GC "If this is enabled, use Boehm-GC." OFF)
if(USE_GC)
	pkg_check_modules(BOEHMGC REQUIRED bdw-gc>=7.0)

	find_path(BOEHM_INCLUDE_DIR gc/gc.h
		${BOEHMGC_INCLUDE_DIRS}
	)

	find_library(BOEHM_LIBRARY_GC NAMES gc
		PATHS
		${BOEHMGC_LIBRARY_DIRS}
	)
	include_directories(${BOEHM_INCLUDE_DIR})
	link_directories(${BOEHMGC_LIBRARY_DIRS})
else(USE_GC)
	add_definitions(-DDISABLE_GC)
endif(USE_GC)



################################################################################
# Other options
option(USE_64BIT "Should 64-bit data types be used (otherwise 32-bit)." ON)
if(USE_64BIT)
	add_definitions(-DUSE64)
else(USE_64BIT)
	add_definitions(-DUSE32)
endif(USE_64BIT)

option(CONCURRENT_FUNGE "Enable support for concurrent funge." ON)
if(CONCURRENT_FUNGE)
	add_definitions(-DCONCURRENT_FUNGE)
endif(CONCURRENT_FUNGE)

option(ENABLE_TRACE "Enable support for tracing the execution (recommended)." ON)
if(NOT ENABLE_TRACE)
	add_definitions(-DDISABLE_TRACE)
endif(NOT ENABLE_TRACE)

option(USE_MUDFLAP "Enable mudflap pointer debugging library (does not work with Boehm-GC), needs GCC." OFF)
if(USE_MUDFLAP)
	if (USE_GC)
		message(FATAL_ERROR "You can't use mudflap and Boehm GC at the same time")
	endif (USE_GC)
	add_definitions(-fmudflap)
endif(USE_MUDFLAP)

option(USE_VALGRIND "Add special source annotations for the memory debugger valgrind (does not work with Boehm-GC)." OFF)
if(USE_VALGRIND)
	if (USE_GC)
		message(FATAL_ERROR "You can't use valgrind and Boehm GC at the same time")
	endif (USE_GC)
	add_definitions(-DENABLE_VALGRIND)
endif(USE_VALGRIND)


option(USE_WERROR "Should we build with -Werror" OFF)
if(USE_WERROR)
	if (CMAKE_COMPILER_IS_GNUCC)
		CFUNGE_CHECK_CFLAG(Werror -Werror)
	endif (CMAKE_COMPILER_IS_GNUCC)
endif(USE_WERROR)

option(HARDENED "If this is enabled, and GCC is used, enable stack smash protection (slows down though)." OFF)
if (HARDENED)
	if (CMAKE_COMPILER_IS_GNUCC)
		# Slows down though.
		CFUNGE_CHECK_CFLAG(fstack-protector -fstack-protector)
		add_definitions(-DFORTIFY_SOURCE=2)
	endif (CMAKE_COMPILER_IS_GNUCC)
endif (HARDENED)



################################################################################
# Compiler defines.
CFUNGE_CHECK_CFLAG(std_c99 -std=c99)
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
CFUNGE_REQUIRE_INCLUDE(unistd.h)
CFUNGE_REQUIRE_INCLUDE(fcntl.h)



################################################################################
# Existance of various functions

# These are optional in POSIX.1-2001 but we need them.
CFUNGE_REQUIRE_FUNC(mmap)
CFUNGE_REQUIRE_FUNC(munmap)

# These aren't optional in POSIX, but if someone tries this on a non POSIX
# system they may be missing.
CFUNGE_REQUIRE_FUNC(dup2)
CFUNGE_REQUIRE_FUNC(fork)
CFUNGE_REQUIRE_FUNC(fcntl)
CFUNGE_REQUIRE_FUNC(socket)
CFUNGE_REQUIRE_FUNC(regcomp)
CFUNGE_REQUIRE_FUNC(rmdir)
CFUNGE_REQUIRE_FUNC(mkdir)
CFUNGE_REQUIRE_FUNC(gettimeofday)

# Some optional ones (in POSIX.1-2001) that can use optionally.
# These are complex to check for since they can be in libc (FreeBSD) or
# librt (Linux).

CFUNGE_CHECK_FUNC(clock_gettime)
if (NOT CFUNGE_HAVE_clock_gettime)
	find_library(LIBRT_LOCATION
	             NAMES rt)
endif (NOT CFUNGE_HAVE_clock_gettime)

# random() and srandom() are XSI extensions, we fall back on rand/srand if
# they aren't available.
CFUNGE_CHECK_FUNC(random)
CFUNGE_CHECK_FUNC(srandom)

# However we require the XSI extension strdup():
CFUNGE_REQUIRE_FUNC(strdup)

# For these we fall back on non-l version.
set(CMAKE_REQUIRED_LIBRARIES "-lm")
CFUNGE_CHECK_FUNC(acosl)
CFUNGE_CHECK_FUNC(asinl)
CFUNGE_CHECK_FUNC(atanl)
CFUNGE_CHECK_FUNC(cosl)
CFUNGE_CHECK_FUNC(powl)
CFUNGE_CHECK_FUNC(roundl)
CFUNGE_CHECK_FUNC(sinl)
CFUNGE_CHECK_FUNC(sqrtl)
CFUNGE_CHECK_FUNC(tanl)



################################################################################
# Warnings
if (CMAKE_COMPILER_IS_GNUCC)
	# Not all GCC versions support all of these flags.
	CFUNGE_CHECK_CFLAG(Wall                         -Wall)
	CFUNGE_CHECK_CFLAG(Wextra                       -Wextra)
	CFUNGE_CHECK_CFLAG(pedantic                     -pedantic)
	CFUNGE_CHECK_CFLAG(Wpointer-arith               -Wpointer-arith)
	CFUNGE_CHECK_CFLAG(Wimplicit                    -Wimplicit)
	CFUNGE_CHECK_CFLAG(Wnested-externs              -Wnested-externs)
	CFUNGE_CHECK_CFLAG(Wcast-align                  -Wcast-align)
	CFUNGE_CHECK_CFLAG(Wcast-qual                   -Wcast-qual)
	CFUNGE_CHECK_CFLAG(Wbad-function-cast           -Wbad-function-cast)
	CFUNGE_CHECK_CFLAG(Wstrict-prototypes           -Wstrict-prototypes)
	CFUNGE_CHECK_CFLAG(Wmissing-prototypes          -Wmissing-prototypes)
	CFUNGE_CHECK_CFLAG(Wmissing-declarations        -Wmissing-declarations)
	CFUNGE_CHECK_CFLAG(Wparentheses                 -Wparentheses)
	CFUNGE_CHECK_CFLAG(Wshadow                      -Wshadow)
	CFUNGE_CHECK_CFLAG(Wundef                       -Wundef)
	CFUNGE_CHECK_CFLAG(Wpacked                      -Wpacked)
	CFUNGE_CHECK_CFLAG(Wredundant-decls             -Wredundant-decls)
	CFUNGE_CHECK_CFLAG(Wfloat-equal                 -Wfloat-equal)
	# =2 is best on 4.1.2, but not on 4.3... Figure out a fix for this.
	CFUNGE_CHECK_CFLAG(Wstrict-aliasing_2           -Wstrict-aliasing=2)
	CFUNGE_CHECK_CFLAG(Wformat_2                    -Wformat=2)
	CFUNGE_CHECK_CFLAG(Wdisabled-optimization       -Wdisabled-optimization)
	# This is not very useful really.
	#CFUNGE_CHECK_CFLAG(Winline                      -Winline)
	CFUNGE_CHECK_CFLAG(Wmissing-noreturn            -Wmissing-noreturn)
	CFUNGE_CHECK_CFLAG(Wmissing-format-attribute    -Wmissing-format-attribute)
	CFUNGE_CHECK_CFLAG(Wdeclaration-after-statement -Wdeclaration-after-statement)
	CFUNGE_CHECK_CFLAG(Wunused-function             -Wunused-function)
	CFUNGE_CHECK_CFLAG(Wunused-label                -Wunused-label)
	CFUNGE_CHECK_CFLAG(Wunused-value                -Wunused-value)
	CFUNGE_CHECK_CFLAG(Wunused-variable             -Wunused-variable)
	CFUNGE_CHECK_CFLAG(Wwrite-strings               -Wwrite-strings)
	CFUNGE_CHECK_CFLAG(Winit-self                   -Winit-self)
	# GCC 4.1(?) or later
	CFUNGE_CHECK_CFLAG(Wunsafe-loop-optimizations   -Wunsafe-loop-optimizations)
	CFUNGE_CHECK_CFLAG(Wmissing-include-dirs        -Wmissing-include-dirs)

	# Maintainer flags. Gives lots of false positives.
	#add_definitions(-Wunreachable-code -fno-inline -fno-inline-functions-called-once -fno-inline-functions -fkeep-inline-functions -Wno-inline)
	# Even more false positives
	#add_definitions(-Wunreachable-code)
endif (CMAKE_COMPILER_IS_GNUCC)



################################################################################
# Stuff for debug/non-debug builds
# No case insensitive string copmpare, lets convert to upper-case.
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPERCASE)
if (CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "DEBUG")
	add_definitions(-DGC_DEBUG)
	add_definitions(-DDEBUG)
else (CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "DEBUG")
	add_definitions(-DNDEBUG)
	remove_definitions(-DGC_DEBUG)
	remove_definitions(-DDEBUG)
	CFUNGE_CHECK_CFLAG(fvisibility_hidden -fvisibility=hidden)
	CFUNGE_CHECK_CFLAG(fno-ident -fno-ident)
endif (CMAKE_BUILD_TYPE_UPPERCASE STREQUAL "DEBUG")



################################################################################
# Includes
include_directories(${CFUNGE_SOURCE_DIR}/src/)



################################################################################
# Targets
FILE(GLOB CFUNGE_SOURCES RELATIVE ${CFUNGE_SOURCE_DIR}
	lib/libghthash/*.c
	lib/genx/*.c
	lib/stringbuffer/*.c
	src/*.c
	src/funge-space/*.c
	src/instructions/*.c
	src/fingerprints/*.c
	src/fingerprints/*/*.c
)

add_executable(cfunge ${CFUNGE_SOURCES})

# This is disabled until I figure out how to detect if the flags are supported.
#set_target_properties(cfunge PROPERTIES
#	LINK_FLAGS "-Wl,-O1,--as-needed,--warn-common"
#)



################################################################################
# Linking libraries

# Need -lm for some fingerprints.
target_link_libraries(cfunge m)
target_link_libraries(cfunge ncurses)

if(USE_GC)
	target_link_libraries(cfunge ${BOEHM_LIBRARY_GC})
endif(USE_GC)

if(USE_MUDFLAP)
	set_target_properties(cfunge PROPERTIES LINK_FLAGS "-fmudflap")
	target_link_libraries(cfunge mudflap)
endif(USE_MUDFLAP)

if (NOT ${LIBRT_LOCATION} STREQUAL "LIBRT_LOCATION-NOTFOUND")
	get_filename_component(LIBRT_PATH ${LIBRT_LOCATION} PATH)
	CFUNGE_CHECK_LIBRARY_FUNC(cfunge rt clock_gettime ${LIBRT_PATH})
endif (NOT ${LIBRT_LOCATION} STREQUAL "LIBRT_LOCATION-NOTFOUND")



################################################################################
# Generate man page
add_custom_target(build-man
	help2man -N "--name=A fast Befunge93/98/108 interpreter in C" --help-option=-h --version-option=-V --include doc/cfunge-man.in --include doc/cfunge-man-fingerprints.in ${CFUNGE_BINARY_DIR}/cfunge > doc/cfunge.1
	DEPENDS cfunge
	WORKING_DIRECTORY ${CFUNGE_SOURCE_DIR}
	COMMENT "Generating man page..."
	VERBATIM
)



################################################################################
# Installation
install(TARGETS cfunge
	RUNTIME DESTINATION bin
)
install(FILES doc/cfunge.1
	DESTINATION ${MAN_INSTALL_PREFIX}/man1
)
