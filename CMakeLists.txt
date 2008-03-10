PROJECT(CFUNGE08)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
MARK_AS_ADVANCED(
        CMAKE_BACKWARDS_COMPATIBILITY
)


# if you want the full compiler output, uncomment the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)

# add definitions, compiler switches, etc.
ADD_DEFINITIONS(-std=c99)
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
ADD_DEFINITIONS(-D_GNU_SOURCE -DGC_DEBUG)


include_directories (${CFUNGE08_SOURCE_DIR}/src/ ${CFUNGE08_SOURCE_DIR}/src/funge-space/b93/)


# list all source files here
FILE(GLOB CFUNGE08_SOURCES RELATIVE ${CFUNGE08_SOURCE_DIR} src/*.c src/funge-space/b93/*.c)
ADD_EXECUTABLE(cfunge08 ${CFUNGE08_SOURCES})


# need to link to some other libraries ? just add them here
TARGET_LINK_LIBRARIES(cfunge08 gc)
