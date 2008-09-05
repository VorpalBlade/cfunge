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

PROJECT(CFUNGE C)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

MARK_AS_ADVANCED(
	CMAKE_BACKWARDS_COMPATIBILITY
	EXECUTABLE_OUTPUT_PATH
	LIBRARY_OUTPUT_PATH
	BOEHM_INCLUDE_DIR
	BOEHM_CORD_INCLUDE_DIR
	BOEHM_LIBRARY_GC
	BOEHM_LIBRARY_CORD
	LEAK_MODE
	USE_MUDFLAP
	USE_WERROR
)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
#set(CMAKE_MODULE_PATH
#	${CMAKE_SOURCE_DIR}/cmake/Modules
#)


# if you want the full compiler output, uncomment the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)
# http://www.cmake.org/pipermail/cmake/2007-October/016786.html
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

INCLUDE(UsePkgConfig)
INCLUDE(CheckCCompilerFlag)


SET(MAN_INSTALL_PREFIX
	"${CMAKE_INSTALL_PREFIX}/share/man"
	CACHE PATH "Where to install man page."
)

# Macro to check if a CFLAG is supported, and then add it
MACRO(CFUNGE_CHECK_CFLAG name flag)
	CHECK_C_COMPILER_FLAG (${flag} _CHECKFLAG_${name})
	IF (_CHECKFLAG_${name})
		ADD_DEFINITIONS(${flag})
	ENDIF (_CHECKFLAG_${name})
ENDMACRO(CFUNGE_CHECK_CFLAG)


################################################################################
# Boehm GC
OPTION(USE_GC "If this is enabled, use Boehm-GC." OFF)
IF(USE_GC)
	PKGCONFIG(bdw-gc BoehmIncDir BoehmLinkDir BoehmLinkFlags BoehmCflags)
	IF (BoehmIncDir AND BoehmLinkDir AND BoehmLinkFlags AND BoehmCflags)
		MESSAGE(STATUS "Found Boehm GC")
	ELSE (BoehmIncDir AND BoehmLinkDir AND BoehmLinkFlags AND BoehmCflags)
		MESSAGE(FATAL_ERROR "Could NOT find new enough Boehm GC (you need 7.x or later)")
	ENDIF (BoehmIncDir AND BoehmLinkDir AND BoehmLinkFlags AND BoehmCflags)

	FIND_PATH(BOEHM_INCLUDE_DIR gc/gc.h
		${BoehmIncDir}
	)
	FIND_PATH(BOEHM_CORD_INCLUDE_DIR gc/cord.h
		${BoehmIncDir}
	)

	FIND_LIBRARY(BOEHM_LIBRARY_GC NAMES gc libgc
		PATHS
		${BoehmLinkDir}
	)
	FIND_LIBRARY(BOEHM_LIBRARY_CORD NAMES cord libcord
		PATHS
		${BoehmLinkDir}
	)
	INCLUDE_DIRECTORIES(${BOEHM_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${BOEHM_CORD_INCLUDE_DIR})
	LINK_DIRECTORIES(${BoehmLinkDir})
ELSE(USE_GC)
	ADD_DEFINITIONS(-DDISABLE_GC)
ENDIF(USE_GC)

################################################################################
# Other options
OPTION(USE_64BIT "Should 64-bit data types be used (otherwise 32-bit)." ON)
IF(USE_64BIT)
	ADD_DEFINITIONS(-DUSE64)
ELSE(USE_64BIT)
	ADD_DEFINITIONS(-DUSE32)
ENDIF(USE_64BIT)

OPTION(CONCURRENT_FUNGE "Enable support for concurrent funge." ON)
IF(CONCURRENT_FUNGE)
	ADD_DEFINITIONS(-DCONCURRENT_FUNGE)
ENDIF(CONCURRENT_FUNGE)

OPTION(ENABLE_TRACE "Enable support for tracing the execution (recommended)." ON)
IF(NOT ENABLE_TRACE)
	ADD_DEFINITIONS(-DDISABLE_TRACE)
ENDIF(NOT ENABLE_TRACE)

OPTION(USE_MUDFLAP "Enable mudflap pointer debugging library (does not work with GC)." OFF)
IF(USE_MUDFLAP)
	if (USE_GC)
		MESSAGE(FATAL_ERROR "You can't use mudflap and Boehm GC at the same time")
	ENDIF (USE_GC)
	ADD_DEFINITIONS(-fmudflap)
ENDIF(USE_MUDFLAP)

OPTION(USE_WERROR "Should we build with -Werror" OFF)
IF(USE_WERROR)
	IF (CMAKE_COMPILER_IS_GNUCC)
		CFUNGE_CHECK_CFLAG(Werror -Werror)
	ENDIF (CMAKE_COMPILER_IS_GNUCC)
ENDIF(USE_WERROR)

OPTION(HARDENED "If this is enabled, and GCC is used, enable stack smash protection (slows down though)." OFF)
IF (HARDENED)
	IF (CMAKE_COMPILER_IS_GNUCC)
		# Slows down though.
		CFUNGE_CHECK_CFLAG(fstack-protector -fstack-protector)
		ADD_DEFINITIONS(-DFORTIFY_SOURCE=2)
	ENDIF (CMAKE_COMPILER_IS_GNUCC)
ENDIF (HARDENED)


################################################################################
# Compiler defines.
CFUNGE_CHECK_CFLAG(std_c99 -std=c99)
# Feature test macros for glibc headers.
# Shouldn't hurt other systems.
ADD_DEFINITIONS(-D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED)



################################################################################
# Warnings
IF (CMAKE_COMPILER_IS_GNUCC)
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
	#ADD_DEFINITIONS(-Wunreachable-code -fno-inline -fno-inline-functions-called-once -fno-inline-functions -fkeep-inline-functions -Wno-inline)
	# Even more false positives
	#ADD_DEFINITIONS(-Wunreachable-code)
ENDIF (CMAKE_COMPILER_IS_GNUCC)



################################################################################
# Stuff for debug/non-debug builds
IF (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	REMOVE_DEFINITIONS(-O2)
	ADD_DEFINITIONS(-DGC_DEBUG)
	ADD_DEFINITIONS(-DDEBUG)
ELSE (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	ADD_DEFINITIONS(-DNDEBUG)
	REMOVE_DEFINITIONS(-DGC_DEBUG)
	REMOVE_DEFINITIONS(-DDEBUG)

	CFUNGE_CHECK_CFLAG(fvisibility_hidden -fvisibility=hidden)
	CFUNGE_CHECK_CFLAG(fno-ident -fno-ident)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "DEBUG")



################################################################################
# Includes
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/lib/libghthash/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/funge-space/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/instructions/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/fingerprints/)



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

ADD_EXECUTABLE(cfunge ${CFUNGE_SOURCES})

SET_TARGET_PROPERTIES(cfunge PROPERTIES
	LINK_FLAGS "-Wl,-O1,--as-needed,--warn-common"
)



################################################################################
# Linking libraries

# Need -lm for some fingerprints.
TARGET_LINK_LIBRARIES(cfunge m)
TARGET_LINK_LIBRARIES(cfunge ncurses)

IF(NOT LEAK_MODE)
	TARGET_LINK_LIBRARIES(cfunge ${BOEHM_LIBRARY_GC} ${BOEHM_LIBRARY_CORD})
ENDIF(NOT LEAK_MODE)

IF(USE_MUDFLAP)
	SET_TARGET_PROPERTIES(cfunge PROPERTIES LINK_FLAGS "-fmudflap")
	TARGET_LINK_LIBRARIES(cfunge mudflap)
ENDIF(USE_MUDFLAP)


################################################################################
# Generate man page
ADD_CUSTOM_TARGET(build-man
	help2man -N "--name=A fast Befunge93/98/108 interpreter in C" --help-option=-h --version-option=-V ${CFUNGE_BINARY_DIR}/cfunge > doc/cfunge.1
	DEPENDS cfunge
	WORKING_DIRECTORY ${CFUNGE_SOURCE_DIR}
	COMMENT "Generating man page..."
	VERBATIM
)

################################################################################
# Installation
INSTALL(TARGETS cfunge
	RUNTIME DESTINATION bin
)
INSTALL(FILES doc/cfunge.1
	DESTINATION ${MAN_INSTALL_PREFIX}/man1
)
