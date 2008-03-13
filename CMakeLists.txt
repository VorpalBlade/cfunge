PROJECT(CFUNGE08)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

MARK_AS_ADVANCED(
	CMAKE_BACKWARDS_COMPATIBILITY
	EXECUTABLE_OUTPUT_PATH
	LIBRARY_OUTPUT_PATH
)

# if you want the full compiler output, uncomment the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE(UsePkgConfig)
INCLUDE(CheckTypeSize)
PKGCONFIG(bdw-gc BoehmIncDir BoehmLinkDir BoehmLinkFlags BoehmCflags)

IF (BoehmIncDir AND BoehmLinkDir AND BoehmLinkFlags AND BoehmCflags)
	MESSAGE(STATUS "Found Boehm GC")
ELSE (BoehmIncDir AND BoehmLinkDir AND BoehmLinkFlags AND BoehmCflags)
	MESSAGE(FATAL_ERROR "Could NOT find new enough Boehm GC (you need 7.x or later)")
ENDIF (BoehmIncDir AND BoehmLinkDir AND BoehmLinkFlags AND BoehmCflags)

FIND_PATH(BOEHM_INCLUDE_DIR gc/gc.h
	${BoehmIncDir}
)


CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)

OPTION(USE_64BIT "Should 64-bit data types be used (otherwise 32-bit)." ON)
IF(USE_64BIT)
	ADD_DEFINITIONS(-DUSE64)
ELSE(USE_64BIT)
	ADD_DEFINITIONS(-DUSE32)
ENDIF(USE_64BIT)

# add definitions, compiler switches, etc.
ADD_DEFINITIONS(-std=c99)
IF (CMAKE_COMPILER_IS_GNUCC)
	ADD_DEFINITIONS(-Wall -Wextra -pedantic -Wformat=2)
	ADD_DEFINITIONS(-Wpointer-arith)
	ADD_DEFINITIONS(-Wwrite-strings -Wshadow)
	ADD_DEFINITIONS(-Wstrict-aliasing=2 -Wfloat-equal)
	ADD_DEFINITIONS(-Wcast-align -Wcast-qual -Wbad-function-cast)
	ADD_DEFINITIONS(-Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations)
	ADD_DEFINITIONS(-Wmissing-noreturn -Wmissing-format-attribute)
	ADD_DEFINITIONS(-Wundef)
	ADD_DEFINITIONS(-Wnested-externs -Wdeclaration-after-statement)
	ADD_DEFINITIONS(-Wmissing-include-dirs)
	ADD_DEFINITIONS(-Wunused-function -Wunused-label -Wunused-value -Wunused-variable)
	ADD_DEFINITIONS(-Wredundant-decls)
ENDIF (CMAKE_COMPILER_IS_GNUCC)
# Feature test macros for glibc headers.
ADD_DEFINITIONS(-D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED)
IF (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	REMOVE_DEFINITIONS(-O2)
	ADD_DEFINITIONS(-DGC_DEBUG)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "DEBUG")

ADD_DEFINITIONS(-I${BOEHM_INCLUDE_DIR})

include_directories (${CFUNGE08_SOURCE_DIR}/lib/libghthash/)
include_directories (${CFUNGE08_SOURCE_DIR}/src/)
include_directories (${CFUNGE08_SOURCE_DIR}/src/funge-space/)
include_directories (${CFUNGE08_SOURCE_DIR}/src/instructions/)
include_directories (${CFUNGE08_SOURCE_DIR}/src/fingerprints/)


# list all source files here
FILE(GLOB CFUNGE08_SOURCES RELATIVE ${CFUNGE08_SOURCE_DIR} lib/libghthash/*.c src/*.c src/funge-space/b98/*.c src/instructions/*.c src/fingerprints/*.c src/fingerprints/*/*.c)
ADD_EXECUTABLE(cfunge08 ${CFUNGE08_SOURCES})

# need to link to some other libraries ? just add them here
TARGET_LINK_LIBRARIES(cfunge08 ${BoehmLinkFlags} cord)
SET_TARGET_PROPERTIES(cfunge08 PROPERTIES LINK_FLAGS "-Wl,-O1,--as-needed,--warn-common")
