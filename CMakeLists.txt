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

PROJECT(CFUNGE)

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

# if you want the full compiler output, uncomment the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE(UsePkgConfig)
INCLUDE(CheckTypeSize)

OPTION(LEAK_MODE "If this is enabled, do not use boehm-gc. Do not enable unless you know what you are doing." OFF)
IF(LEAK_MODE)
	ADD_DEFINITIONS(-DDISABLE_GC)
ELSE(LEAK_MODE)
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
ENDIF(LEAK_MODE)

CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)

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

OPTION(USE_MUDFLAP "Enable mudflap pointer debugging library (does not work with GC)." OFF)
IF(USE_MUDFLAP)
	ADD_DEFINITIONS(-fmudflap)
ENDIF(USE_MUDFLAP)


OPTION(USE_WERROR "Should we build with -Werror" OFF)
IF(USE_WERROR)
	IF (CMAKE_COMPILER_IS_GNUCC)
		ADD_DEFINITIONS(-Werror)
	ENDIF (CMAKE_COMPILER_IS_GNUCC)
ENDIF(USE_WERROR)

# add definitions, compiler switches, etc.
ADD_DEFINITIONS(-std=c99)
IF (CMAKE_COMPILER_IS_GNUCC)
	ADD_DEFINITIONS(-Wall -Wextra -pedantic)
	ADD_DEFINITIONS(-Wpointer-arith)
	ADD_DEFINITIONS(-Wimplicit -Wnested-externs)
	ADD_DEFINITIONS(-Wcast-align -Wcast-qual -Wbad-function-cast)
	ADD_DEFINITIONS(-Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations)
	ADD_DEFINITIONS(-Wparentheses)
	ADD_DEFINITIONS(-Wshadow)
	ADD_DEFINITIONS(-Wundef)
	ADD_DEFINITIONS(-Wpacked)
	ADD_DEFINITIONS(-Wredundant-decls)
	ADD_DEFINITIONS(-Wfloat-equal)
	ADD_DEFINITIONS(-Wstrict-aliasing=2)
	ADD_DEFINITIONS(-Wformat=2)
	ADD_DEFINITIONS(-Wdisabled-optimization)
	ADD_DEFINITIONS(-Wunsafe-loop-optimizations)
	ADD_DEFINITIONS(-Winline)
	ADD_DEFINITIONS(-Wmissing-noreturn -Wmissing-format-attribute)
	ADD_DEFINITIONS(-Wdeclaration-after-statement)
	ADD_DEFINITIONS(-Wmissing-include-dirs)
	ADD_DEFINITIONS(-Wunused-function -Wunused-label -Wunused-value -Wunused-variable)
	ADD_DEFINITIONS(-Wwrite-strings)
	# Maintainer flags. Gives lots of false positives.
	#ADD_DEFINITIONS(-Wunreachable-code -fno-inline -fno-inline-functions-called-once -fno-inline-functions -fkeep-inline-functions -Wno-inline)
	# Even more false positives
	#ADD_DEFINITIONS(-Wunreachable-code)
ENDIF (CMAKE_COMPILER_IS_GNUCC)

# Feature test macros for glibc headers.
ADD_DEFINITIONS(-D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED)

IF (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	REMOVE_DEFINITIONS(-O2)
	ADD_DEFINITIONS(-DGC_DEBUG)
ELSE (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	ADD_DEFINITIONS(-fno-ident)
	ADD_DEFINITIONS(-fvisibility=hidden)
	REMOVE_DEFINITIONS(-DGC_DEBUG)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "DEBUG")

INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/lib/libghthash/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/funge-space/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/instructions/)
INCLUDE_DIRECTORIES(${CFUNGE_SOURCE_DIR}/src/fingerprints/)


# list all source files here
FILE(GLOB CFUNGE_SOURCES RELATIVE ${CFUNGE_SOURCE_DIR} lib/libghthash/*.c src/*.c src/funge-space/b98/*.c src/instructions/*.c src/fingerprints/*.c src/fingerprints/*/*.c)
ADD_EXECUTABLE(cfunge ${CFUNGE_SOURCES})

# need to link to some other libraries ? just add them here

SET_TARGET_PROPERTIES(cfunge PROPERTIES LINK_FLAGS "-Wl,-O1,--as-needed,--warn-common")

IF(NOT LEAK_MODE)
	TARGET_LINK_LIBRARIES(cfunge ${BOEHM_LIBRARY_GC} ${BOEHM_LIBRARY_CORD})
ENDIF(NOT LEAK_MODE)

IF(USE_MUDFLAP)
	SET_TARGET_PROPERTIES(cfunge PROPERTIES LINK_FLAGS "-fmudflap")
	TARGET_LINK_LIBRARIES(cfunge mudflap)
ENDIF(USE_MUDFLAP)
